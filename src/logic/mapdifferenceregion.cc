/*
 * Copyright (C) 2007, 2010-2013 by the Widelands Development Team
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

#include "logic/mapdifferenceregion.h"

#include "base/wexception.h"

namespace Widelands {

template <> bool MapDifferenceRegion<Area<FCoords> >::advance(const Map & map)
{
	assert(1 <= direction_);
	assert     (direction_ <= 6);
	if (remaining_in_edge_) {
		map.get_neighbour(area_, direction_, &area_);
		--remaining_in_edge_;
		return true;
	} else {
		if (!passed_corner_) {
			passed_corner_ = true;
			--direction_; if (!direction_) direction_ = 6;
			remaining_in_edge_ = area_.radius;
			return advance(map);
		}
	}
	return false;
}

template <>
void MapDifferenceRegion<Area<FCoords> >::move_to_other_side(const Map & map)
{
	assert(1 <= direction_);
	assert     (direction_ <= 6);
	assert(passed_corner_);
	--direction_; if (!direction_) direction_ = 6;
	Area<FCoords>::RadiusType steps_left = area_.radius + 1;
	switch (direction_) {
#define DIRECTION_CASE(dir, neighbour_function)                               \
   case dir:                                                                  \
      for (; steps_left; --steps_left)                                        \
			map.neighbour_function(area_, &area_);                               \
      break;                                                                  \

	DIRECTION_CASE(WALK_NW, get_tln);
	DIRECTION_CASE(WALK_NE, get_trn);
	DIRECTION_CASE(WALK_E,  get_rn);
	DIRECTION_CASE(WALK_SE, get_brn);
	DIRECTION_CASE(WALK_SW, get_bln);
	DIRECTION_CASE(WALK_W,  get_ln);
	default:
			NEVER_HERE();
	}
	--direction_; if (!direction_) direction_ = 6;
	remaining_in_edge_ = area_.radius;
	passed_corner_     = false;
}

}
