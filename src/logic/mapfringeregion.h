/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAPFRINGEREGION_H
#define WL_LOGIC_MAPFRINGEREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that iterates over every node on the fringe of an
 * area.
 *
 * Note that the order in which nodes are returned is not guarantueed (although
 * the current implementation begins at the top left node and then moves around
 * clockwise when advance is called repeatedly).
 */
template <typename AreaType = Area<>> struct MapFringeRegion {
	MapFringeRegion(const Map& map, AreaType area)
	   : area_(area), remaining_in_phase_(area.radius), phase_(area.radius ? 6 : 0) {
		for (typename AreaType::RadiusType r = area.radius; r; --r)
			map.get_tln(area_, &area_);
	}

	const typename AreaType::CoordsType& location() const {
		return area_;
	}

	/**
	 * Moves on to the next location. The return value indicates whether the new
	 * location has not yet been reached during this iteration. Note that when
	 * the area is so large that it overlaps itself because of wrapping, the same
	 * location may be reached several times during an iteration, while advance
	 * keeps returning true. When finally advance returns false, it means that
	 * the iteration is done and location is the same as it was before the first
	 * call to advance. The iteration can then be redone by calling advance
	 * again, which will return true until it reaches the first location the next
	 * time around, and so on.
	 */
	bool advance(const Map&);

	/**
	 * When advance has returned false, iterating over the same fringe again is
	 * not the only possibility. It is also possible to call extend. This makes
	 * the region ready to iterate over the next layer of nodes.
	 */
	void extend(const Map& map) {
		map.get_tln(area_, &area_);
		++area_.radius;
		remaining_in_phase_ = area_.radius;
		phase_ = 6;
	}

	typename AreaType::RadiusType radius() const {
		return area_.radius;
	}

private:
	AreaType area_;
	typename AreaType::RadiusType remaining_in_phase_;
	uint8_t phase_;
};

// Forward declarations of template instantiations
template <> bool MapFringeRegion<Area<FCoords>>::advance(const Map& map);
template <> bool MapFringeRegion<Area<>>::advance(const Map& map);
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAPFRINGEREGION_H
