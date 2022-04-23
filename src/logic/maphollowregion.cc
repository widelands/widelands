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

#include "logic/maphollowregion.h"

namespace Widelands {

template <>
MapHollowRegion<Area<>>::MapHollowRegion(const Map& map, const HollowArea<Area<>>& hollow_area)
   : hollow_area_(hollow_area),
     phase_(Phase::kTop),
     delta_radius_(hollow_area.radius - hollow_area.hole_radius),
     row_(0),
     rowwidth_(hollow_area.radius + 1),
     rowpos_(0),
     left_(hollow_area) {
	assert(hollow_area.hole_radius < hollow_area.radius);
	for (uint16_t r = hollow_area.radius; r != 0u; --r) {
		map.get_tln(hollow_area_, &hollow_area_);
	}
	left_ = hollow_area_;
}

template <> bool MapHollowRegion<Area<>>::advance(const Map& map) {
	if (phase_ == Phase::kNone) {
		return false;
	}
	++rowpos_;
	if (rowpos_ < rowwidth_) {
		map.get_rn(hollow_area_, &hollow_area_);
		if (((phase_ & (Phase::kUpper | Phase::kLower)) != 0) && rowpos_ == delta_radius_) {
			//  Jump over the hole.
			const uint32_t holewidth = rowwidth_ - 2 * delta_radius_;
			for (uint32_t i = 0; i < holewidth; ++i) {
				map.get_rn(hollow_area_, &hollow_area_);
			}
			rowpos_ += holewidth;
		}
	} else {
		++row_;
		if (phase_ == Phase::kTop && row_ == delta_radius_) {
			phase_ = Phase::kUpper;
		}

		// If we completed the widest, center line, switch into lower mode
		// There are radius_+1 lines in the upper "half", because the upper
		// half includes the center line.
		else if (phase_ == Phase::kUpper && row_ > hollow_area_.radius) {
			row_ = 1;
			phase_ = Phase::kLower;
		}

		if ((phase_ & (Phase::kTop | Phase::kUpper)) != 0) {
			map.get_bln(left_, &hollow_area_);
			++rowwidth_;
		} else {

			if (row_ > hollow_area_.radius) {
				phase_ = Phase::kNone;
				return true;  // early out
			}
			if (phase_ == Phase::kLower && row_ > hollow_area_.hole_radius) {
				phase_ = Phase::kBottom;
			}

			map.get_brn(left_, &hollow_area_);
			--rowwidth_;
		}

		left_ = hollow_area_;
		rowpos_ = 0;
	}

	return true;
}
}  // namespace Widelands
