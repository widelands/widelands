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

#ifndef WL_BASE_POINT_H
#define WL_BASE_POINT_H

#include <limits>

#include <stdint.h>

template <typename T> struct GenericPoint {
	GenericPoint(const T& px, const T& py) : x(px), y(py) {
	}
	GenericPoint() : GenericPoint(T(0), T(0)) {
	}

	// Returns an invalid point.
	static GenericPoint invalid() {
		return GenericPoint(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
	}

	bool operator == (const GenericPoint& other) const {
		return x == other.x && y == other.y;
	}
	bool operator != (const GenericPoint& other) const {
		return !(*this == other);
	}

	GenericPoint operator + (const GenericPoint& other) const {
		return GenericPoint(x + other.x, y + other.y);
	}

	GenericPoint operator - () const {
		return GenericPoint(-x, -y);
	}

	GenericPoint operator - (const GenericPoint& other) const {
		return GenericPoint(x - other.x, y - other.y);
	}

	GenericPoint& operator += (const GenericPoint& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	GenericPoint& operator -= (const GenericPoint& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	T x, y;
};

using Point = GenericPoint<int>;
using FloatPoint = GenericPoint<float>;

/// Returns the point in the middle between a and b (rounded to integer
/// values).
Point middle(const Point& a, const Point& b);

#endif  // end of include guard: WL_BASE_POINT_H
