/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef VERTEX_H
#define VERTEX_H

#include "point.h"

/// Like a point but with an additional bright factor and texture coordinates.
struct Vertex:public Point {
	Vertex() : Point (0, 0), b(0), tx(0), ty(0) {}
	Vertex
		(const int32_t vx,  const int32_t vy,
		 const int32_t vb,
		 const int32_t vtx, const int32_t vty)

		: Point(vx, vy), b(vb), tx(vtx), ty(vty)
	{}

	int32_t b, tx, ty;
};

#endif
