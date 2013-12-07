/*
 * Copyright (C) 2013 by the Widelands Development Team
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



#include "point.h"

struct Vector {
	Vector() :
		x(0.f), y(0.f), z(0.f) {
	}

	Vector(float vx, float vy, float vz) :
		x(vx), y(vy), z(vz) {
	}

	Vector(Point3D p) : x(p.x), y(p.y), z(p.z) {}

	bool operator==(const Vector & other) const {
		return x == other.x and y == other.y and z == other.z;
	}
	bool operator!=(const Vector & other) const {
		return not (*this == other);
	}

	Vector operator+(const Vector & other) const {
		return Vector(x + other.x, y + other.y, z + other.z);
	}
	Vector operator-() const {
		return Vector(-x, -y, -z);
	}
	Vector operator-(const Vector & other) const {
		return Vector(x - other.x, y - other.y, z - other.z);
	}
	Vector & operator+=(const Vector & other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}
	Vector & operator-=(const Vector & other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	float  operator*(const Vector & other) {
			return x*other.x+y*other.y+z*other.z;
	}

	float  operator*(const Point3D & other) {
			return x*other.x+y*other.y+z*other.z;
	}

	Vector  operator*(const float scale) {
		return Vector(x*scale,y*scale,z*scale);
	}

	float x, y, z;
};



#endif
