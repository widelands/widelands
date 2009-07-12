/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WIDELANDS_GEOMETRY_H
#define WIDELANDS_GEOMETRY_H

#include "compile_assert.h"

#include <cmath>
#include <stdint.h>

namespace Widelands {

typedef uint32_t Map_Index;

struct Extent {
	Extent(uint16_t const W, uint16_t const H) throw () : w(W), h(H) {}
	uint16_t w, h;
};

typedef int16_t Coordinate;
typedef Coordinate X_Coordinate;
typedef Coordinate Y_Coordinate;
/**
 * Structure used to store map coordinates
 */
struct Coords {
	explicit Coords() throw () {}
	Coords(const X_Coordinate nx, const Y_Coordinate ny) throw ()
		: x(nx), y(ny)
	{}

	/// Returns a special value indicating invalidity.
	static Coords Null() throw () {return Coords(-1, -1);}

	bool operator== (Coords const other) const throw () {
		return x == other.x and y == other.y;
	}
	bool operator!= (Coords const other) const throw () {
		return not (*this == other);
	}

	operator bool() const throw () {return *this != Null();}

	/**
	 * For use with standard containers.
	 */
	struct ordering_functor {
		bool operator()(Coords const a, Coords const b) const {
			return a.all < b.all;
		}
	};

	void reorigin(Coords new_origin, Extent const extent) {
		if (*this) {
			if (y < new_origin.y)
				y += extent.h;
			y -= new_origin.y;
			if (y & 1 and new_origin.y & 1 and ++new_origin.x == extent.w)
				new_origin.x = 0;
			if (x < new_origin.x)
				x += extent.w;
			x -= new_origin.x;
		}
	}

	union {struct {X_Coordinate x; Y_Coordinate y;}; uint32_t all;};
};
compile_assert(sizeof(Coords) == 4);

template <typename _Coords_type = Coords, typename _Radius_type = uint16_t>
struct Area : public _Coords_type
{
	typedef _Coords_type Coords_type;
	typedef _Radius_type Radius_type;
	Area() throw () {}
	Area(const Coords_type center, const Radius_type Radius) throw ()
		: Coords_type(center), radius(Radius)
	{}

	bool operator== (Area const other) const throw () {
		return Coords_type::operator== (other) and radius == other.radius;
	}
	bool operator!= (Area const other) const throw () {
		return Coords_type::operator!= (other) or  radius != other.radius;
	}

	Radius_type radius;
};

template <typename Area_type = Area<> > struct HollowArea : public Area_type {
	HollowArea
		(const Area_type area, const typename Area_type::Radius_type Hole_Radius)
		: Area_type(area), hole_radius(Hole_Radius)
	{}

	bool operator== (HollowArea const other) const throw () {
		return
			Area_type::operator== (other) and hole_radius == other.hole_radius;
	}
	bool operator!= (HollowArea const other) const throw () {
		return not (*this == other);
	}

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

	bool operator== (TCoords const other) const throw () {
		return Coords_type::operator== (other) and t == other.t;
	}
	bool operator!= (TCoords const other) const throw () {
		return Coords_type::operator!= (other) or  t != other.t;
	}

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

	bool operator== (Node_and_Triangle<> const other) const throw () {
		return node == other.node and triangle == other.triangle;
	}
	bool operator!= (Node_and_Triangle<> const other) const throw () {
		return not (*this == other);
	}

	Node_Coords_type              node;
	TCoords<Triangle_Coords_type> triangle;
};

};

#endif
