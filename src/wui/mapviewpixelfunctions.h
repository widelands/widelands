/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WL_WUI_MAPVIEWPIXELFUNCTIONS_H
#define WL_WUI_MAPVIEWPIXELFUNCTIONS_H

#include "base/point.h"
#include "logic/field.h"
#include "logic/map.h"
#include "logic/widelands_geometry.h"
#include "wui/mapviewpixelconstants.h"

namespace Widelands {
class Map;
}

namespace MapviewPixelFunctions {

float calc_brightness(int32_t l, int32_t r, int32_t tl, int32_t tr, int32_t bl, int32_t br);

Point calc_pix_difference(const Widelands::Map&, Point, Point);
uint32_t calc_pix_distance(const Widelands::Map&, Point, Point);

inline uint32_t get_map_end_screen_x(const Widelands::Map& map) {
	return map.get_width() * kTriangleWidth;
}

inline uint32_t get_map_end_screen_y(const Widelands::Map& map) {
	return map.get_height() * kTriangleHeight;
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

void normalize_pix(const Widelands::Map& map, const float zoom, Point* p);

// Calculate the on-screen position of the node without taking height into
// account.
inline void get_basepix(const Widelands::Coords& c, const float zoom, int32_t* px, int32_t* py) {
	*py = c.y * kTriangleHeight * zoom;
	*px = c.x * kTriangleWidth * zoom + (c.y & 1) * (kTriangleWidth * zoom / 2);
}

/**
 * Calculate the on-screen position of the node.
 */
inline void get_pix(const Widelands::FCoords& fc, const float zoom, int32_t* px, int32_t* py) {
	get_basepix(fc, zoom, px, py);
	py -= fc.field->get_height() * kHeightFactor;
}

inline void get_pix(const Widelands::Map& map,
                    const Widelands::Coords& c,
                    const float zoom,
                    int32_t* px,
                    int32_t* py) {
	get_pix(map.get_fcoords(c), zoom, px, py);
}

// fx and fy might be out of range, must be normalized for the field
// theres no need for such a function for FCoords, since x, y out of range
// but field valid doesn't make sense
inline void get_save_pix(const Widelands::Map& map,
                         const Widelands::Coords& c,
                         const float zoom,
                         int32_t* px,
                         int32_t* py) {
	Widelands::Coords c1 = c;
	map.normalize_coords(c1);
	Widelands::FCoords fc = map.get_fcoords(c1);
	fc.x = c.x;
	fc.y = c.y;
	get_pix(fc, zoom, px, py);
}

}  // namespace MapviewPixelFunctions

#endif  // end of include guard: WL_WUI_MAPVIEWPIXELFUNCTIONS_H
