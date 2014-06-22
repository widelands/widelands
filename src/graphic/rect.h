/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef RECT_H
#define RECT_H

#include "point.h"

struct Rect : public Point {
	Rect() : w(0), h(0) {}
	Rect(int32_t gx, int32_t gy, uint32_t W, uint32_t H)
		: Point(gx, gy), w(W), h(H)
	{}
	Rect(const Point& p, uint32_t W, uint32_t H)
		: Point(p), w(W), h(H)
	{}
	Point bottom_right() const {return *this + Point(w, h);}

	/**
	 * Returns true if this rectangle contains the given point.
	 *
	 * The bottom and right borders of the rectangle are considered to be excluded.
	 */
	bool contains(Point pt) const {
		return
			pt.x >= x && pt.x < x + static_cast<int32_t>(w) &&
			pt.y >= y && pt.y < y + static_cast<int32_t>(h);
	}

	uint32_t w, h;
};

#endif
