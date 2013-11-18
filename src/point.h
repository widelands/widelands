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

#ifndef POINT_H
#define POINT_H

#include <limits>

#include <stdint.h>

struct Point {
	Point() : x(0), y(0) {}
	Point(const int32_t px, const int32_t py) : x(px), y(py) {}

	static Point invalid() {
		return
			Point
				(std::numeric_limits<int32_t>::max(),
				 std::numeric_limits<int32_t>::max());
	}

	bool operator== (const Point & other) const {
		return x == other.x and y == other.y;
	}
	bool operator!= (const Point & other) const {
		return not (*this == other);
	}

	Point   operator+  (const Point & other) const {
		return Point(x + other.x, y + other.y);
	}
	Point   operator-  () const {
		return Point(-x, -y);
	}
	Point   operator-  (const Point & other) const {
		return Point(x - other.x, y - other.y);
	}
	Point & operator+= (const Point & other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Point & operator-= (const Point & other) {
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
inline Point middle(const Point & a, const Point & b)
{
	return Point((a.x + b.x) >> 1, (a.y + b.y) >> 1);
}

#endif
