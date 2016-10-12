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

#ifndef WL_BASE_VECTOR_H
#define WL_BASE_VECTOR_H

#include <cmath>

// NOCOM(#sirver): move to point.h and ranem point to vector.
struct Vector3f {
	Vector3f(const float X, const float Y, const float Z) : x(X), y(Y), z(Z) {
	}

	void normalize() {
		const float f = static_cast<float>(std::sqrt(x * x + y * y + z * z));
		if (std::fabs(f) < 0.00001f)  // check for ==0
			return;
		x /= f;
		y /= f;
		z /= f;
	}

	// vector addition
	Vector3f operator+(const Vector3f& other) const {
		return Vector3f(x + other.x, y + other.y, z + other.z);
	}

	// inner product
	float dot(const Vector3f& other) const {
		return x * other.x + y * other.y + z * other.z;
	}

	float x, y, z;
};

#endif  // end of include guard: WL_BASE_VECTOR_H
