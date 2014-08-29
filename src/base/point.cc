/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "base/point.h"

#include <limits>

Point::Point() : x(0), y(0) {}

Point::Point(const int32_t px, const int32_t py) : x(px), y(py) {}

Point Point::invalid() {
	return Point(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());
}

bool Point::operator == (const Point& other) const {
	return x == other.x && y == other.y;
}
bool Point::operator != (const Point& other) const {
	return !(*this == other);
}

Point Point::operator +(const Point& other) const {
	return Point(x + other.x, y + other.y);
}

Point Point::operator -() const {
	return Point(-x, -y);
}

Point Point::operator -(const Point& other) const {
	return Point(x - other.x, y - other.y);
}

Point& Point::operator += (const Point& other) {
	x += other.x;
	y += other.y;
	return *this;
}

Point& Point::operator -= (const Point& other) {
	x -= other.x;
	y -= other.y;
	return *this;
}

Point middle(const Point& a, const Point& b) {
	return Point((a.x + b.x) >> 1, (a.y + b.y) >> 1);
}
