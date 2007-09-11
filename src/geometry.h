/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "types.h"
#include "compile_assert.h"

#include <SDL_types.h>

#include <cmath>
#include <limits>

struct Point {
	Point() throw () {}
	Point(const int px, const int py) throw () : x(px), y(py) {}

	static Point invalid() throw () {
		return Point
			(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
	}

	bool operator==(const Point other) const throw ()
	{return x == other.x and y == other.y;}
	bool operator!=(const Point other) const  throw ()
	{return not (*this == other);}

	Point operator+(const Point other) const
		{return Point(x + other.x, y + other.y);}
	Point operator-(const Point other) const
		{return Point(x - other.x, y - other.y);}
	Point & operator+=(const Point other) {
		x += other.x;
		y += other.y;
		return *this;
	}
	Point & operator-=(const Point other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	int x, y;
};

/**
 * Returns the point in the middle between a and b (rounded to integer values).
 *
 * This may not be overflow safe as it could be. If the components of Point had
 * been unsigned, "((a^b)>>1)+(a&b)" would have worked, but they are signed.
 */
inline Point middle(const Point a, const Point b) throw ()
{return Point((a.x + b.x) >> 1, (a.y + b.y) >> 1);}


struct Rect : public Point {
	Rect() throw () {}
	Rect(const Point p, const uint W, const uint H) throw ()
		: Point(p), w(W), h(H)
	{}
	Point bottom_left() const {return *this + Point(w, h);}

	uint w, h;
};


/** struct Vertex
 *
 * This struct is like a point, but with an additional bright factor and texture
 * coordinates.
 */
struct Vertex:public Point {
	Vertex() throw () : Point (0, 0), b(0), tx(0), ty(0) {}
	Vertex
		(const int vx, const int vy, const int vb, const int vtx, const int vty)
		throw ()
		: Point(vx, vy), b(vb), tx(vtx), ty(vty)
	{}

	int b, tx, ty;
};


// hm, floats...
// tried to be faster with fixed point arithmetics
// it was, but i'll try to find other opts first
struct Vector {
	Vector() : x(0), y(0), z(0) {}
	Vector(const float X, const float Y, const float Z) : x(X), y(Y), z(Z) {}

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


typedef Sint16 Coordinate;
typedef Coordinate X_Coordinate;
typedef Coordinate Y_Coordinate;
/**
 * Structure used to store map coordinates
 */
struct Coords {
	Coords() throw () {}
	Coords(const X_Coordinate nx, const Y_Coordinate ny) throw ()
			: x(nx), y(ny)
	{}

	/// Returns a special value indicating invalidity.
	static Coords Null() throw () {return Coords(-1, -1);}

	bool operator==(const Coords other) const throw ()
	{return x == other.x and y == other.y;}
	bool operator!=(const Coords other) const throw ()
	{return not (*this == other);}

	__attribute__((deprecated)) bool is_valid () const throw ()
	{return x != -1 and y != -1;}

	bool isNull() const throw () {return *this == Null();}

	/**
	 * For use with standard containers.
	 */
	struct ordering_functor {
		bool operator()(const Coords a, const Coords b) const throw ()
		{return a.all < b.all;}
	};

	union {struct {X_Coordinate x; Y_Coordinate y;}; Uint32 all;};
};
compile_assert(sizeof(Coords) == 4);

struct Extent {
	Extent(const Uint16 W, const Uint16 H) throw () : w(W), h(H) {}
	Uint16 w, h;
};

template <typename _Coords_type = Coords, typename _Radius_type = Uint16>
struct Area : public _Coords_type
{
	typedef _Coords_type Coords_type;
	typedef _Radius_type Radius_type;
	Area() throw () {}
	Area(const Coords_type center, const Radius_type Radius) throw ()
		: Coords_type(center), radius(Radius)
	{}

	bool operator==(const Area other) const throw ()
	{return Coords_type::operator==(other) and radius == other.radius;}
	bool operator!=(const Area other) const throw ()
	{return Coords_type::operator!=(other) or  radius != other.radius;}

	Radius_type radius;
};

template <typename Area_type = Area<> > struct HollowArea : public Area_type {
	HollowArea
		(const Area_type area, const typename Area_type::Radius_type Hole_Radius)
		: Area_type(area), hole_radius(Hole_Radius)
	{}

	bool operator==(const HollowArea other) const throw ()
	{return Area_type::operator==(other) and hole_radius == other.hole_radius;}
	bool operator!=(const HollowArea other) const throw ()
	{return not (*this == other);}

	typename Area_type::Radius_type hole_radius;
};

struct Field;

struct FCoords : public Coords {
	FCoords() throw () {}
	FCoords(const Coords nc, Field * const nf) throw () : Coords(nc), field(nf)
	{}

	/**
	 * Used in RenderTarget::rendermap where this is first called, then the
	 * coordinates are normalized and after that field is set.
	 *
	 * \note You really want to use \ref Map::get_fcoords instead.
	 */
	explicit FCoords(const Coords nc) throw () : Coords(nc) {}

	Field * field;
};


template <typename Coords_type = Coords> struct TCoords : public Coords_type {
	enum TriangleIndex {D, R, None};

	TCoords() throw () {}
	TCoords(const Coords_type C, const TriangleIndex T = None) throw ()
			: Coords_type(C), t(T)
	{}

	bool operator==(const TCoords other) const throw ()
	{return Coords_type::operator==(other) and t == other.t;}
	bool operator!=(const TCoords other) const throw ()
	{return Coords_type::operator!=(other) or  t != other.t;}

	TriangleIndex t;
};


template
<typename Node_Coords_type = Coords, typename Triangle_Coords_type = Coords>
struct Node_and_Triangle {
	Node_and_Triangle() throw () {}
	Node_and_Triangle
		(const Node_Coords_type              Node,
		 const TCoords<Triangle_Coords_type> Triangle)
		throw ()
			: node(Node), triangle(Triangle)
	{}

	bool operator==(const Node_and_Triangle<> other) const throw ()
	{return node == other.node and triangle == other.triangle;}
	bool operator!=(const Node_and_Triangle<> other) const throw ()
	{return not (*this == other);}

	Node_Coords_type              node;
	TCoords<Triangle_Coords_type> triangle;
};

#endif /* GEOMETRY_H */
