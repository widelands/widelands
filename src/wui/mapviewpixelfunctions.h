/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_MAPVIEWPIXELFUNCTIONS_H
#define WL_WUI_MAPVIEWPIXELFUNCTIONS_H

#include "base/vector.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/widelands_geometry.h"
#include "wui/mapviewpixelconstants.h"

namespace MapviewPixelFunctions {

Vector2f calc_pix_difference(const Widelands::Map&, Vector2f, Vector2f);
float calc_pix_distance(const Widelands::Map&, Vector2f, Vector2f);

inline uint32_t get_map_end_screen_x(const Widelands::Map& map) {
	return map.get_width() * kTriangleWidth;
}

inline uint32_t get_map_end_screen_y(const Widelands::Map& map) {
	return map.get_height() * kTriangleHeight;
}

inline Vector2f
map_to_panel(const Vector2f& viewpoint, const float zoom, const Vector2f& map_pixel) {
	return map_pixel / zoom - viewpoint / zoom;
}

inline Vector2f
panel_to_map(const Vector2f& viewpoint, const float zoom, const Vector2f& panel_pixel) {
	return panel_pixel * zoom + viewpoint;
}

/**
 * Calculate the coordinates of the triangle the given point in pixels is in.
 *
 * Also chose 1 node that the point is close to. This may not always be the
 * closest node, because it is calculated in a simple way. It only choses among
 * the 2 neighbouring nodes whose screen coordinates define the rectangle that
 * the point is in. But this should be fully correct for all but the most
 * bizarre triangle shapes, and acceptable even for them.
 */
Widelands::NodeAndTriangle<> calc_node_and_triangle(const Widelands::Map&, uint32_t x, uint32_t y);

void normalize_pix(const Widelands::Map& map, Vector2f* p);

// Calculate the on-screen position of the node without taking height into
// account.
inline Vector2f to_map_pixel_ignoring_height(const Widelands::Coords& c) {
	return Vector2f(
	   c.x * kTriangleWidth + (c.y & 1) * (kTriangleWidth / 2.f), c.y * kTriangleHeight);
}

/**
 * Calculate the on-screen position of the node.
 */
inline Vector2f to_map_pixel(const Widelands::FCoords& fc) {
	Vector2f p = to_map_pixel_ignoring_height(fc);
	p.y -= fc.field->get_height() * kHeightFactor;
	return p;
}

inline Vector2f to_map_pixel(const Widelands::Map& map, const Widelands::Coords& c) {
	return to_map_pixel(map.get_fcoords(c));
}

// fx and fy might be out of range, must be normalized for the field
// theres no need for such a function for FCoords, since x, y out of range
// but field valid doesn't make sense
inline Vector2f to_map_pixel_with_normalization(const Widelands::Map& map,
                                                const Widelands::Coords& c) {
	Widelands::Coords c1 = c;
	map.normalize_coords(c1);
	Widelands::FCoords fc = map.get_fcoords(c1);
	fc.x = c.x;
	fc.y = c.y;
	return to_map_pixel(fc);
}

}  // namespace MapviewPixelFunctions

#endif  // end of include guard: WL_WUI_MAPVIEWPIXELFUNCTIONS_H
