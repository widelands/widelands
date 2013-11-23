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

#ifndef POINT_3D_H
#define POINT_3D_H

#include <limits>

#include <stdint.h>

#include "point.h"

struct Point3D {
	Point3D() : x(0), y(0), z(0) {}
	Point3D(const int32_t px, const int32_t py, const int32_t pz) : x(px), y(py), z(pz) {}
	Point3D(Point p) : x(p.x), y(p.y), z(0) {};
	Point3D(Point p, int32_t nz) : x(p.x), y(p.y), z(nz) {};

	static Point3D invalid() {
		return
			Point3D
				(std::numeric_limits<int32_t>::max(),
				 std::numeric_limits<int32_t>::max(),
				 std::numeric_limits<int32_t>::max());
	}

	bool operator== (const Point3D & other) const {
		return x == other.x and y == other.y and z == other.z;
	}
	bool operator!= (const Point3D & other) const {
		return not (*this == other);
	}

	Point3D   operator+  (const Point3D & other) const {
		return Point3D(x + other.x, y + other.y, z + other.z);
	}
	Point3D   operator-  () const {
		return Point3D(-x, -y, -z);
	}
	Point3D   operator-  (const Point3D & other) const {
		return Point3D(x - other.x, y - other.y, z - other.z);
	}
	Point3D & operator+= (const Point3D & other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Point3D & operator-= (const Point3D & other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	int32_t x, y, z;
};

/// Returns the point in the middle between a and b (rounded to integer
/// values).
///
/// This may not be overflow safe as it could be. If the components of Point
/// had been unsigned, "((a^b)>>1)+(a&b)" would have worked, but they are
/// signed.
inline Point3D middle(const Point3D & a, const Point3D & b)
{
	return Point3D((a.x + b.x) >> 1, (a.y + b.y) >> 1, (a.z + b.z) >> 1);
}

#endif
