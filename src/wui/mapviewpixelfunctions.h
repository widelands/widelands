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

#ifndef MAPVIEWPIXELFUNCTIONS_H
#define MAPVIEWPIXELFUNCTIONS_H

#include "logic/field.h"
#include "logic/map.h"
#include "logic/widelands_geometry.h"
#include "wui/mapviewpixelconstants.h"
#include "point.h"

namespace Widelands {class Map;}

namespace MapviewPixelFunctions {

float calc_brightness
	(int32_t const l,
	 int32_t const r,
	 int32_t const tl,
	 int32_t const tr,
	 int32_t const bl,
	 int32_t const br);

Point calc_pix_difference(const Widelands::Map &, Point, Point);
uint32_t calc_pix_distance(const Widelands::Map &, Point, Point);

uint32_t get_map_end_screen_x(const Widelands::Map &);
uint32_t get_map_end_screen_y(const Widelands::Map &);

/**
 * Calculate the coordinates of the triangle the given point in pixels is in.
 *
 * Also chose 1 node that the point is close to. This may not always be the
 * closest node, because it is calculated in a simple way. It only choses among
 * the 2 neighbouring nodes whose screen coordinates define the rectangle that
 * the point is in. But this should be fully correct for all but the most
 * bizarre triangle shapes, and acceptable even for them.
 */
Widelands::Node_and_Triangle<> calc_node_and_triangle
	(const Widelands::Map &, uint32_t x, uint32_t y);

void normalize_pix(const Widelands::Map &, Point & p);

/**
 * Calculate the on-screen position of the node without taking height into
 * account.
 */
void get_basepix(Widelands::Coords fc, int32_t & px, int32_t & py);

/**
 * Calculate the on-screen position of the node.
 */
void get_pix(Widelands::FCoords fc, int32_t & px, int32_t & py);

void get_pix
	(const Widelands::Map &, Widelands::Coords c, int32_t & px, int32_t & py);

void get_save_pix
	(const Widelands::Map &, Widelands::Coords c, int32_t & px, int32_t & py);
}

//  Implementation follows:
//  The rest of the content of this file is only here to be inlined. It should
//  have been in the cc file otherwise. Now objectcode modularity is not
//  achieved, but only sourcecode modularity is required.

inline uint32_t MapviewPixelFunctions::get_map_end_screen_x
	(const Widelands::Map & map)
{
	return map.get_width() * TRIANGLE_WIDTH;
}
inline uint32_t MapviewPixelFunctions::get_map_end_screen_y
	(const Widelands::Map & map)
{
	return map.get_height() * TRIANGLE_HEIGHT;
}

/*
===============
Calculate the on-screen position of the node without taking height
into account.
===============
*/
inline void MapviewPixelFunctions::get_basepix
	(Widelands::Coords const  c, int32_t & px, int32_t & py)
{
	py = c.y * TRIANGLE_HEIGHT;
	px = c.x * TRIANGLE_WIDTH + (c.y & 1) * (TRIANGLE_WIDTH / 2);
}


inline void MapviewPixelFunctions::get_pix
	(Widelands::FCoords const fc, int32_t & px, int32_t & py)
{
	get_basepix(fc, px, py);
	py -= fc.field->get_height() * HEIGHT_FACTOR;
}

inline void MapviewPixelFunctions::get_pix
	(const Widelands::Map & map, const Widelands::Coords c,
	 int32_t & px, int32_t & py)
{
	get_pix(map.get_fcoords(c), px, py);
}

// fx and fy might be out of range, must be normalized for the field
// theres no need for such a function for FCoords, since x, y out of range
// but field valid doesn't make sense
inline void MapviewPixelFunctions::get_save_pix
	(const Widelands::Map & map, Widelands::Coords const c,
	 int32_t & px, int32_t & py)
{
	Widelands::Coords c1 = c;
	map.normalize_coords(c1);
	Widelands::FCoords fc = map.get_fcoords(c1);
	fc.x = c.x;
	fc.y = c.y;
	get_pix(fc, px, py);
}

#endif
