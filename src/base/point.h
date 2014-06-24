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

#include <stdint.h>

struct Point {
	// Initializes the Point to (0,0).
	Point();
	Point(int32_t px, int32_t py);

	// Returns an invalid point.
	static Point invalid();

	bool operator == (const Point& other) const;
	bool operator != (const Point& other) const;
	Point operator + (const Point& other) const;
	Point operator - () const;
	Point operator - (const Point& other) const;
	Point& operator += (const Point& other);
	Point& operator -= (const Point& other);

	int32_t x, y;
};

/// Returns the point in the middle between a and b (rounded to integer
/// values).
Point middle(const Point& a, const Point& b);

#endif
