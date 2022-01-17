/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "graphic/gl/fields_to_draw.h"

#include <cstdlib>

#include "base/log.h"
#include "graphic/gl/coordinate_conversion.h"
#include "logic/map_objects/world/terrain_description.h"
#include "wui/mapviewpixelfunctions.h"

/*
 * Explanation of how drawing works:
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
 *
 * Each field (f) owns two triangles: (r)ight & (d)own. When we look at the
 * field, we have to make sure to schedule drawing the triangles. This is done
 * by TerrainProgram.
 *
 * To draw dithered edges, we have to look at the neighboring triangles for the
 * two triangles too: If a neighboring triangle has another texture and our
 * dither layer is smaller, we have to draw a dithering triangle too - this lets
 * the neighboring texture bleed into our triangle.
 *
 * The dither triangle is the triangle that should be partially drawn (either r or
 * d). Example: if r and d have different textures and r.dither_layer >
 * d.dither_layer, then we will repaint d with the dither texture as mask.
 */

namespace {

// Returns the brightness value in [0, 1.] for 'fcoords'.
float field_brightness(const Widelands::FCoords& fcoords) {
	uint32_t brightness = 144 + fcoords.field->get_brightness();
	brightness = std::min<uint32_t>(255, (brightness * 255) / 160);
	return brightness / 255.;
}

}  // namespace

void FieldsToDraw::reset(const Widelands::EditorGameBase& egbase,
                         const Vector2f& viewpoint,
                         const float zoom,
                         RenderTarget* dst) {
	assert(viewpoint.x >= 0);  // divisions involving negative numbers are bad
	assert(viewpoint.y >= 0);
	assert(dst->get_offset().x <= 0);
	assert(dst->get_offset().y <= 0);

	min_fx_ = std::floor(viewpoint.x / kTriangleWidth);
	min_fy_ = std::floor(viewpoint.y / kTriangleHeight);

	// If a view window is partially moved outside of the display, its 'rect' is
	// adjusted to be fully contained on the screen - i.e. x = 0 and width is
	// made smaller. Its offset is made negative to account for this change.
	// To figure out which fields we need to draw, we have to add the absolute
	// value of 'offset' to the actual dimension of the 'rect' to get to desired
	// dimension of the 'rect'
	const Vector2f br_map = MapviewPixelFunctions::panel_to_map(
	   viewpoint, zoom,
	   Vector2f(dst->get_rect().w + std::abs(dst->get_offset().x),
	            dst->get_rect().h + std::abs(dst->get_offset().y)));
	max_fx_ = std::ceil(br_map.x / kTriangleWidth);
	max_fy_ = std::ceil(br_map.y / kTriangleHeight);

	// Adjust for triangle boundary effects and for height differences.
	min_fx_ -= 2;
	max_fx_ += 2;
	min_fy_ -= 2;
	max_fy_ += 10;

	const auto& surface = dst->get_surface();
	const int surface_width = surface.width();
	const int surface_height = surface.height();

	const Widelands::Map& map = egbase.map();

	w_ = max_fx_ - min_fx_ + 1;
	h_ = max_fy_ - min_fy_ + 1;
	assert(w_ > 0);
	assert(h_ > 0);

	// Ensure that there is enough memory for the resize operation
	size_t dimension = w_ * h_;
	const size_t max_dimension = fields_.max_size();
	if (dimension > max_dimension) {
		log_warn_time(egbase.get_gametime(),
		              "Not enough memory allocated to redraw the whole map!\nWe recommend that you "
		              "restart Widelands\n");
		dimension = max_dimension;
	}
	// Now resize the vector
	if (fields_.size() != dimension) {
		fields_.resize(dimension);
	}

	for (int32_t fy = min_fy_; fy <= max_fy_; ++fy) {
		for (int32_t fx = min_fx_; fx <= max_fx_; ++fx) {
			FieldsToDraw::Field& f = fields_[calculate_index(fx, fy)];

			f.geometric_coords = Widelands::Coords(fx, fy);

			f.ln_index = calculate_index(fx - 1, fy);
			f.rn_index = calculate_index(fx + 1, fy);
			f.trn_index = calculate_index(fx + (fy & 1), fy - 1);
			f.bln_index = calculate_index(fx + (fy & 1) - 1, fy + 1);
			f.brn_index = calculate_index(fx + (fy & 1), fy + 1);

			// Texture coordinates for pseudo random tiling of terrain and road
			// graphics. Since screen space X increases top-to-bottom and OpenGL
			// increases bottom-to-top we flip the y coordinate to not have
			// terrains and road graphics vertically mirrorerd.
			Vector2f map_pixel =
			   MapviewPixelFunctions::to_map_pixel_ignoring_height(f.geometric_coords);
			f.texture_coords.x = map_pixel.x / Widelands::kTextureSideLength;
			f.texture_coords.y = -map_pixel.y / Widelands::kTextureSideLength;

			Widelands::Coords normalized = f.geometric_coords;
			map.normalize_coords(normalized);
			f.fcoords = map.get_fcoords(normalized);

			map_pixel.y -= f.fcoords.field->get_height() * kHeightFactor;

			f.rendertarget_pixel = MapviewPixelFunctions::map_to_panel(viewpoint, zoom, map_pixel);
			f.gl_position = f.surface_pixel = f.rendertarget_pixel +
			                                  dst->get_rect().origin().cast<float>() +
			                                  dst->get_offset().cast<float>();
			pixel_to_gl_renderbuffer(
			   surface_width, surface_height, &f.gl_position.x, &f.gl_position.y);

			f.brightness = field_brightness(f.fcoords);

			const Widelands::PlayerNumber owned_by = f.fcoords.field->get_owned_by();
			f.owner = owned_by != 0 ? egbase.get_player(owned_by) : nullptr;
			f.is_border = f.fcoords.field->is_border();
			f.seeing = Widelands::VisibleState::kVisible;
			f.road_e = f.fcoords.field->get_road(Widelands::WALK_E);
			f.road_se = f.fcoords.field->get_road(Widelands::WALK_SE);
			f.road_sw = f.fcoords.field->get_road(Widelands::WALK_SW);
		}
	}
}
