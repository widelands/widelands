/*
 * Copyright (C) 2011-2013 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "gamerenderer_gl.h"

#include "gl_surface.h"

#include "logic/editor_game_base.h"
#include "logic/player.h"

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/texture.h"

#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

static const uint PatchSize = 4;

GameRendererGL::GameRendererGL() :
	m_patch_vertices_size(0),
	m_patch_indices_size(0)
{
}

GameRendererGL::~GameRendererGL()
{
}

void GameRendererGL::rendermap
	(RenderTarget & dst,
	 Widelands::Editor_Game_Base const &       egbase,
	 Widelands::Player           const &       player,
	 Point                                     viewofs)
{
	m_surface = dynamic_cast<GLSurface *>(dst.get_surface());
	if (!m_surface)
		return;
	m_rect = dst.get_rect();
	m_offset = viewofs + m_rect - dst.get_offset();
	m_egbase = &egbase;
	m_player = &player;

	draw();
}

void GameRendererGL::rendermap
	(RenderTarget & dst,
	 Widelands::Editor_Game_Base const & egbase,
	 Point                               viewofs)
{
	m_surface = dynamic_cast<GLSurface *>(dst.get_surface());
	if (!m_surface)
		return;
	m_rect = dst.get_rect();
	m_offset = viewofs + m_rect - dst.get_offset();
	m_egbase = &egbase;
	m_player = 0;

	draw();
}

void GameRendererGL::draw()
{
	draw_terrain();
}

uint GameRendererGL::patch_index(int32_t fx, int32_t fy) const
{
	uint x = fx - m_patch_size.x;
	uint y = fy - m_patch_size.y;

	assert(x < m_patch_size.w);
	assert(y < m_patch_size.h);

	uint outerx = x / PatchSize;
	uint outery = y / PatchSize;
	uint innerx = x % PatchSize;
	uint innery = y % PatchSize;

	return
		(outery * (m_patch_size.w / PatchSize) + outerx) * (PatchSize * PatchSize) +
		innery * PatchSize + innerx;
}

void GameRendererGL::draw_terrain()
{
	assert(sizeof(basevertex) == 16);

	assert(m_offset.x >= 0); // divisions involving negative numbers are bad
	assert(m_offset.y >= 0);

	m_minfx = m_offset.x / TRIANGLE_WIDTH - 1;
	m_minfy = m_offset.y / TRIANGLE_HEIGHT - 1;
	m_maxfx = (m_offset.x + m_rect.w + (TRIANGLE_WIDTH / 2)) / TRIANGLE_WIDTH;
	m_maxfy = (m_offset.y + m_rect.h) / TRIANGLE_HEIGHT;

	// fudge for triangle boundary effects and for height differences
	m_minfx -= 1;
	m_minfy -= 1;
	m_maxfx += 1;
	m_maxfy += 10;

	m_patch_size.x = m_minfx - 1;
	m_patch_size.y = m_minfy;
	m_patch_size.w = ((m_maxfx - m_minfx + 2 + PatchSize) / PatchSize) * PatchSize;
	m_patch_size.h = ((m_maxfy - m_minfy + 1 + PatchSize) / PatchSize) * PatchSize;

	glScissor
		(m_rect.x, m_surface->get_h() - m_rect.y - m_rect.h,
		 m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);

	setup_terrain_base();
	draw_terrain_base();

	glDisable(GL_SCISSOR_TEST);
}

void GameRendererGL::setup_terrain_base()
{
	Map const & map = m_egbase->map();

	uint reqsize = m_patch_size.w * m_patch_size.h;
	if (reqsize > 0x10000)
		throw wexception("Too many vertices; decrease screen resolution");

	if (reqsize > m_patch_vertices_size) {
		m_patch_vertices.reset(new basevertex[reqsize]);
		m_patch_vertices_size = reqsize;
	}

	if (m_terrain_freq.size() < 16)
		m_terrain_freq.resize(16);
	m_terrain_freq.assign(m_terrain_freq.size(), 0);

	uint index = 0;
	for (uint outery = 0; outery < m_patch_size.h / PatchSize; ++outery) {
		for (uint outerx = 0; outerx < m_patch_size.w / PatchSize; ++outerx) {
			for (uint innery = 0; innery < PatchSize; ++innery) {
				for (uint innerx = 0; innerx < PatchSize; ++innerx) {
					Coords coords
						(m_patch_size.x + outerx * PatchSize + innerx,
						 m_patch_size.y + outery * PatchSize + innery);
					assert(index == patch_index(coords.x, coords.y));
					Coords ncoords(coords);
					map.normalize_coords(ncoords);
					FCoords fcoords = map.get_fcoords(ncoords);
					Point pix;
					MapviewPixelFunctions::get_basepix(coords, pix.x, pix.y);
					pix.y -= fcoords.field->get_height() * HEIGHT_FACTOR;
					pix -= m_offset;
					m_patch_vertices[index].x = pix.x;
					m_patch_vertices[index].y = pix.y;
					Point tex;
					MapviewPixelFunctions::get_basepix(coords, tex.x, tex.y);
					m_patch_vertices[index].tcx = float(tex.x) / TEXTURE_WIDTH;
					m_patch_vertices[index].tcy = float(tex.y) / TEXTURE_HEIGHT;
					++index;

					if
						(coords.x < m_minfx || coords.y < m_minfy ||
						 coords.x > m_maxfx || coords.y > m_maxfy)
						continue;

					Terrain_Index ter_d = fcoords.field->get_terrains().d;
					Terrain_Index ter_r = fcoords.field->get_terrains().r;
					if (ter_d >= m_terrain_freq.size())
						m_terrain_freq.resize(ter_d + 1);
					m_terrain_freq[ter_d] += 1;
					if (ter_r >= m_terrain_freq.size())
						m_terrain_freq.resize(ter_r + 1);
					m_terrain_freq[ter_r] += 1;
				}
			}
		}
	}

	m_terrain_freq_cum.resize(m_terrain_freq.size());
	uint nrtriangles = 0;
	for (uint idx = 0; idx < m_terrain_freq.size(); ++idx) {
		m_terrain_freq_cum[idx] = nrtriangles;
		nrtriangles += m_terrain_freq[idx];
	}

	if (3 * nrtriangles > m_patch_indices_size) {
		m_patch_indices.reset(new uint16_t[3 * nrtriangles]);
		m_patch_indices_size = 3 * nrtriangles;
	}

	index = 0;
	for (Terrain_Index ter = 0; ter < m_terrain_freq.size(); ++ter) {
		assert(index == 3 * m_terrain_freq_cum[ter]);
		for (uint outery = 0; outery < m_patch_size.h / PatchSize; ++outery) {
			for (uint outerx = 0; outerx < m_patch_size.w / PatchSize; ++outerx) {
				for (uint innery = 0; innery < PatchSize; ++innery) {
					for (uint innerx = 0; innerx < PatchSize; ++innerx) {
						Coords coords
							(m_patch_size.x + PatchSize * outerx + innerx,
							 m_patch_size.y + PatchSize * outery + innery);
						if
							(coords.x < m_minfx || coords.y < m_minfy ||
							 coords.x > m_maxfx || coords.y > m_maxfy)
							continue;

						Coords ncoords(coords);
						map.normalize_coords(ncoords);
						FCoords fcoords = map.get_fcoords(ncoords);
						if (fcoords.field->get_terrains().d == ter) {
							Coords brn(coords.x + (coords.y & 1), coords.y + 1);
							Coords bln(brn.x - 1, brn.y);
							m_patch_indices[index++] = patch_index(coords.x, coords.y);
							m_patch_indices[index++] = patch_index(bln.x, bln.y);
							m_patch_indices[index++] = patch_index(brn.x, brn.y);
						}
						if (fcoords.field->get_terrains().r == ter) {
							Coords brn(coords.x + (coords.y & 1), coords.y + 1);
							Coords rn(coords.x + 1, coords.y);
							m_patch_indices[index++] = patch_index(coords.x, coords.y);
							m_patch_indices[index++] = patch_index(brn.x, brn.y);
							m_patch_indices[index++] = patch_index(rn.x, rn.y);
						}
					}
				}
			}
		}
	}
}

void GameRendererGL::draw_terrain_base()
{
	Map const & map = m_egbase->map();
	World const & world = map.world();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glVertexPointer(2, GL_FLOAT, sizeof(basevertex), &m_patch_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(basevertex), &m_patch_vertices[0].tcx);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glColor3f(1.0, 1.0, 1.0);
	glDisable(GL_BLEND);

	for (Terrain_Index ter = 0; ter < m_terrain_freq.size(); ++ter) {
		if (!m_terrain_freq[ter])
			continue;

		const Texture & texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(ter).get_texture());
		glBindTexture(GL_TEXTURE_2D, texture.getTexture());
		glDrawRangeElements
			(GL_TRIANGLES,
			 0, m_patch_size.w * m_patch_size.h - 1,
			 3 * m_terrain_freq[ter], GL_UNSIGNED_SHORT,
			 &m_patch_indices[3 * m_terrain_freq_cum[ter]]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
