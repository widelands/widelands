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

#include "graphic/gl/game_renderer.h"

#include <memory>

#include "graphic/gl/dither_program.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/road_program.h"
#include "graphic/gl/terrain_program.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "logic/editor_game_base.h"
#include "logic/player.h"
#include "logic/world/world.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/overlay_manager.h"

namespace {

using namespace Widelands;

// Returns the brightness value in [0, 1.] for 'fcoords' at 'gametime' for
// 'player' (which can be nullptr).
float field_brightness(const FCoords& fcoords,
                       const uint32_t gametime,
                       const Map& map,
                       const Player* const player) {
	uint32_t brightness = 144 + fcoords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);

	if (player && !player->see_all()) {
		const Player::Field& pf = player->fields()[Map::get_index(fcoords, map.get_width())];
		if (pf.vision == 0) {
			return 0.;
		} else if (pf.vision == 1) {
			static const uint32_t kDecayTimeInMs = 20000;
			const Duration time_ago = gametime - pf.time_node_last_unseen;
			if (time_ago < kDecayTimeInMs) {
				brightness = (brightness * (2 * kDecayTimeInMs - time_ago)) / (2 * kDecayTimeInMs);
			} else {
				brightness = brightness / 2;
			}
		}
	}
	return brightness / 255.;
}

// Returns the road that should be rendered here. The format is like in field,
// but this is not the physically present road, but the one that should be
// drawn (i.e. taking into account if there is fog of war involved or road
// building overlays enabled).
uint8_t field_roads(const FCoords& coords, const Map& map, const Player* const player) {
	uint8_t roads;
	if (player && !player->see_all()) {
		const Player::Field& pf = player->fields()[Map::get_index(coords, map.get_width())];
		roads = pf.roads | map.overlay_manager().get_road_overlay(coords);
	} else {
		roads = coords.field->get_roads();
	}
	roads |= map.overlay_manager().get_road_overlay(coords);
	return roads;
}

}  // namespace

// Explanation of how drawing works:
// Schematic of triangle neighborhood:
//
//               *
//              / \
//             / u \
//         (f)/     \
//    *------*------* (r)
//     \  l / \  r / \
//      \  /   \  /   \
//       \/  d  \/ rr  \
//       *------*------* (br)
//        \ dd /
//         \  /
//          \/
//          *
//
// Each field (f) owns two triangles: (r)ight & (d)own. When we look at the
// field, we have to make sure to schedule drawing the triangles. This is done
// by of these triangles is done by TerrainProgram.
//
// To draw dithered edges, we have to look at the neighboring triangles for the
// two triangles too: If a neighboring triangle has another texture and our
// dither layer is smaller, we have to draw a dithering triangle too - this lets the neighboring
// texture
// bleed into our triangle.
//
// The dither triangle is the triangle that should be partially (either r or
// d). Example: if r and d have different textures and r.dither_layer >
// d.dither_layer, then we will repaint d with the dither texture as mask.


std::unique_ptr<TerrainProgram> GlGameRenderer::terrain_program_;
std::unique_ptr<DitherProgram> GlGameRenderer::dither_program_;
std::unique_ptr<RoadProgram> GlGameRenderer::road_program_;

GlGameRenderer::GlGameRenderer()  {
}

GlGameRenderer::~GlGameRenderer() {
}

void GlGameRenderer::draw() {
	if (terrain_program_ == nullptr) {
		terrain_program_.reset(new TerrainProgram());
		dither_program_.reset(new DitherProgram());
		road_program_.reset(new RoadProgram());
	}

	Surface* surface = m_dst->get_surface();
	if (!surface)
		return;

	const Rect& bounding_rect = m_dst->get_rect();
	const Point surface_offset = m_dst_offset + bounding_rect.top_left() + m_dst->get_offset();

	glScissor(bounding_rect.x,
	          surface->height() - bounding_rect.y - bounding_rect.h,
	          bounding_rect.w,
	          bounding_rect.h);
	glEnable(GL_SCISSOR_TEST);

	Map& map = m_egbase->map();
	const uint32_t gametime = m_egbase->get_gametime();

	FieldsToDraw fields_to_draw(m_minfx, m_maxfx, m_minfy, m_maxfy);
	for (int32_t fy = m_minfy; fy <= m_maxfy; ++fy) {
		for (int32_t fx = m_minfx; fx <= m_maxfx; ++fx) {
			FieldsToDraw::Field& f =
			   *fields_to_draw.mutable_field(fields_to_draw.calculate_index(fx, fy));

			f.fx = fx;
			f.fy = fy;

			Coords coords(fx, fy);
			int x, y;
			MapviewPixelFunctions::get_basepix(coords, x, y);

			map.normalize_coords(coords);
			const FCoords& fcoords = map.get_fcoords(coords);

			f.texture_x = float(x) / kTextureWidth;
			f.texture_y = float(y) / kTextureHeight;

			f.gl_x = f.pixel_x = x + surface_offset.x;
			f.gl_y = f.pixel_y = y + surface_offset.y - fcoords.field->get_height() * HEIGHT_FACTOR;
			surface->pixel_to_gl(&f.gl_x, &f.gl_y);

			f.ter_d = fcoords.field->terrain_d();
			f.ter_r = fcoords.field->terrain_r();

			f.brightness = field_brightness(fcoords, gametime, map, m_player);

			f.roads = field_roads(fcoords, map, m_player);
		}
	}

	// NOCOM(#sirver): consolidate with base class.
	const World& world = m_egbase->world();
	terrain_program_->draw(gametime, world.terrains(), fields_to_draw);
	dither_program_->draw(gametime, world.terrains(), fields_to_draw);
	road_program_->draw(*surface, fields_to_draw);

	draw_objects();

	glDisable(GL_SCISSOR_TEST);
}
