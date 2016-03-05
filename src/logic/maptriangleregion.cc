/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "logic/maptriangleregion.h"

namespace Widelands {

template <> MapTriangleRegion<>::MapTriangleRegion
	(const Map & map, Area<TCoords<> > area)
: radius_is_odd_(area.radius & 1)
{
	assert(area.t == TCoords<>::R || area.t == TCoords<>::D);
	const uint16_t radius_plus_1 = area.radius + 1;
	const uint16_t half_radius_rounded_down = area.radius / 2;
	row_length_ = radius_plus_1;
	for (uint32_t i = half_radius_rounded_down; i; --i) map.get_tln(area, &area);
	if (area.t == TCoords<>::R) {
		left_ = area;
		if (area.radius) {
			remaining_rows_in_upper_phase_ = half_radius_rounded_down + 1;
			remaining_rows_in_lower_phase_ = (area.radius - 1) / 2;
			if (radius_is_odd_) {
				map.get_trn(area, &area);
				phase_ = Top;
				row_length_ = area.radius + 2;
				remaining_in_row_ = radius_plus_1 / 2;
				area.t = TCoords<>::D;
			} else {
				phase_ = Upper;
				remaining_in_row_ = row_length_ = radius_plus_1;
				area.t = TCoords<>::R;
			}
		} else {
			assert(area.radius == 0);
			phase_ = Bottom;
			remaining_in_row_ = 0;
			area.t = TCoords<>::R;
		}
	} else {
		remaining_rows_in_upper_phase_ = radius_plus_1 / 2;
		remaining_rows_in_lower_phase_ = half_radius_rounded_down;
		if (radius_is_odd_) {
			map.get_ln(area, &area);
			left_ = area;
			phase_ = Upper;
			remaining_in_row_ = row_length_ = area.radius + 2;
			area.t = TCoords<>::R;
		} else {
			map.get_bln(area, &left_);
			phase_ = Top;
			row_length_ = area.radius + 3;
			remaining_in_row_ = half_radius_rounded_down + (0 < area.radius);
			area.t = TCoords<>::D;
		}
	}
	location_ = area;
}


template <> bool MapTriangleRegion<>::advance(const Map & map) {
	assert(remaining_in_row_ < 10000); //  Catch wrapping (integer underflow)
	if (remaining_in_row_ == 0)
		return false;
	--remaining_in_row_;
	switch (phase_) {
	case Top:
		if (remaining_in_row_)
			map.get_rn(location_, &location_);
		else if (remaining_rows_in_upper_phase_) {
			phase_ = Upper;
			remaining_in_row_ = row_length_;
			assert(remaining_in_row_);
			location_ = TCoords<>(left_, location_.t);
		}
		break;
	case Upper:
		if (remaining_in_row_) {
			if (location_.t == TCoords<>::D)
				location_.t = TCoords<>::R;
			else
				location_ = TCoords<>(map.r_n(location_), TCoords<>::D);
		} else {
			if (--remaining_rows_in_upper_phase_) {
				row_length_ += 2;
				left_ = map.bl_n(left_);
			} else {
				if (remaining_rows_in_lower_phase_) {
					phase_ = Lower;
					assert(row_length_ >= 2);
					row_length_ -= 2;
				} else if (location_.t == TCoords<>::R) {
					phase_ = Bottom;
					row_length_ /= 2;
				} else return false;
				left_ = map.br_n(left_);
			}
			remaining_in_row_ = row_length_;
			location_ = TCoords<>(left_, location_.t);
		}
		break;
	case Lower:
		if (remaining_in_row_) {
			if (location_.t == TCoords<>::D)
				location_.t = TCoords<>::R;
			else
				location_ = TCoords<>(map.r_n(location_), TCoords<>::D);
		} else {
			if (--remaining_rows_in_lower_phase_) {
				assert(row_length_ >= 2);
				remaining_in_row_ = row_length_ -= 2;
				left_ = map.br_n(left_);
			}
			else if (location_.t == TCoords<>::R) {
				phase_ = Bottom;
				remaining_in_row_ = row_length_ / 2;
				left_ = map.br_n(left_);
			}
			location_ = TCoords<>(left_, location_.t);
		}
		break;
	case Bottom:
		if (remaining_in_row_)
			map.get_rn(location_, &location_);
		break;
	}
	assert(remaining_in_row_ < 10000); //  Catch wrapping (integer underflow)
	return true;
}


template <> MapTriangleRegion<TCoords<FCoords> >::MapTriangleRegion
	(const Map & map, Area<TCoords<FCoords> > area)
: radius_is_odd_(area.radius & 1)
{
	assert(area.t == TCoords<FCoords>::R || area.t == TCoords<FCoords>::D);
	const uint16_t radius_plus_1 = area.radius + 1;
	const uint16_t half_radius_rounded_down = area.radius / 2;
	row_length_ = radius_plus_1;
	for (uint32_t i = half_radius_rounded_down; i; --i) map.get_tln(area, &area);
	if (area.t == TCoords<FCoords>::R) {
		left_ = area;
		if (area.radius) {
			remaining_rows_in_upper_phase_ = half_radius_rounded_down + 1;
			remaining_rows_in_lower_phase_ = (area.radius - 1) / 2;
			if (radius_is_odd_) {
				map.get_trn(area, &area);
				phase_ = Top;
				row_length_ = area.radius + 2;
				remaining_in_row_ = radius_plus_1 / 2;
				area.t = TCoords<FCoords>::D;
			} else {
				phase_ = Upper;
				remaining_in_row_ = row_length_ = radius_plus_1;
				area.t = TCoords<FCoords>::R;
			}
		} else {
			phase_ = Bottom;
			remaining_in_row_ = 0;
			area.t = TCoords<FCoords>::R;
		}
	} else {
		remaining_rows_in_upper_phase_ = radius_plus_1 / 2;
		remaining_rows_in_lower_phase_ = half_radius_rounded_down;
		if (radius_is_odd_) {
			map.get_ln(area, &area);
			left_ = area;
			phase_ = Upper;
			remaining_in_row_ = row_length_ = area.radius + 2;
			area.t = TCoords<FCoords>::R;
		} else {
			map.get_bln(area, &left_);
			phase_ = Top;
			row_length_ = area.radius + 3;
			remaining_in_row_ = half_radius_rounded_down + (0 < area.radius);
			area.t = TCoords<FCoords>::D;
		}
	}
	location_ = area;
}


/// Traverse the region by row.
template <>
bool MapTriangleRegion<TCoords<FCoords> >::advance(const Map & map)
{
	assert(remaining_in_row_ < 10000); //  Catch wrapping (integer underflow)
	if (remaining_in_row_ == 0)
		return false;
	--remaining_in_row_;
	switch (phase_) {
	case Top:
		if (remaining_in_row_)
			map.get_rn(location_, &location_);
		else if (remaining_rows_in_upper_phase_) {
			phase_ = Upper;
			remaining_in_row_ = row_length_;
			assert(remaining_in_row_);
			location_ = TCoords<FCoords>(left_, location_.t);
		}
		break;
	case Upper:
		if (remaining_in_row_) {
			if (location_.t == TCoords<FCoords>::D)
				location_.t = TCoords<FCoords>::R;
			else
				location_ =
					TCoords<FCoords>(map.r_n(location_), TCoords<FCoords>::D);
		} else {
			if (--remaining_rows_in_upper_phase_) {
				row_length_ += 2;
				left_ = map.bl_n(left_);
			} else {
				if (remaining_rows_in_lower_phase_) {
					phase_ = Lower;
					assert(row_length_ >= 2);
					row_length_ -= 2;
				} else if (location_.t == TCoords<FCoords>::R) {
					phase_ = Bottom;
					row_length_ /= 2;
				} else
					return false;
				left_ = map.br_n(left_);
			}
			remaining_in_row_ = row_length_;
			location_ = TCoords<FCoords>(left_, location_.t);
		}
		break;
	case Lower:
		if (remaining_in_row_) {
			if (location_.t == TCoords<FCoords>::D)
				location_.t = TCoords<FCoords>::R;
			else
				location_ =
					TCoords<FCoords>(map.r_n(location_), TCoords<FCoords>::D);
		} else {
			if (--remaining_rows_in_lower_phase_) {
				assert(row_length_ >= 2);
				remaining_in_row_ = row_length_ -= 2;
				left_ = map.br_n(left_);
			}
			else if (location_.t == TCoords<FCoords>::R) {
				phase_ = Bottom;
				remaining_in_row_ = row_length_ / 2;
				left_ = map.br_n(left_);
			}
			location_ = TCoords<FCoords>(left_, location_.t);
		}
		break;
	case Bottom:
		if (remaining_in_row_)
			map.get_rn(location_, &location_);
		break;
	}
	assert(remaining_in_row_ < 10000); //  Catch wrapping (integer underflow)
	return true;
}

}
