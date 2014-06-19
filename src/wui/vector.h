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

#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

// hm, floats...
// tried to be faster with fixed point arithmetics
// it was, but i'll try to find other opts first
struct Vector {
	Vector() : x(0), y(0), z(0) {}
	Vector(const float X, const float Y, const float Z) : x(X), y(Y), z(Z) {}

	void normalize() {
		const float f = static_cast<float>(sqrt(x * x + y * y + z * z));
		if (fabs(f) < 0.00001) // check for ==0
			return;
		x /= f;
		y /= f;
		z /= f;
	}

	// vector addition
	Vector operator+ (Vector const other) const {
		return Vector(x + other.x, y + other.y, z + other.z);
	}

	// inner product
	float operator* (Vector const other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	float x, y, z;
};

#endif
