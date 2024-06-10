/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#ifndef WL_LOGIC_WIDELANDS_GEOMETRY_H
#define WL_LOGIC_WIDELANDS_GEOMETRY_H

#include <cstdint>
#include <tuple>

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
// TODO(sirver): This should go away and be replaced by Vector2i16.
struct Coords {
	Coords() = default;
	Coords(int16_t nx, int16_t ny) : x(nx), y(ny) {
	}

	/// Returns a special value indicating invalidity.
	[[nodiscard]] static Coords null() {
		return Coords(-1, -1);
	}

	/// Unhash coordinates so they can be gotten from a container
	[[nodiscard]] static Coords unhash(uint32_t hash);

	/// Hash coordinates to use them as keys in a container
	[[nodiscard]] uint32_t hash() const;

	[[nodiscard]] bool operator==(const Coords& other) const {
		return x == other.x && y == other.y;
	}

	[[nodiscard]] bool operator!=(const Coords& other) const {
		return !(*this == other);
	}

	[[nodiscard]] bool operator<(const Coords& other) const {
		return std::forward_as_tuple(y, x) < std::forward_as_tuple(other.y, other.x);
	}

	[[nodiscard]] bool valid() const {
		return *this != null();
	}

	// Move the coords to the 'new_origin'.
	void reorigin(Coords new_origin, const Extent& extent);

	int16_t x{0};
	int16_t y{0};
};
static_assert(sizeof(Coords) == 4, "assert(sizeof(Coords) == 4) failed.");

/** The total number of fields in an area with the given radius, */
constexpr inline uint32_t calc_area_size(const uint32_t radius) {
	return 3 * radius * (radius + 1) + 1;
}

template <typename CT = Coords, typename RT = uint16_t> struct Area : public CT {
	using CoordsType = CT;
	using RadiusType = RT;
	Area() = default;
	Area(const CoordsType center, const RadiusType rad) : CoordsType(center), radius(rad) {
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

class Field;

struct FCoords : public Coords {
	FCoords() = default;
	FCoords(const Coords& nc, Field* const nf) : Coords(nc), field(nf) {
	}
	Field* field{nullptr};
};

enum class TriangleIndex {
	D,  // Downward
	R   // Right
};

// This uniquely indexes a single Triangle on the map. A Triangle is identified
// by its owning node and the triangle index (down or right).
template <typename CoordsType = Coords> struct TCoords {
	TCoords(const CoordsType C, const TriangleIndex T) : node(C), t(T) {
	}

	bool operator==(const TCoords& other) const {
		return node == other.node && t == other.t;
	}
	bool operator!=(const TCoords& other) const {
		return !(*this == other);
	}
	bool operator<(const TCoords& other) const {
		return std::forward_as_tuple(node, t) < std::forward_as_tuple(other.node, other.t);
	}

	CoordsType node;
	TriangleIndex t;
};

// A pair of a coord and a triangle, used to signify which field and which
// triangle the cursor is closest to. The triangle might belong to another
// field.
template <typename NodeCoordsType = Coords, typename TriangleCoordsType = Coords>
struct NodeAndTriangle {
	NodeCoordsType node;
	TCoords<TriangleCoordsType> triangle;
};

// A height interval.
struct HeightInterval {
	HeightInterval(const uint8_t Min, const uint8_t Max) : min(Min), max(Max) {
	}
	[[nodiscard]] bool valid() const {
		return min <= max;
	}

	uint8_t min, max;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_WIDELANDS_GEOMETRY_H
