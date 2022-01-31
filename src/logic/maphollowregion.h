/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAPHOLLOWREGION_H
#define WL_LOGIC_MAPHOLLOWREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that iterates over every node for which the
 * distance to the center point is greater than hollow_area.hole_radius and
 * at most hollow_area.radius.
 *
 * \note The order in which fields are returned is not guarantueed.
 */
template <typename AreaType = Area<>> struct MapHollowRegion {
	MapHollowRegion(const Map& map, const HollowArea<AreaType>& hollow_area);

	const typename AreaType::CoordsType& location() const {
		return hollow_area_;
	}

	/**
	 * Moves on to the next location, traversing the region by row.
	 *
	 * I hope traversing by row results in slightly better cache behaviour
	 * than other algorithms (e.g. one could also walk concentric "circles"
	 * / hexagons).
	 *
	 * \return Whether the new location has not yet been reached during this
	 * iteration.
	 *
	 * \note When the area is so large that it overlaps itself because of
	 * wrapping, the same location may be reached several times during an
	 * iteration, while advance keeps returning true. When finally advance
	 * returns false, it means that the iteration is done.
	 */
	bool advance(const Map&);

	enum class Phase {
		kNone = 0,    // not initialized or completed
		kTop = 1,     // above the hole
		kUpper = 2,   // upper half
		kLower = 4,   // lower half
		kBottom = 8,  // below the hole
	};

private:
	HollowArea<AreaType> hollow_area_;
	Phase phase_;
	const uint32_t delta_radius_;
	uint32_t row_;                        // # of rows completed in this phase
	uint32_t rowwidth_;                   // # of fields to return per row
	uint32_t rowpos_;                     // # of fields we have returned in this row
	typename AreaType::CoordsType left_;  //  left-most node of current row
};

// Forward declarations of template instantiations
template <>
MapHollowRegion<Area<>>::MapHollowRegion(const Map& map, const HollowArea<Area<>>& hollow_area);
template <> bool MapHollowRegion<Area<>>::advance(const Map& map);

// A bunch of operators that turn MapHollowRegion<Area<>>::Phase into a bitwise combinable class.
inline MapHollowRegion<Area<>>::Phase operator|(MapHollowRegion<Area<>>::Phase left,
                                                MapHollowRegion<Area<>>::Phase right) {
	return MapHollowRegion<Area<>>::Phase(static_cast<int>(left) | static_cast<int>(right));
}
inline int operator&(MapHollowRegion<Area<>>::Phase left, MapHollowRegion<Area<>>::Phase right) {
	return static_cast<int>(left) & static_cast<int>(right);
}
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAPHOLLOWREGION_H
