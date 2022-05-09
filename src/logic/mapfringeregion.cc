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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/mapfringeregion.h"

#include "base/wexception.h"

namespace Widelands {

template <> bool MapFringeRegion<Area<FCoords>>::advance(const Map& map) {
	switch (phase_) {
	case 0:
		if (area_.radius != 0u) {
			remaining_in_phase_ = area_.radius;
			phase_ = 6;
			// Fallthrough
		} else {
			return false;
		}
		FALLS_THROUGH;
	case 1:
		map.get_trn(area_, &area_);
		break;
	case 2:
		map.get_tln(area_, &area_);
		break;
	case 3:
		map.get_ln(area_, &area_);
		break;
	case 4:
		map.get_bln(area_, &area_);
		break;
	case 5:
		map.get_brn(area_, &area_);
		break;
	case 6:
		map.get_rn(area_, &area_);
		break;
	default:
		NEVER_HERE();
	}

	if (--remaining_in_phase_ == 0) {
		remaining_in_phase_ = area_.radius;
		--phase_;
	}
	return phase_ != 0u;
}

template <> bool MapFringeRegion<Area<>>::advance(const Map& map) {
	switch (phase_) {
	case 0:
		if (area_.radius != 0u) {
			remaining_in_phase_ = area_.radius;
			phase_ = 6;
			// Fallthrough
		} else {
			return false;
		}
		FALLS_THROUGH;
	case 1:
		map.get_trn(area_, &area_);
		break;
	case 2:
		map.get_tln(area_, &area_);
		break;
	case 3:
		map.get_ln(area_, &area_);
		break;
	case 4:
		map.get_bln(area_, &area_);
		break;
	case 5:
		map.get_brn(area_, &area_);
		break;
	case 6:
		map.get_rn(area_, &area_);
		break;
	default:
		NEVER_HERE();
	}

	if (--remaining_in_phase_ == 0) {
		remaining_in_phase_ = area_.radius;
		--phase_;
	}
	return phase_ != 0u;
}
}  // namespace Widelands
