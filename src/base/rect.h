/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_BASE_RECT_H
#define WL_BASE_RECT_H

#include "base/vector.h"

template <typename T> struct Rect {
	/// Generates a degenerate Recti at (0, 0) with no height or width.
	Rect() : Rect(T(0), T(0), T(0), T(0)) {
	}

	Rect(const T& gx, const T& gy, const T& W, const T& H) : x(gx), y(gy), w(W), h(H) {
	}

	template <typename PointType>
	Rect(const Vector2<PointType>& p, const T& width, const T& height)
	   : Rect(T(p.x), T(p.y), width, height) {
	}

	/// The Vector2i (x, y).
	Vector2<T> origin() const {
		return Vector2<T>(x, y);
	}

	/// The point (x + w, y + h).
	Vector2<T> opposite_of_origin() const {
		return Vector2<T>(x + w, y + h);
	}

	/// Returns true if this rectangle contains the given point.
	/// The bottom and right borders of the rectangle are considered to be excluded.
	template <typename PointType> bool contains(const Vector2<PointType>& pt) const {
		return T(pt.x) >= x && T(pt.x) < x + w && T(pt.y) >= y && T(pt.y) < y + h;
	}

	// The center point of 'r'.
	Vector2f center() const {
		return Vector2f(x + w / 2.f, y + h / 2.f);
	}

	template <typename Type> Rect<Type> cast() const {
		return Rect<Type>(Type(x), Type(y), Type(w), Type(h));
	}

	T x, y, w, h;
};

using Recti = Rect<int>;
using Rectf = Rect<float>;

#endif  // end of include guard: WL_BASE_RECT_H
