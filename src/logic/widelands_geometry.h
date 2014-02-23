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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WIDELANDS_GEOMETRY_H
#define WIDELANDS_GEOMETRY_H

#include <cmath>

#include <stdint.h>

namespace Widelands {

typedef uint32_t Map_Index;

struct Extent {
	Extent(uint16_t const W, uint16_t const H) : w(W), h(H) {}
	uint16_t w, h;
};

typedef int16_t Coordinate;
typedef Coordinate X_Coordinate;
typedef Coordinate Y_Coordinate;
/**
 * Structure used to store map coordinates
 */
struct Coords {
	explicit Coords() {}
	Coords(const X_Coordinate nx, const Y_Coordinate ny)
		: x(nx), y(ny)
	{}

	/// Returns a special value indicating invalidity.
	static Coords Null() {return Coords(-1, -1);}

	bool operator== (const Coords& other) const {
		return x == other.x and y == other.y;
	}
	bool operator!= (const Coords & other) const {
		return not (*this == other);
	}

	operator bool() const {return *this != Null();}

	/**
	 * For use with standard containers.
	 */
	struct ordering_functor {
		bool operator()(const Coords & a, const Coords & b) const {
			return a.all < b.all;
		}
	};

	void reorigin(Coords new_origin, const Extent & extent) {
		if (*this) {
			if (y < new_origin.y)
				y += extent.h;
			y -= new_origin.y;
			if ((y & 1) and (new_origin.y & 1) and ++new_origin.x == extent.w)
				new_origin.x = 0;
			if (x < new_origin.x)
				x += extent.w;
			x -= new_origin.x;
		}
	}

	union {struct {X_Coordinate x; Y_Coordinate y;}; uint32_t all;};
};
static_assert(sizeof(Coords) == 4, "assert(sizeof(Coords) == 4) failed.");

template <typename _Coords_type = Coords, typename _Radius_type = uint16_t>
struct Area : public _Coords_type
{
	typedef _Coords_type Coords_type;
	typedef _Radius_type Radius_type;
	Area() {}
	Area(const Coords_type center, const Radius_type Radius)
		: Coords_type(center), radius(Radius)
	{}

	bool operator== (const Area& other) const {
		return Coords_type::operator== (other) and radius == other.radius;
	}
	bool operator!= (const Area& other) const {
		return Coords_type::operator!= (other) or  radius != other.radius;
	}

	Radius_type radius;
};

template <typename Area_type = Area<> > struct HollowArea : public Area_type {
	HollowArea
		(const Area_type area, const typename Area_type::Radius_type Hole_Radius)
		: Area_type(area), hole_radius(Hole_Radius)
	{}

	bool operator== (const HollowArea& other) const {
		return
			Area_type::operator== (other) and hole_radius == other.hole_radius;
	}
	bool operator!= (const HollowArea& other) const {
		return not (*this == other);
	}

	typename Area_type::Radius_type hole_radius;
};

struct Field;

struct FCoords : public Coords {
	FCoords() : field(nullptr) {}
	FCoords(const Coords & nc, Field * const nf) : Coords(nc), field(nf)
	{}

	/**
	 * Used in RenderTarget::rendermap where this is first called, then the
	 * coordinates are normalized and after that field is set.
	 *
	 * \note You really want to use \ref Map::get_fcoords instead.
	 */
	explicit FCoords(const Coords & nc) : Coords(nc), field(nullptr) {}

	Field * field;
};


template <typename Coords_type = Coords> struct TCoords : public Coords_type {
	enum TriangleIndex {D, R, None};

	TCoords() : t() {}
	TCoords(const Coords_type C, const TriangleIndex T = None)
		: Coords_type(C), t(T)
	{}

	bool operator== (const TCoords& other) const {
		return Coords_type::operator== (other) and t == other.t;
	}
	bool operator!= (const TCoords& other) const {
		return Coords_type::operator!= (other) or  t != other.t;
	}

	TriangleIndex t;
};


template
<typename Node_Coords_type = Coords, typename Triangle_Coords_type = Coords>
struct Node_and_Triangle {
	Node_and_Triangle() {}
	Node_and_Triangle
		(const Node_Coords_type              Node,
		 const TCoords<Triangle_Coords_type>& Triangle)

			: node(Node), triangle(Triangle)
	{}

	bool operator== (Node_and_Triangle<> const other) const {
		return node == other.node and triangle == other.triangle;
	}
	bool operator!= (Node_and_Triangle<> const other) const {
		return not (*this == other);
	}

	Node_Coords_type              node;
	TCoords<Triangle_Coords_type> triangle;
};

}

#endif
