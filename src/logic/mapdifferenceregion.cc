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

#include "logic/mapdifferenceregion.h"

#include "base/wexception.h"

namespace Widelands {

template <> bool MapDifferenceRegion<Area<FCoords>>::advance(const Map& map) {
	assert(1 <= direction_);
	assert(direction_ <= 6);
	if (remaining_in_edge_) {
		map.get_neighbour(area_, direction_, &area_);
		--remaining_in_edge_;
		return true;
	}
	if (!passed_corner_) {
		passed_corner_ = true;
		--direction_;
		if (!direction_) {
			direction_ = 6;
		}
		remaining_in_edge_ = area_.radius;
		return advance(map);
	}
	return false;
}
}  // namespace Widelands
