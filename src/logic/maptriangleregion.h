/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAPTRIANGLEREGION_H
#define WL_LOGIC_MAPTRIANGLEREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that returns every triangle which can be reached by
 * crossing at most \a radius edges.
 *
 * Each such location is returned exactly once via next(). But this does not
 * guarantee that a location is returned at most once when the radius is so
 * large that the area overlaps itself because of wrapping.
 *
 * Note that the order in which locations are returned is not guarantueed. (But
 * in fact the triangles are returned row by row from top to bottom and from
 * left to right in each row and I see no reason why that would ever change.)
 */
template <typename CoordsType = TCoords<>, typename RadiusType = uint16_t>
struct MapTriangleRegion {
	MapTriangleRegion(const Map&, CoordsType, uint16_t radius);

	const CoordsType& location() const;

	/**
	 * Moves on to the next location, traversing the region by row.
	 *
	 * \return Whether the new location has not yet been reached during this
	 * iteration.
	 *
	 * \note When the area is so large that it overlaps itself because of
	 * wrapping, the same location may be reached several times during an
	 * iteration, while advance keeps returning true. When finally advance
	 * returns false, it means that the iteration is done and location is
	 * the same as it was before the first call to advance. The iteration
	 * can then be redone by calling advance again, which will return true
	 * until it reaches the first location the next time around, and so on.
	 */
	bool advance(const Map&);
};
template <> struct MapTriangleRegion<FCoords> {
	MapTriangleRegion(const Map& map, const Area<FCoords>& area)
	   : area_(TCoords<FCoords>(area, TriangleIndex::D), area.radius + 1),
	     rowwidth_(area_.radius * 2 + 1),
	     remaining_in_row_(rowwidth_),
	     remaining_rows_(area_.radius * 2) {
		for (uint8_t r = area_.radius; r; --r)
			map.get_tln(area_.node, &area_.node);
		left_ = area_.node;
	}

	const TCoords<FCoords>& location() const {
		return area_;
	}

	bool advance(const Map& map) {
		if (--remaining_in_row_) {
			if (area_.t == TriangleIndex::D)
				area_.t = TriangleIndex::R;
			else {
				area_.t = TriangleIndex::D;
				map.get_rn(area_.node, &area_.node);
			}
		} else if (area_.radius < --remaining_rows_) {
			map.get_bln(left_, &area_.node);
			left_ = area_.node;
			area_.t = TriangleIndex::D;
			remaining_in_row_ = rowwidth_ += 2;
		} else if (remaining_rows_) {
			map.get_brn(left_, &area_.node);
			left_ = area_.node;
			area_.t = TriangleIndex::D;
			remaining_in_row_ = rowwidth_ -= 2;
		} else
			return false;
		return true;
	}

private:
	Area<TCoords<FCoords>> area_;
	FCoords left_;
	uint16_t rowwidth_ = 0;
	uint16_t remaining_in_row_ = 0;
	uint16_t remaining_rows_ = 0;
};
template <typename CoordsType> struct MapTriangleRegion<TCoords<CoordsType>> {
	MapTriangleRegion(const Map&, Area<TCoords<CoordsType>, uint16_t>);

	const TCoords<CoordsType>& location() const {
		return location_;
	}

	bool advance(const Map&);

private:
	const bool radius_is_odd_ = false;
	enum class Phase { kTop, kUpper, kLower, kBottom } phase_ = Phase::kTop;
	uint16_t remaining_rows_in_upper_phase_ = 0U;
	uint16_t remaining_rows_in_lower_phase_ = 0U;
	uint16_t row_length_, remaining_in_row_ = 0U;
	CoordsType left_;
	TCoords<CoordsType> location_;
};

// Forward declarations of template instantiations
template <> MapTriangleRegion<>::MapTriangleRegion(const Map& map, Area<TCoords<>> area);
template <> bool MapTriangleRegion<>::advance(const Map& map);

template <>
MapTriangleRegion<TCoords<FCoords>>::MapTriangleRegion(const Map& map, Area<TCoords<FCoords>> area);
template <> bool MapTriangleRegion<TCoords<FCoords>>::advance(const Map& map);
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAPTRIANGLEREGION_H
