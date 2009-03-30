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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef POINT_H
#define POINT_H

#include <stdint.h>
#include <limits>

struct Point {
	Point() throw () {}
	Point(const int32_t px, const int32_t py) throw () : x(px), y(py) {}

	static Point invalid() throw () {
		return
			Point
			(std::numeric_limits<int32_t>::max(),
			 std::numeric_limits<int32_t>::max());
	}

	bool operator== (Point const other) const throw () {
		return x == other.x and y == other.y;
	}
	bool operator!= (Point const other) const throw () {
		return not (*this == other);
	}

	Point   operator+  (Point const other) const
		{return Point(x + other.x, y + other.y);}
	Point   operator-  (Point const other) const
		{return Point(x - other.x, y - other.y);}
	Point & operator+= (Point const other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Point & operator-= (Point const other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	int32_t x, y;
};

/// Returns the point in the middle between a and b (rounded to integer
/// values).
///
/// This may not be overflow safe as it could be. If the components of Point
/// had been unsigned, "((a^b)>>1)+(a&b)" would have worked, but they are
/// signed.
inline Point middle(const Point a, const Point b) throw ()
{
	return Point((a.x + b.x) >> 1, (a.y + b.y) >> 1);
}

#endif
