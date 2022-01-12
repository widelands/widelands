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

#ifndef WL_LOGIC_MAPREGION_H
#define WL_LOGIC_MAPREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that iterates over every node of an area.
 *
 * Note that the order in which fields are returned is not guaranteed.
 */
template <typename AreaType = Area<>> struct MapRegion {
	MapRegion(const Map& map, AreaType area)
	   : area_(area),
	     rowwidth_(area.radius + 1),
	     remaining_in_row_(rowwidth_),
	     remaining_rows_(rowwidth_ + area.radius) {
		for (typename AreaType::RadiusType r = area.radius; r; --r)
			map.get_tln(area_, &area_);
		left_ = area_;
	}

	const typename AreaType::CoordsType& location() const {
		return area_;
	}

	/// Moves on to the next location. The return value indicates whether the
	/// new location has not yet been reached during this iteration. Note that
	/// when the area is so large that it overlaps itself because of wrapping,
	/// the same location may be reached several times during an iteration,
	/// while advance keeps returning true. When finally advance returns false,
	/// it means that the iteration is done.
	bool advance(const Map& map) {
		if (--remaining_in_row_)
			map.get_rn(area_, &area_);
		else if (area_.radius < --remaining_rows_) {
			map.get_bln(left_, &area_);
			left_ = area_;
			remaining_in_row_ = ++rowwidth_;
		} else if (remaining_rows_) {
			map.get_brn(left_, &area_);
			left_ = area_;
			remaining_in_row_ = --rowwidth_;
		} else
			return false;
		return true;
	}

	typename AreaType::RadiusType radius() const {
		return area_.radius;
	}

private:
	AreaType area_;
	typename AreaType::CoordsType left_;
	typename AreaType::RadiusType rowwidth_;
	typename AreaType::RadiusType remaining_in_row_;
	typename AreaType::RadiusType remaining_rows_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAPREGION_H
