/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <SDL_types.h>

struct Point {
	Point() {}
	Point(const int px, const int py) : x(px), y(py) {}

	bool operator==(const Point other) const
		{return x == other.x and y == other.y;}
	bool operator!=(const Point other) const {return not (*this == other);}

	Point operator+(const Point other) const
		{return Point(x + other.x, y + other.y);}
	Point operator-(const Point other) const
		{return Point(x - other.x, y - other.y);}

	int x, y;
};


struct Rect : public Point {
	Rect() {}
	Rect(int px, int py, int pw, int ph) : Point(px, py), w(pw), h(ph) {}

	int w, h;
};


/** struct Vertex
 *
 * This struct is like a point, but with an additional bright factor and texture
 * coordinates.
 */
struct Vertex:public Point {
	Vertex() : Point (0,0), b(0), tx(0), ty(0) {}
	Vertex
	(const int vx, const int vy, const int vb, const int vtx, const int vty) :
	Point(vx,vy), b(vb), tx(vtx), ty(vty) {}

	int b, tx, ty;
};


// hm, floats...
// tried to be faster with fixed point arithmetics
// it was, but i'll try to find other opts first
struct Vector {
	Vector() : x(0), y(0), z(0) {}
	Vector(const float X, const float Y, const float Z) : x(X), y(Y), z(Z){}

	void normalize() {
		const float f = static_cast<float>(sqrt(x * x + y * y + z * z));
		if (f == 0) return;
		x /= f;
		y /= f;
		z /= f;
	}

	// vector addition
	Vector operator+(const Vector other) const
	{return Vector(x + other.x, y + other.y, z + other.z);}

	// inner product
	float operator*(const Vector other) const
		{return x * other.x + y * other.y + z * other.z;}

	float x, y, z;
};



/**
 * Structure used to store map coordinates
 */
struct Coords {
	Coords() { }
	Coords(const int nx, const int ny) : x(nx), y(ny) { }

	bool operator==(const Coords other) const
		{return x == other.x and y == other.y;}
	bool operator!=(const Coords other) const {return not (*this == other);}

	/**
	 * For use with standard containers.
	 */
	struct ordering_functor {
		bool operator()(const Coords a, const Coords b) const {
			return
				*reinterpret_cast<const Uint32 * const>(&a)
				<
				*reinterpret_cast<const Uint32 * const>(&b);
		}
	};

	int x : 16, y : 16;
};


class Field;

struct FCoords : public Coords {
	FCoords() {}
	FCoords(const Coords nc, Field * const nf) : Coords(nc), field(nf) {}
	FCoords(const int nx, const int ny, Field * const nf) :
	Coords(nx, ny), field(nf) {}

	/**
	 * Used in RenderTargetImpl::rendermap where this is first called, then
	 * the coordinates are normalized and after that field is set.
	 */
	FCoords(const int nx, const int ny) : Coords(nx, ny) {}

	Field * field;
};


struct TCoords : public Coords {
	enum TriangleIndex {D, R, None};

	TCoords() {}
	TCoords(const Coords C, const TriangleIndex T = None) : Coords(C), t(T)
	{}

	TriangleIndex t;
};


struct Node_and_Triangle {
	Node_and_Triangle() {}
	Node_and_Triangle(const Coords Node, const TCoords Triangle) :
	node(Node), triangle(Triangle) {}

	bool operator==(const Node_and_Triangle other) const
		{return node == other.node and triangle == other.triangle;}
	bool operator!=(const Node_and_Triangle other) const
		{return not (*this == other);}

	Coords node;
	TCoords triangle;
};

#endif /* GEOMETRY_H */
