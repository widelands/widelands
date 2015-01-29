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

#ifndef WL_BASE_RECT_H
#define WL_BASE_RECT_H

#include "base/point.h"

template <typename T>
struct GenericRect {
	/// Generates a degenerate Rect at (0, 0) with no height or width.
	GenericRect() : GenericRect(T(0), T(0), T(0), T(0)) {
	}

	GenericRect(const T& gx, const T& gy, const T& W, const T& H) : x(gx), y(gy), w(W), h(H) {
	}

	template <typename PointType>
	GenericRect(const GenericPoint<PointType>& p, const T& width, const T& height)
	   : GenericRect(T(p.x), T(p.y), width, height) {
	}

	/// The Point (x, y).
	GenericPoint<T> origin() const {
		return GenericPoint<T>(x, y);
	}

	/// The point (x + w, y + h).
	GenericPoint<T> opposite_of_origin() const {
		return GenericPoint<T>(x + w, y + h);
	}

	/// Returns true if this rectangle contains the given point.
	/// The bottom and right borders of the rectangle are considered to be excluded.
	template <typename PointType> bool contains(const GenericPoint<PointType>& pt) const {
		return T(pt.x) >= x && T(pt.x) < x + w && T(pt.y) >= y && T(pt.y) < y + h;
	}

	T x, y, w, h;
};

using Rect = GenericRect<int>;
using FloatRect = GenericRect<float>;

#endif  // end of include guard: WL_BASE_RECT_H
