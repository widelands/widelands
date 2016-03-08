/*
 * Copyright (C) 2007-2008, 2010 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAPDIFFERENCEREGION_H
#define WL_LOGIC_MAPDIFFERENCEREGION_H

#include "logic/map.h"

namespace Widelands {

/// Producer/Coroutine struct that iterates over the set of nodes that is the
/// difference between the areas A and B around neighboring nodes a and b. The
/// constructor takes a Direction parameter, which is the walking direction
/// when going from a to b.
///
/// It first iterates over the set of nodes that are in A but not in B. When
/// that iteration is completed (advance has returned false),
/// move_to_other_side can be called to prepare the region for iterating over
/// the set of nodes in B but not in A. (Because of symmetry, it is after that
/// iteration again possible to call move_to_other_side to iterate over the
/// nodes that are in A but not in B again, and so on.)
///
/// \note The order in which nodes are returned is not guarantueed.
template <typename AreaType = Area<> > struct MapDifferenceRegion {
	MapDifferenceRegion
		(const Map & map, AreaType area, Direction direction)
		: area_(area), remaining_in_edge_(area.radius), passed_corner_(false)
	{
		assert(1 <= direction);
		assert     (direction <= 6);
		--direction; if (!direction) direction = 6;
		--direction; if (!direction) direction = 6;
		switch (direction) {
#define DIRECTION_CASE(dir, neighbour_function)                               \
      case dir:                                                               \
         for (; area.radius; --area.radius)                                   \
				map.neighbour_function(area_, &area_);                            \
         break;                                                               \

		DIRECTION_CASE(WALK_NW, get_tln);
		DIRECTION_CASE(WALK_NE, get_trn);
		DIRECTION_CASE(WALK_E,  get_rn);
		DIRECTION_CASE(WALK_SE, get_brn);
		DIRECTION_CASE(WALK_SW, get_bln);
		DIRECTION_CASE(WALK_W,  get_ln);
#undef DIRECTION_CASE
		}
		--direction; if (!direction) direction = 6;
		--direction; if (!direction) direction = 6;
		direction_ = direction;
	}

	typename AreaType::CoordsType & location() const {return area_;}

	/**
	 * Moves on to the next location. The return value indicates whether the new
	 * location has not yet been reached during this iteration. Note that when
	 * the area is so large that it overlaps itself because of wrapping, the same
	 * location may be reached several times during an iteration, while advance
	 * keeps returning true. When finally advance returns false, it means that
	 * the iteration is done.
	 */
	bool advance(const Map & map);

	void move_to_other_side(const Map & map);

	typename AreaType::RadiusType radius() const {return area_.radius;}
private:
	AreaType                       area_;
	typename AreaType::RadiusType remaining_in_edge_;
	bool                            passed_corner_;
	Direction                       direction_;
};

}

#endif  // end of include guard: WL_LOGIC_MAPDIFFERENCEREGION_H
