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

#ifndef WL_BASE_VECTOR_H
#define WL_BASE_VECTOR_H

#include <cmath>
#include <limits>

template <typename T> struct Vector2 {
	Vector2(const T& px, const T& py) : x(px), y(py) {
	}
	Vector2() = delete;

	static Vector2 zero() {
		return Vector2(T(0), T(0));
	}

	// Returns an invalid point.
	static Vector2 invalid() {
		return Vector2(std::numeric_limits<T>::max(), std::numeric_limits<T>::max());
	}

	bool operator==(const Vector2& other) const {
		return x == other.x && y == other.y;
	}
	bool operator!=(const Vector2& other) const {
		return !(*this == other);
	}

	Vector2 operator+(const Vector2& other) const {
		return Vector2(x + other.x, y + other.y);
	}

	Vector2 operator*(const float a) const {
		return Vector2(a * x, a * y);
	}

	Vector2 operator/(const float a) const {
		return Vector2(x / a, y / a);
	}

	Vector2 operator-() const {
		return Vector2(-x, -y);
	}

	Vector2 operator-(const Vector2& other) const {
		return Vector2(x - other.x, y - other.y);
	}

	Vector2& operator+=(const Vector2& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	Vector2& operator-=(const Vector2& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	template <typename Type> Vector2<Type> cast() const {
		return Vector2<Type>(Type(x), Type(y));
	}

	T x, y;
};

using Vector2i = Vector2<int>;
using Vector2f = Vector2<float>;

/// Returns the point in the middle between a and b.
Vector2f middle(const Vector2f& a, const Vector2f& b);

// Returns an point rounded to the closest integer.
Vector2i round(const Vector2f& a);

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
