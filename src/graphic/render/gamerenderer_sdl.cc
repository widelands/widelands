/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#include "graphic/render/gamerenderer_sdl.h"

#include "graphic/render/terrain_sdl.h"
#include "graphic/rendertarget.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/player.h"
#include "wui/overlay_manager.h"


using namespace Widelands;

///This is used by rendermap to calculate the brightness of the terrain.
inline static Sint8 node_brightness
	(Widelands::Time   const gametime,
	 Widelands::Time   const last_seen,
	 Widelands::Vision const vision,
	 int8_t                  result)
{
	if      (vision == 0)
		result = -128;
	else if (vision == 1) {
		assert(last_seen <= gametime);
		Widelands::Duration const time_ago = gametime - last_seen;
		result =
			static_cast<Sint16>
			(((static_cast<Sint16>(result) + 128) >> 1)
			 *
			 (1.0 + (time_ago < 45000 ? expf(-8.46126929e-5 * time_ago) : 0)))
			-
			128;
	}

	return result;
}


void GameRendererSDL::draw()
{
	draw_terrain();
	draw_objects();
}

void GameRendererSDL::draw_terrain()
{
	if (m_player && !m_player->see_all())
		m_dst->get_surface()->fill_rect(m_dst->get_rect(), RGBAColor(0, 0, 0, 255));

	const Map & map = m_egbase->map();
	const World & world = map.world();
	uint32_t const mapwidth = map.get_width();

#define get_terrain_texture(ter) \
	(g_gr->get_maptexture_data(world.terrain_descr((ter)).get_texture()))

	int32_t dx              = m_maxfx - m_minfx + 1;
	int32_t dy              = m_maxfy - m_minfy + 1;
	int32_t linear_fy       = m_minfy;
	int32_t b_posy          = linear_fy * TRIANGLE_HEIGHT + m_dst_offset.y;

	Widelands::Time const gametime = m_egbase->get_gametime();

	while (dy--) {
		const int32_t posy = b_posy;
		b_posy += TRIANGLE_HEIGHT;
		const int32_t linear_fx = m_minfx;
		FCoords r(Coords(linear_fx, linear_fy));
		FCoords br(Coords(linear_fx + (linear_fy & 1) - 1, linear_fy + 1));
		int32_t r_posx =
			r.x * TRIANGLE_WIDTH
			+
			(linear_fy & 1) * (TRIANGLE_WIDTH / 2)
			+
			m_dst_offset.x;
		int32_t br_posx = r_posx - TRIANGLE_WIDTH / 2;

		// Calculate safe (bounded) field coordinates and get field pointers
		map.normalize_coords(r);
		map.normalize_coords(br);
		Map_Index  r_index = Map::get_index (r, mapwidth);
		r.field = &map[r_index];
		Map_Index br_index = Map::get_index(br, mapwidth);
		br.field = &map[br_index];
		FCoords tr;
		map.get_tln(r, &tr);
		Map_Index tr_index = tr.field - &map[0];

		const Texture * f_r_texture;

		if (m_player && !m_player->see_all()) {
			const Player::Field & l_pf = m_player->fields()[Map::get_index(map.l_n(r), mapwidth)];
			f_r_texture = get_terrain_texture(l_pf.terrains.r);
		} else {
			f_r_texture = get_terrain_texture(map.l_n(r).field->get_terrains().r);
		}

		uint32_t count = dx;

		while (count--) {
			const FCoords bl = br;
			const FCoords f = r;
			const int32_t f_posx = r_posx;
			const int32_t bl_posx = br_posx;
			Map_Index f_index = r_index;
			Map_Index bl_index = br_index;
			move_r(mapwidth, tr, tr_index);
			move_r(mapwidth,  r,  r_index);
			move_r(mapwidth, br, br_index);
			r_posx  += TRIANGLE_WIDTH;
			br_posx += TRIANGLE_WIDTH;

			const Texture * l_r_texture = f_r_texture;
			const Texture * f_d_texture;
			const Texture * tr_d_texture;
			uint8_t roads;
			Sint8 f_brightness;
			Sint8 r_brightness;
			Sint8 bl_brightness;
			Sint8 br_brightness;

			if (m_player && !m_player->see_all()) {
				const Player::Field & f_pf = m_player->fields()[f_index];
				const Player::Field & r_pf = m_player->fields()[r_index];
				const Player::Field & bl_pf = m_player->fields()[bl_index];
				const Player::Field & br_pf = m_player->fields()[br_index];
				const Player::Field & tr_pf = m_player->fields()[tr_index];

				f_r_texture = get_terrain_texture(f_pf.terrains.r);
				f_d_texture = get_terrain_texture(f_pf.terrains.d);
				tr_d_texture = get_terrain_texture(tr_pf.terrains.d);

				roads = f_pf.roads;

				f_brightness = node_brightness
				 	(gametime, f_pf.time_node_last_unseen,
				 	 f_pf.vision, f.field->get_brightness());
				r_brightness = node_brightness
				 	(gametime, r_pf.time_node_last_unseen,
				 	 r_pf.vision, r.field->get_brightness());
				bl_brightness = node_brightness
				 	(gametime, bl_pf.time_node_last_unseen,
				 	 bl_pf.vision, bl.field->get_brightness());
				br_brightness = node_brightness
				 	(gametime, br_pf.time_node_last_unseen,
				 	 br_pf.vision, br.field->get_brightness());
			} else {
				f_r_texture = get_terrain_texture(f.field->get_terrains().r);
				f_d_texture = get_terrain_texture(f.field->get_terrains().d);
				tr_d_texture = get_terrain_texture(tr.field->get_terrains().d);

				roads = f.field->get_roads();

				f_brightness = f.field->get_brightness();
				r_brightness = r.field->get_brightness();
				bl_brightness = bl.field->get_brightness();
				br_brightness = br.field->get_brightness();
			}
			roads |= map.overlay_manager().get_road_overlay(f);

			Vertex f_vert
				(f_posx, posy - f.field->get_height() * HEIGHT_FACTOR,
				 f_brightness, 0, 0);
			Vertex r_vert
				(r_posx, posy - r.field->get_height() * HEIGHT_FACTOR,
				 r_brightness, TRIANGLE_WIDTH, 0);
			Vertex bl_vert
				(bl_posx, b_posy - bl.field->get_height() * HEIGHT_FACTOR,
				 bl_brightness, 0, 64);
			Vertex br_vert
				(br_posx, b_posy - br.field->get_height() * HEIGHT_FACTOR,
				 br_brightness, TRIANGLE_WIDTH, 64);

			if (linear_fy & 1) {
				f_vert.tx += TRIANGLE_WIDTH / 2;
				r_vert.tx += TRIANGLE_WIDTH / 2;
			} else {
				f_vert.tx += TRIANGLE_WIDTH;
				r_vert.tx += TRIANGLE_WIDTH;
				bl_vert.tx += TRIANGLE_WIDTH / 2;
				br_vert.tx += TRIANGLE_WIDTH / 2;
			}

			draw_field //  Render ground
				(*m_dst,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 *tr_d_texture, *l_r_texture, *f_d_texture, *f_r_texture);
		}

		++linear_fy;
	}

#undef get_terrain_texture
}


/**
 * Draw ground textures and roads for the given parallelogram (two triangles)
 * into the bitmap.
 *
 * Vertices:
 *   - f_vert vertex of the field
 *   - r_vert vertex right of the field
 *   - bl_vert vertex bottom left of the field
 *   - br_vert vertex bottom right of the field
 *
 * Textures:
 *   - f_r_texture Terrain of the triangle right of the field
 *   - f_d_texture Terrain of the triangle under of the field
 *   - tr_d_texture Terrain of the triangle top of the right triangle ??
 *   - l_r_texture Terrain of the triangle left of the down triangle ??
 *
 *             (tr_d)
 *
 *       (f) *------* (r)
 *          / \  r /
 *  (l_r)  /   \  /
 *        /  d  \/
 *  (bl) *------* (br)
 */
void GameRendererSDL::draw_field
	(RenderTarget & dst,
	 const Vertex  &  f_vert,
	 const Vertex  &  r_vert,
	 const Vertex  & bl_vert,
	 const Vertex  & br_vert,
	 uint8_t         roads,
	 const Texture & tr_d_texture,
	 const Texture &  l_r_texture,
	 const Texture &  f_d_texture,
	 const Texture &  f_r_texture)
{
	upcast(SDLSurface, sdlsurf, dst.get_surface());
	if (sdlsurf)
	{
		sdlsurf->set_subwin(dst.get_rect());
		switch (sdlsurf->format().BytesPerPixel) {
		case 2:
			draw_field_int<Uint16>
				(*sdlsurf,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, f_r_texture);
			break;
		case 4:
			draw_field_int<Uint32>
				(*sdlsurf,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, f_r_texture);
			break;
		default:
			assert(false);
			break;
		}
		sdlsurf->unset_subwin();
	}
}
