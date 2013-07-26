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

#include "graphic/render/gamerenderer_gl.h"

#include <SDL_image.h>

#include "graphic/graphic.h"
#include "graphic/render/gl_surface.h"
#include "graphic/rendertarget.h"
#include "graphic/surface_cache.h"
#include "graphic/texture.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/overlay_manager.h"

using namespace Widelands;

static const uint32_t PatchSize = 4;

GameRendererGL::GameRendererGL() :
	m_patch_vertices_size(0),
	m_patch_indices_size(0),
	m_edge_vertices_size(0),
	m_road_vertices_size(0)
{
}

GameRendererGL::~GameRendererGL()
{
}

const GLSurfaceTexture * GameRendererGL::get_dither_edge_texture(const Widelands::World & world)
{
	const std::string fname = world.basedir() + "/pics/edge.png";
	const std::string cachename = std::string("gltex#") + fname;

	if (Surface* surface = g_gr->surfaces().get(cachename))
		return dynamic_cast<GLSurfaceTexture *>(surface);

	// TODO: This duplicates code from the ImageLoader, but as we cannot convert
	// a GLSurface into another format currently, we have to eat this frog.
	FileRead fr;
	fr.fastOpen(*g_fs, fname.c_str());

	SDL_Surface * sdlsurf = IMG_Load_RW(SDL_RWFromMem(fr.Data(0), fr.GetSize()), 1);
	if (!sdlsurf)
		throw wexception("%s", IMG_GetError());

	GLSurfaceTexture * edgetexture = new GLSurfaceTexture(sdlsurf, true);
	g_gr->surfaces().insert(cachename, edgetexture, false);
	return edgetexture;
}

uint32_t GameRendererGL::patch_index(const Coords & f) const
{
	uint32_t x = f.x - m_patch_size.x;
	uint32_t y = f.y - m_patch_size.y;

	assert(x < m_patch_size.w);
	assert(y < m_patch_size.h);

	uint32_t outerx = x / PatchSize;
	uint32_t outery = y / PatchSize;
	uint32_t innerx = x % PatchSize;
	uint32_t innery = y % PatchSize;

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

void GameRendererGL::draw()
{
	m_surface = dynamic_cast<GLSurface *>(m_dst->get_surface());
	if (!m_surface)
		return;
	m_rect = m_dst->get_rect();
	m_surface_offset = m_dst_offset + m_rect + m_dst->get_offset();

	m_patch_size.x = m_minfx - 1;
	m_patch_size.y = m_minfy;
	m_patch_size.w = ((m_maxfx - m_minfx + 2 + PatchSize) / PatchSize) * PatchSize;
	m_patch_size.h = ((m_maxfy - m_minfy + 1 + PatchSize) / PatchSize) * PatchSize;

	glScissor
		(m_rect.x, m_surface->height() - m_rect.y - m_rect.h,
		 m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);

	prepare_terrain_base();
	draw_terrain_base();
	if (g_gr->caps().gl.multitexture && g_gr->caps().gl.max_tex_combined >= 2) {
		prepare_terrain_dither();
		draw_terrain_dither();
	}
	prepare_roads();
	draw_roads();
	draw_objects();

	glDisable(GL_SCISSOR_TEST);
}

template<typename vertex>
void GameRendererGL::compute_basevertex(const Coords & coords, vertex & vtx) const
{
	const Map & map = m_egbase->map();
	Coords ncoords(coords);
	map.normalize_coords(ncoords);
	FCoords fcoords = map.get_fcoords(ncoords);
	Point pix;
	MapviewPixelFunctions::get_basepix(coords, pix.x, pix.y);
	pix.y -= fcoords.field->get_height() * HEIGHT_FACTOR;
	pix += m_surface_offset;
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

void GameRendererGL::count_terrain_base(Terrain_Index ter)
{
	if (ter >= m_terrain_freq.size())
		m_terrain_freq.resize(ter + 1);
	m_terrain_freq[ter] += 1;
}

void GameRendererGL::add_terrain_base_triangle
	(Terrain_Index ter, const Coords & p1, const Coords & p2, const Coords & p3)
{
	uint32_t index = m_patch_indices_indexs[ter];
	m_patch_indices[index++] = patch_index(p1);
	m_patch_indices[index++] = patch_index(p2);
	m_patch_indices[index++] = patch_index(p3);
	m_patch_indices_indexs[ter] = index;
}

void GameRendererGL::collect_terrain_base(bool onlyscan)
{
	const Map & map = m_egbase->map();

	uint32_t index = 0;
	for (uint32_t outery = 0; outery < m_patch_size.h / PatchSize; ++outery) {
		for (uint32_t outerx = 0; outerx < m_patch_size.w / PatchSize; ++outerx) {
			for (uint32_t innery = 0; innery < PatchSize; ++innery) {
				for (uint32_t innerx = 0; innerx < PatchSize; ++innerx) {
					Coords coords
						(m_patch_size.x + outerx * PatchSize + innerx,
						 m_patch_size.y + outery * PatchSize + innery);

					if (onlyscan) {
						assert(index == patch_index(coords));
						compute_basevertex(coords, m_patch_vertices[index]);
						++index;
					}

					if (coords.x >= m_minfx && coords.y >= m_minfy && coords.x <= m_maxfx && coords.y <= m_maxfy) {
						Coords ncoords(coords);
						map.normalize_coords(ncoords);
						FCoords fcoords = map.get_fcoords(ncoords);
						Terrain_Index ter_d = fcoords.field->get_terrains().d;
						Terrain_Index ter_r = fcoords.field->get_terrains().r;

						if (onlyscan) {
							count_terrain_base(ter_d);
							count_terrain_base(ter_r);
						} else {
							Coords brn(coords.x + (coords.y & 1), coords.y + 1);
							Coords bln(brn.x - 1, brn.y);
							Coords rn(coords.x + 1, coords.y);

							add_terrain_base_triangle(ter_d, coords, bln, brn);
							add_terrain_base_triangle(ter_r, coords, brn, rn);
						}
					}
				}
			}
		}
	}
}

void GameRendererGL::prepare_terrain_base()
{
	static_assert(sizeof(basevertex) == 32, "assert(sizeof(basevertex) == 32) failed.");

	uint32_t reqsize = m_patch_size.w * m_patch_size.h;
	if (reqsize > 0x10000)
		throw wexception("Too many vertices; decrease screen resolution");

	if (reqsize > m_patch_vertices_size) {
		m_patch_vertices.reset(new basevertex[reqsize]);
		m_patch_vertices_size = reqsize;
	}

	if (m_terrain_freq.size() < 16)
		m_terrain_freq.resize(16);
	m_terrain_freq.assign(m_terrain_freq.size(), 0);

	collect_terrain_base(true);

	m_terrain_freq_cum.resize(m_terrain_freq.size());
	uint32_t nrtriangles = 0;
	for (uint32_t idx = 0; idx < m_terrain_freq.size(); ++idx) {
		m_terrain_freq_cum[idx] = nrtriangles;
		nrtriangles += m_terrain_freq[idx];
	}

	if (3 * nrtriangles > m_patch_indices_size) {
		m_patch_indices.reset(new uint16_t[3 * nrtriangles]);
		m_patch_indices_size = 3 * nrtriangles;
	}

	m_patch_indices_indexs.resize(m_terrain_freq.size());
	for (Terrain_Index ter = 0; ter < m_terrain_freq.size(); ++ter)
		m_patch_indices_indexs[ter] = 3 * m_terrain_freq_cum[ter];

	collect_terrain_base(false);

	for (Terrain_Index ter = 0; ter < m_terrain_freq.size(); ++ter) {
		assert(m_patch_indices_indexs[ter] == 3 * (m_terrain_freq_cum[ter] + m_terrain_freq[ter]));
	}
}

void GameRendererGL::draw_terrain_base()
{
	const Map & map = m_egbase->map();
	const World & world = map.world();

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

void GameRendererGL::add_terrain_dither_triangle
	(bool onlyscan, Terrain_Index ter, const Coords & edge1, const Coords & edge2, const Coords & opposite)
{
	if (onlyscan) {
		if (ter >= m_terrain_edge_freq.size())
			m_terrain_edge_freq.resize(ter + 1);
		m_terrain_edge_freq[ter] += 1;
	} else {
		static const float TyZero = 1.0 / TEXTURE_HEIGHT;
		static const float TyOne = 1.0 - TyZero;

		uint32_t index = m_terrain_edge_indexs[ter];
		compute_basevertex(edge1, m_edge_vertices[index]);
		m_edge_vertices[index].edgex = 0.0;
		m_edge_vertices[index].edgey = TyZero;
		++index;
		compute_basevertex(edge2, m_edge_vertices[index]);
		m_edge_vertices[index].edgex = 1.0;
		m_edge_vertices[index].edgey = TyZero;
		++index;
		compute_basevertex(opposite, m_edge_vertices[index]);
		m_edge_vertices[index].edgex = 0.5;
		m_edge_vertices[index].edgey = TyOne;
		++index;
		m_terrain_edge_indexs[ter] = index;
	}
}

void GameRendererGL::collect_terrain_dither(bool onlyscan)
{
	const Map & map = m_egbase->map();
	const World & world = map.world();

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
			int32_t lyr_d = world.get_ter(ter_d).dither_layer();
			int32_t lyr_r = world.get_ter(ter_r).dither_layer();
			int32_t lyr_u = world.get_ter(ter_u).dither_layer();
			int32_t lyr_rr = world.get_ter(ter_rr).dither_layer();
			int32_t lyr_l = world.get_ter(ter_l).dither_layer();
			int32_t lyr_dd = world.get_ter(ter_dd).dither_layer();

			Coords f(fx, fy);
			Coords rn(fx + 1, fy);
			Coords brn(fx + (fy & 1), fy + 1);
			Coords bln(brn.x - 1, brn.y);

			if (lyr_r > lyr_d) {
				add_terrain_dither_triangle(onlyscan, ter_r, brn, f, bln);
			} else if (ter_d != ter_r) {
				add_terrain_dither_triangle(onlyscan, ter_d, f, brn, rn);
			}
			if ((lyr_u > lyr_r) || (lyr_u == lyr_r && ter_u != ter_r)) {
				add_terrain_dither_triangle(onlyscan, ter_u, rn, f, brn);
			}
			if (lyr_rr > lyr_r) {
				add_terrain_dither_triangle(onlyscan, ter_rr, brn, rn, f);
			}
			if ((lyr_l > lyr_d) || (lyr_l == lyr_d && ter_l != ter_d)) {
				add_terrain_dither_triangle(onlyscan, ter_l, f, bln, brn);
			}
			if (lyr_dd > lyr_d) {
				add_terrain_dither_triangle(onlyscan, ter_dd, bln, brn, f);
			}
		}
	}
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
void GameRendererGL::prepare_terrain_dither()
{
	static_assert(sizeof(dithervertex) == 32, "assert(sizeof(dithervertex) == 32) failed.");

	if (m_terrain_edge_freq.size() < 16)
		m_terrain_edge_freq.resize(16);
	m_terrain_edge_freq.assign(m_terrain_edge_freq.size(), 0);

	collect_terrain_dither(true);

	uint32_t nrtriangles = 0;
	m_terrain_edge_freq_cum.resize(m_terrain_edge_freq.size());
	for (Terrain_Index ter = 0; ter < m_terrain_edge_freq.size(); ++ter) {
		m_terrain_edge_freq_cum[ter] = nrtriangles;
		nrtriangles += m_terrain_edge_freq[ter];
	}

	if (3 * nrtriangles > m_edge_vertices_size) {
		m_edge_vertices.reset(new dithervertex[3 * nrtriangles]);
		m_edge_vertices_size = 3 * nrtriangles;
	}

	m_terrain_edge_indexs.resize(m_terrain_edge_freq_cum.size());
	for (Terrain_Index ter = 0; ter < m_terrain_edge_freq.size(); ++ter)
		m_terrain_edge_indexs[ter] = 3 * m_terrain_edge_freq_cum[ter];

	collect_terrain_dither(false);

	for (Terrain_Index ter = 0; ter < m_terrain_edge_freq.size(); ++ter) {
		assert(m_terrain_edge_indexs[ter] == 3 * (m_terrain_edge_freq_cum[ter] + m_terrain_edge_freq[ter]));
	}
}

void GameRendererGL::draw_terrain_dither()
{
	const Map & map = m_egbase->map();
	const World & world = map.world();

	if (m_edge_vertices_size == 0)
		return;

	glVertexPointer(2, GL_FLOAT, sizeof(dithervertex), &m_edge_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(dithervertex), &m_edge_vertices[0].tcx);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(dithervertex), &m_edge_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer(2, GL_FLOAT, sizeof(dithervertex), &m_edge_vertices[0].edgex);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	GLuint edge = get_dither_edge_texture(world)->get_gl_texture();
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

uint8_t GameRendererGL::field_roads(const FCoords & coords) const
{
	uint8_t roads;
	const Map & map = m_egbase->map();
	if (m_player && !m_player->see_all()) {
		const Player::Field & pf = m_player->fields()[Map::get_index(coords, map.get_width())];
		roads = pf.roads | map.overlay_manager().get_road_overlay(coords);
	} else {
		roads = coords.field->get_roads();
	}
	roads |= map.overlay_manager().get_road_overlay(coords);
	return roads;
}

void GameRendererGL::prepare_roads()
{
	const Map & map = m_egbase->map();

	m_road_freq[0] = 0;
	m_road_freq[1] = 0;

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);
			uint8_t roads = field_roads(fcoords);

			for (int dir = 0; dir < 3; ++dir) {
				uint8_t road = (roads >> (2 * dir)) & Road_Mask;
				if (road >= Road_Normal && road <= Road_Busy) {
					++m_road_freq[road - Road_Normal];
				}
			}
		}
	}

	uint32_t nrquads = m_road_freq[0] + m_road_freq[1];
	if (4 * nrquads > m_road_vertices_size) {
		m_road_vertices.reset(new basevertex[4 * nrquads]);
		m_road_vertices_size = 4 * nrquads;
	}

	uint32_t indexs[2];
	indexs[0] = 0;
	indexs[1] = 4 * m_road_freq[0];

	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			Coords ncoords(fx, fy);
			map.normalize_coords(ncoords);
			FCoords fcoords = map.get_fcoords(ncoords);
			uint8_t roads = field_roads(fcoords);

			uint8_t road = (roads >> Road_East) & Road_Mask;
			if (road >= Road_Normal && road <= Road_Busy) {
				uint32_t index = indexs[road - Road_Normal];
				basevertex start, end;
				compute_basevertex(Coords(fx, fy), start);
				compute_basevertex(Coords(fx + 1, fy), end);
				m_road_vertices[index] = start;
				m_road_vertices[index].y -= 2;
				m_road_vertices[index].tcy -= 2.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = start;
				m_road_vertices[index].y += 2;
				m_road_vertices[index].tcy += 2.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].y += 2;
				m_road_vertices[index].tcy += 2.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].y -= 2;
				m_road_vertices[index].tcy -= 2.0 / TEXTURE_HEIGHT;
				++index;
				indexs[road - Road_Normal] = index;
			}

			road = (roads >> Road_SouthEast) & Road_Mask;
			if (road >= Road_Normal && road <= Road_Busy) {
				uint32_t index = indexs[road - Road_Normal];
				basevertex start, end;
				compute_basevertex(Coords(fx, fy), start);
				compute_basevertex(Coords(fx + (fy & 1), fy + 1), end);
				m_road_vertices[index] = start;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = start;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				indexs[road - Road_Normal] = index;
			}

			road = (roads >> Road_SouthWest) & Road_Mask;
			if (road >= Road_Normal && road <= Road_Busy) {
				uint32_t index = indexs[road - Road_Normal];
				basevertex start, end;
				compute_basevertex(Coords(fx, fy), start);
				compute_basevertex(Coords(fx + (fy & 1) - 1, fy + 1), end);
				m_road_vertices[index] = start;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = start;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x -= 3;
				m_road_vertices[index].tcx -= 3.0 / TEXTURE_HEIGHT;
				++index;
				m_road_vertices[index] = end;
				m_road_vertices[index].x += 3;
				m_road_vertices[index].tcx += 3.0 / TEXTURE_HEIGHT;
				++index;
				indexs[road - Road_Normal] = index;
			}
		}
	}

	assert(indexs[0] == 4 * m_road_freq[0]);
	assert(indexs[1] == 4 * nrquads);
}

void GameRendererGL::draw_roads()
{
	if (!m_road_freq[0] && !m_road_freq[1])
		return;

	GLuint rt_normal =
		dynamic_cast<const GLSurfaceTexture &>
		(g_gr->get_road_texture(Widelands::Road_Normal)).get_gl_texture();
	GLuint rt_busy =
		dynamic_cast<const GLSurfaceTexture &>
		(g_gr->get_road_texture(Widelands::Road_Busy)).get_gl_texture();

	glVertexPointer(2, GL_FLOAT, sizeof(basevertex), &m_road_vertices[0].x);
	glTexCoordPointer(2, GL_FLOAT, sizeof(basevertex), &m_road_vertices[0].tcx);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(basevertex), &m_road_vertices[0].color);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDisable(GL_BLEND);

	if (m_road_freq[0]) {
		glBindTexture(GL_TEXTURE_2D, rt_normal);
		glDrawArrays(GL_QUADS, 0, 4 * m_road_freq[0]);
	}

	if (m_road_freq[1]) {
		glBindTexture(GL_TEXTURE_2D, rt_busy);
		glDrawArrays(GL_QUADS, 4 * m_road_freq[0], 4 * m_road_freq[1]);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

