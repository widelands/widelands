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
	m_patch_indices_size(0),
	m_edge_vertices_size(0)
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

uint8_t GameRendererGL::field_brightness(const FCoords & coords) const
{
	uint32_t brightness;
	brightness = 144 + coords.field->get_brightness();
	brightness = (brightness * 255) / 160;
	if (brightness > 255)
		brightness = 255;

	if (m_player && !m_player->see_all()) {
		const Map & map = m_egbase->map();
		const Player::Field & pf = m_player->fields()[Map::get_index(coords, map.get_width())];
		if (pf.vision == 0) {
			return 0;
		} else if (pf.vision == 1) {
			static const uint32_t DecayTime = 20000;
			Duration time_ago = m_egbase->get_gametime() - pf.time_node_last_unseen;
			if (time_ago < DecayTime)
				brightness = (brightness * (2 * DecayTime - time_ago)) / (2 * DecayTime);
			else
				brightness = brightness / 2;
		}
	}

	return brightness;
}

void GameRendererGL::draw_terrain()
{
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

	prepare_terrain_base();
	draw_terrain_base();
	if (g_gr->caps().gl.multitexture && g_gr->caps().gl.max_tex_combined >= 2) {
		prepare_terrain_fuzz();
		draw_terrain_fuzz();
	}

	glDisable(GL_SCISSOR_TEST);
}

template<typename vertex>
void GameRendererGL::compute_basevertex(const Coords & coords, vertex & vtx) const
{
	Map const & map = m_egbase->map();
	Coords ncoords(coords);
	map.normalize_coords(ncoords);
	FCoords fcoords = map.get_fcoords(ncoords);
	Point pix;
	MapviewPixelFunctions::get_basepix(coords, pix.x, pix.y);
	pix.y -= fcoords.field->get_height() * HEIGHT_FACTOR;
	pix -= m_offset;
	vtx.x = pix.x;
	vtx.y = pix.y;
	Point tex;
	MapviewPixelFunctions::get_basepix(coords, tex.x, tex.y);
	vtx.tcx = float(tex.x) / TEXTURE_WIDTH;
	vtx.tcy = float(tex.y) / TEXTURE_HEIGHT;
	uint8_t brightness = field_brightness(fcoords);
	vtx.color[0] = vtx.color[1] = vtx.color[2] = brightness;
	vtx.color[3] = 255;
}

void GameRendererGL::prepare_terrain_base()
{
	assert(sizeof(basevertex) == 32);

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
					compute_basevertex(coords, m_patch_vertices[index]);
					++index;

					if
						(coords.x < m_minfx || coords.y < m_minfy ||
						 coords.x > m_maxfx || coords.y > m_maxfy)
						continue;

					Coords ncoords(coords);
					map.normalize_coords(ncoords);
					FCoords fcoords = map.get_fcoords(ncoords);
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
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(basevertex), &m_patch_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

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
	glDisableClientState(GL_COLOR_ARRAY);
}

/*
 * Schematic of triangle neighborhood:
 *
 *               *
 *              / \
 *             / u \
 *         (f)/     \
 *    *------*------* (r)
 *     \  l / \  r / \
 *      \  /   \  /   \
 *       \/  d  \/ rr  \
 *       *------*------* (br)
 *        \ dd /
 *         \  /
 *          \/
 *          *
 */
void GameRendererGL::prepare_terrain_fuzz()
{
	assert(sizeof(edgefuzzvertex) == 32);

	Map const & map = m_egbase->map();
	World const & world = map.world();

	if (m_terrain_edge_freq.size() < 16)
		m_terrain_edge_freq.resize(16);
	m_terrain_edge_freq.assign(m_terrain_edge_freq.size(), 0);

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);

			Terrain_Index ter_d = fcoords.field->get_terrains().d;
			Terrain_Index ter_r = fcoords.field->get_terrains().r;
			Terrain_Index ter_u = map.tr_n(fcoords).field->get_terrains().d;
			Terrain_Index ter_rr = map.r_n(fcoords).field->get_terrains().d;
			Terrain_Index ter_l = map.l_n(fcoords).field->get_terrains().r;
			Terrain_Index ter_dd = map.bl_n(fcoords).field->get_terrains().r;
			int32_t lyr_d = world.get_ter(ter_d).edge_fuzz_layer();
			int32_t lyr_r = world.get_ter(ter_r).edge_fuzz_layer();
			int32_t lyr_u = world.get_ter(ter_u).edge_fuzz_layer();
			int32_t lyr_rr = world.get_ter(ter_rr).edge_fuzz_layer();
			int32_t lyr_l = world.get_ter(ter_l).edge_fuzz_layer();
			int32_t lyr_dd = world.get_ter(ter_dd).edge_fuzz_layer();

			if (lyr_r > lyr_d) {
				if (ter_r >= m_terrain_edge_freq.size())
					m_terrain_edge_freq.resize(ter_r + 1);
				m_terrain_edge_freq[ter_r] += 1;
			} else if (ter_d != ter_r) {
				if (ter_d >= m_terrain_edge_freq.size())
					m_terrain_edge_freq.resize(ter_d + 1);
				m_terrain_edge_freq[ter_d] += 1;
			}
			if ((lyr_u > lyr_r) || (lyr_u == lyr_r && ter_u != ter_r)) {
				if (ter_u >= m_terrain_edge_freq.size())
					m_terrain_edge_freq.resize(ter_u + 1);
				m_terrain_edge_freq[ter_u] += 1;
			}
			if (lyr_rr > lyr_r) {
				if (ter_rr >= m_terrain_edge_freq.size())
					m_terrain_edge_freq.resize(ter_rr + 1);
				m_terrain_edge_freq[ter_rr] += 1;
			}
			if ((lyr_l > lyr_d) || (lyr_l == lyr_d && ter_l != ter_d)) {
				if (ter_l >= m_terrain_edge_freq.size())
					m_terrain_edge_freq.resize(ter_l + 1);
				m_terrain_edge_freq[ter_l] += 1;
			}
			if (lyr_dd > lyr_d) {
				if (ter_dd >= m_terrain_edge_freq.size())
					m_terrain_edge_freq.resize(ter_dd + 1);
				m_terrain_edge_freq[ter_dd] += 1;
			}
		}
	}

	uint nrtriangles = 0;
	m_terrain_edge_freq_cum.resize(m_terrain_edge_freq.size());
	for (Terrain_Index ter = 0; ter < m_terrain_edge_freq.size(); ++ter) {
		m_terrain_edge_freq_cum[ter] = nrtriangles;
		nrtriangles += m_terrain_edge_freq[ter];
	}

	if (3 * nrtriangles > m_edge_vertices_size) {
		m_edge_vertices.reset(new edgefuzzvertex[3 * nrtriangles]);
		m_edge_vertices_size = 3 * nrtriangles;
	}

	std::vector<uint> indexs;
	indexs.resize(m_terrain_edge_freq_cum.size());
	for (Terrain_Index ter = 0; ter < m_terrain_edge_freq.size(); ++ter)
		indexs[ter] = 3 * m_terrain_edge_freq_cum[ter];

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);

			Terrain_Index ter_d = fcoords.field->get_terrains().d;
			Terrain_Index ter_r = fcoords.field->get_terrains().r;
			Terrain_Index ter_u = map.tr_n(fcoords).field->get_terrains().d;
			Terrain_Index ter_rr = map.r_n(fcoords).field->get_terrains().d;
			Terrain_Index ter_l = map.l_n(fcoords).field->get_terrains().r;
			Terrain_Index ter_dd = map.bl_n(fcoords).field->get_terrains().r;
			int32_t lyr_d = world.get_ter(ter_d).edge_fuzz_layer();
			int32_t lyr_r = world.get_ter(ter_r).edge_fuzz_layer();
			int32_t lyr_u = world.get_ter(ter_u).edge_fuzz_layer();
			int32_t lyr_rr = world.get_ter(ter_rr).edge_fuzz_layer();
			int32_t lyr_l = world.get_ter(ter_l).edge_fuzz_layer();
			int32_t lyr_dd = world.get_ter(ter_dd).edge_fuzz_layer();

			Coords f(fx, fy);
			Coords rn(fx + 1, fy);
			Coords brn(fx + (fy & 1), fy + 1);
			Coords bln(brn.x - 1, brn.y);

			// Hack texture coordinates to avoid wrap-around
			static const float TyZero = 1.0 / TEXTURE_HEIGHT;
			static const float TyOne = 1.0 - TyZero;

			uint index;
			if (lyr_r > lyr_d) {
				index = indexs[ter_r];
				compute_basevertex(f, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(bln, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.5;
				m_edge_vertices[index].edgey = TyOne;
				++index;
				compute_basevertex(brn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 1.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				indexs[ter_r] = index;
			} else if (ter_d != ter_r) {
				index = indexs[ter_d];
				compute_basevertex(f, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(brn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 1.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(rn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.5;
				m_edge_vertices[index].edgey = TyOne;
				++index;
				indexs[ter_d] = index;
			}
			if ((lyr_u > lyr_r) || (lyr_u == lyr_r && ter_u != ter_r)) {
				index = indexs[ter_u];
				compute_basevertex(f, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(brn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.5;
				m_edge_vertices[index].edgey = TyOne;
				++index;
				compute_basevertex(rn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 1.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				indexs[ter_u] = index;
			}
			if (lyr_rr > lyr_r) {
				index = indexs[ter_rr];
				compute_basevertex(f, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.5;
				m_edge_vertices[index].edgey = TyOne;
				++index;
				compute_basevertex(brn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 1.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(rn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				indexs[ter_rr] = index;
			}
			if ((lyr_l > lyr_d) || (lyr_l == lyr_d && ter_l != ter_d)) {
				index = indexs[ter_l];
				compute_basevertex(f, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 1.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(bln, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(brn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.5;
				m_edge_vertices[index].edgey = TyOne;
				++index;
				indexs[ter_l] = index;
			}
			if (lyr_dd > lyr_d) {
				index = indexs[ter_dd];
				compute_basevertex(f, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.5;
				m_edge_vertices[index].edgey = TyOne;
				++index;
				compute_basevertex(bln, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 1.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				compute_basevertex(brn, m_edge_vertices[index]);
				m_edge_vertices[index].edgex = 0.0;
				m_edge_vertices[index].edgey = TyZero;
				++index;
				indexs[ter_dd] = index;
			}
		}
	}

	for (Terrain_Index ter = 0; ter < m_terrain_edge_freq.size(); ++ter) {
		assert(indexs[ter] == 3 * (m_terrain_edge_freq_cum[ter] + m_terrain_edge_freq[ter]));
	}
}

void GameRendererGL::draw_terrain_fuzz()
{
	Map const & map = m_egbase->map();
	World const & world = map.world();

	if (m_edge_vertices_size == 0)
		return;

	glVertexPointer(2, GL_FLOAT, sizeof(edgefuzzvertex), &m_edge_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(edgefuzzvertex), &m_edge_vertices[0].tcx);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(edgefuzzvertex), &m_edge_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(edgefuzzvertex), &m_edge_vertices[0].edgex);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLuint edge = dynamic_cast<GLSurfaceTexture const &>
		(*g_gr->get_edge_texture()).get_gl_texture();
	glBindTexture(GL_TEXTURE_2D, edge);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

	for (Terrain_Index ter = 0; ter < m_terrain_freq.size(); ++ter) {
		if (!m_terrain_edge_freq[ter])
			continue;

		const Texture & texture =
				*g_gr->get_maptexture_data
					(world.terrain_descr(ter).get_texture());
		glBindTexture(GL_TEXTURE_2D, texture.getTexture());
		glDrawArrays
			(GL_TRIANGLES,
			 3 * m_terrain_edge_freq_cum[ter], 3 * m_terrain_edge_freq[ter]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
}
