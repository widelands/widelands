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

struct Rect {
	/// Generates a degenerate Rect at (0, 0) with no height or width.
	Rect();

	Rect(int32_t x, int32_t y, uint32_t width, uint32_t height);
	Rect(const Point& p, uint32_t width, uint32_t height);

	/// The top left point of this rectangle.
	Point top_left() const;

	/// The bottom right point of this rectangle.
	Point bottom_right() const;

	/// Returns true if this rectangle contains the given point.
	/// The bottom and right borders of the rectangle are considered to be excluded.
	bool contains(const Point& pt) const;

	int32_t x, y;
	uint32_t w, h;
};

// TODO(sirver): Use a templated type for all kinds of rects.
struct FloatRect {
	float x = 0.;
	float y = 0.;
	float w = 0.;
	float h = 0.;
};

#endif  // end of include guard: WL_BASE_RECT_H
