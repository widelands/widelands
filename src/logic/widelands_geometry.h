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

#ifndef WL_LOGIC_WIDELANDS_GEOMETRY_H
#define WL_LOGIC_WIDELANDS_GEOMETRY_H

#include <cmath>
#include <tuple>

#include <stdint.h>

namespace Widelands {

using MapIndex = uint32_t;

struct Extent {
	Extent(uint16_t const W, uint16_t const H) : w(W), h(H) {
	}
	uint16_t w, h;
};

/**
 * Structure used to store map coordinates
 */
struct Coords {
	Coords();
	Coords(int16_t nx, int16_t ny);

	/// Returns a special value indicating invalidity.
	static Coords null();

	/// Unhash coordinates so they can be gotten from a container
	static Coords unhash(uint32_t hash);

	/// Hash coordinates to use them as keys in a container
	uint32_t hash() const;

	bool operator==(const Coords& other) const;
	bool operator!=(const Coords& other) const;
	operator bool() const;

	// Ordering functor for use with std:: containers.
	struct OrderingFunctor {
		bool operator()(const Coords& a, const Coords& b) const {
			return std::forward_as_tuple(a.y, a.x) < std::forward_as_tuple(b.y, b.x);
		}
	};

	// Move the coords to the 'new_origin'.
	void reorigin(Coords new_origin, const Extent& extent);

	int16_t x;
	int16_t y;
};
static_assert(sizeof(Coords) == 4, "assert(sizeof(Coords) == 4) failed.");

template <typename _CoordsType = Coords, typename _RadiusType = uint16_t>
struct Area : public _CoordsType {
	using CoordsType = _CoordsType;
	using RadiusType = _RadiusType;
	Area() {
	}
	Area(const CoordsType center, const RadiusType Radius) : CoordsType(center), radius(Radius) {
	}

	bool operator==(const Area& other) const {
		return CoordsType::operator==(other) && radius == other.radius;
	}
	bool operator!=(const Area& other) const {
		return CoordsType::operator!=(other) || radius != other.radius;
	}

	RadiusType radius;
};

template <typename AreaType = Area<>> struct HollowArea : public AreaType {
	HollowArea(const AreaType area, const typename AreaType::RadiusType Hole_Radius)
	   : AreaType(area), hole_radius(Hole_Radius) {
	}

	bool operator==(const HollowArea& other) const {
		return AreaType::operator==(other) && hole_radius == other.hole_radius;
	}
	bool operator!=(const HollowArea& other) const {
		return !(*this == other);
	}

	typename AreaType::RadiusType hole_radius;
};

struct Field;

struct FCoords : public Coords {
	FCoords() : field(nullptr) {
	}
	FCoords(const Coords& nc, Field* const nf) : Coords(nc), field(nf) {
	}

	/**
	 * Used in RenderTarget::rendermap where this is first called, then the
	 * coordinates are normalized and after that field is set.
	 *
	 * \note You really want to use \ref Map::get_fcoords instead.
	 */
	explicit FCoords(const Coords& nc) : Coords(nc), field(nullptr) {
	}

	Field* field;
};

template <typename CoordsType = Coords> struct TCoords : public CoordsType {
	enum TriangleIndex { D, R, None };

	TCoords() : t() {
	}
	TCoords(const CoordsType C, const TriangleIndex T = None) : CoordsType(C), t(T) {
	}

	bool operator==(const TCoords& other) const {
		return CoordsType::operator==(other) && t == other.t;
	}
	bool operator!=(const TCoords& other) const {
		return CoordsType::operator!=(other) || t != other.t;
	}

	TriangleIndex t;
};

template <typename NodeCoordsType = Coords, typename TriangleCoordsType = Coords>
struct NodeAndTriangle {
	NodeAndTriangle() {
	}
	NodeAndTriangle(const NodeCoordsType Node, const TCoords<TriangleCoordsType>& Triangle)

	   : node(Node), triangle(Triangle) {
	}

	bool operator==(const NodeAndTriangle<>& other) const {
		return node == other.node && triangle == other.triangle;
	}
	bool operator!=(const NodeAndTriangle<>& other) const {
		return !(*this == other);
	}

	NodeCoordsType node;
	TCoords<TriangleCoordsType> triangle;
};

// A height interval.
struct HeightInterval {
	HeightInterval(const uint8_t Min, const uint8_t Max) : min(Min), max(Max) {
	}
	bool valid() const {
		return min <= max;
	}

	uint8_t min, max;
};
}

#endif  // end of include guard: WL_LOGIC_WIDELANDS_GEOMETRY_H
