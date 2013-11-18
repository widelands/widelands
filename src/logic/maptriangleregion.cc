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
: m_radius_is_odd(area.radius & 1)
{
	assert(area.t == TCoords<>::R or area.t == TCoords<>::D);
	const uint16_t radius_plus_1 = area.radius + 1;
	const uint16_t half_radius_rounded_down = area.radius / 2;
	m_row_length = radius_plus_1;
	for (uint32_t i = half_radius_rounded_down; i; --i) map.get_tln(area, &area);
	if (area.t == TCoords<>::R) {
		m_left = area;
		if (area.radius) {
			m_remaining_rows_in_upper_phase = half_radius_rounded_down + 1;
			m_remaining_rows_in_lower_phase = (area.radius - 1) / 2;
			if (m_radius_is_odd) {
				map.get_trn(area, &area);
				m_phase = Top;
				m_row_length = area.radius + 2;
				m_remaining_in_row = radius_plus_1 / 2;
				area.t = TCoords<>::D;
			} else {
				m_phase = Upper;
				m_remaining_in_row = m_row_length = radius_plus_1;
				area.t = TCoords<>::R;
			}
		} else {
			assert(area.radius == 0);
			m_phase = Bottom;
			m_remaining_in_row = 0;
			area.t = TCoords<>::R;
		}
	} else {
		m_remaining_rows_in_upper_phase = radius_plus_1 / 2;
		m_remaining_rows_in_lower_phase = half_radius_rounded_down;
		if (m_radius_is_odd) {
			map.get_ln(area, &area);
			m_left = area;
			m_phase = Upper;
			m_remaining_in_row = m_row_length = area.radius + 2;
			area.t = TCoords<>::R;
		} else {
			map.get_bln(area, &m_left);
			m_phase = Top;
			m_row_length = area.radius + 3;
			m_remaining_in_row = half_radius_rounded_down + (0 < area.radius);
			area.t = TCoords<>::D;
		}
	}
	m_location = area;
}


template <> bool MapTriangleRegion<>::advance(const Map & map) {
	assert(m_remaining_in_row < 10000); //  Catch wrapping (integer underflow)
	if (m_remaining_in_row == 0)
		return false;
	--m_remaining_in_row;
	switch (m_phase) {
	case Top:
		if (m_remaining_in_row)
			map.get_rn(m_location, &m_location);
		else if (m_remaining_rows_in_upper_phase) {
			m_phase = Upper;
			m_remaining_in_row = m_row_length;
			assert(m_remaining_in_row);
			m_location = TCoords<>(m_left, m_location.t);
		}
		break;
	case Upper:
		if (m_remaining_in_row) {
			if (m_location.t == TCoords<>::D)
				m_location.t = TCoords<>::R;
			else
				m_location = TCoords<>(map.r_n(m_location), TCoords<>::D);
		} else {
			if (--m_remaining_rows_in_upper_phase) {
				m_row_length += 2;
				m_left = map.bl_n(m_left);
			} else {
				if (m_remaining_rows_in_lower_phase) {
					m_phase = Lower;
					assert(m_row_length >= 2);
					m_row_length -= 2;
				} else if (m_location.t == TCoords<>::R) {
					m_phase = Bottom;
					m_row_length /= 2;
				} else return false;
				m_left = map.br_n(m_left);
			}
			m_remaining_in_row = m_row_length;
			m_location = TCoords<>(m_left, m_location.t);
		}
		break;
	case Lower:
		if (m_remaining_in_row) {
			if (m_location.t == TCoords<>::D)
				m_location.t = TCoords<>::R;
			else
				m_location = TCoords<>(map.r_n(m_location), TCoords<>::D);
		} else {
			if (--m_remaining_rows_in_lower_phase) {
				assert(m_row_length >= 2);
				m_remaining_in_row = m_row_length -= 2;
				m_left = map.br_n(m_left);
			}
			else if (m_location.t == TCoords<>::R) {
				m_phase = Bottom;
				m_remaining_in_row = m_row_length / 2;
				m_left = map.br_n(m_left);
			}
			m_location = TCoords<>(m_left, m_location.t);
		}
		break;
	case Bottom:
		if (m_remaining_in_row)
			map.get_rn(m_location, &m_location);
		break;
	default:
		assert(false);
		break;
	}
	assert(m_remaining_in_row < 10000); //  Catch wrapping (integer underflow)
	return true;
}


template <> MapTriangleRegion<TCoords<FCoords> >::MapTriangleRegion
	(const Map & map, Area<TCoords<FCoords> > area)
: m_radius_is_odd(area.radius & 1)
{
	assert(area.t == TCoords<FCoords>::R or area.t == TCoords<FCoords>::D);
	const uint16_t radius_plus_1 = area.radius + 1;
	const uint16_t half_radius_rounded_down = area.radius / 2;
	m_row_length = radius_plus_1;
	for (uint32_t i = half_radius_rounded_down; i; --i) map.get_tln(area, &area);
	if (area.t == TCoords<FCoords>::R) {
		m_left = area;
		if (area.radius) {
			m_remaining_rows_in_upper_phase = half_radius_rounded_down + 1;
			m_remaining_rows_in_lower_phase = (area.radius - 1) / 2;
			if (m_radius_is_odd) {
				map.get_trn(area, &area);
				m_phase = Top;
				m_row_length = area.radius + 2;
				m_remaining_in_row = radius_plus_1 / 2;
				area.t = TCoords<FCoords>::D;
			} else {
				m_phase = Upper;
				m_remaining_in_row = m_row_length = radius_plus_1;
				area.t = TCoords<FCoords>::R;
			}
		} else {
			m_phase = Bottom;
			m_remaining_in_row = 0;
			area.t = TCoords<FCoords>::R;
		}
	} else {
		m_remaining_rows_in_upper_phase = radius_plus_1 / 2;
		m_remaining_rows_in_lower_phase = half_radius_rounded_down;
		if (m_radius_is_odd) {
			map.get_ln(area, &area);
			m_left = area;
			m_phase = Upper;
			m_remaining_in_row = m_row_length = area.radius + 2;
			area.t = TCoords<FCoords>::R;
		} else {
			map.get_bln(area, &m_left);
			m_phase = Top;
			m_row_length = area.radius + 3;
			m_remaining_in_row = half_radius_rounded_down + (0 < area.radius);
			area.t = TCoords<FCoords>::D;
		}
	}
	m_location = area;
}


/// Traverse the region by row.
template <>
bool MapTriangleRegion<TCoords<FCoords> >::advance(const Map & map)
{
	assert(m_remaining_in_row < 10000); //  Catch wrapping (integer underflow)
	if (m_remaining_in_row == 0)
		return false;
	--m_remaining_in_row;
	switch (m_phase) {
	case Top:
		if (m_remaining_in_row)
			map.get_rn(m_location, &m_location);
		else if (m_remaining_rows_in_upper_phase) {
			m_phase = Upper;
			m_remaining_in_row = m_row_length;
			assert(m_remaining_in_row);
			m_location = TCoords<FCoords>(m_left, m_location.t);
		}
		break;
	case Upper:
		if (m_remaining_in_row) {
			if (m_location.t == TCoords<FCoords>::D)
				m_location.t = TCoords<FCoords>::R;
			else
				m_location =
					TCoords<FCoords>(map.r_n(m_location), TCoords<FCoords>::D);
		} else {
			if (--m_remaining_rows_in_upper_phase) {
				m_row_length += 2;
				m_left = map.bl_n(m_left);
			} else {
				if (m_remaining_rows_in_lower_phase) {
					m_phase = Lower;
					assert(m_row_length >= 2);
					m_row_length -= 2;
				} else if (m_location.t == TCoords<FCoords>::R) {
					m_phase = Bottom;
					m_row_length /= 2;
				} else
					return false;
				m_left = map.br_n(m_left);
			}
			m_remaining_in_row = m_row_length;
			m_location = TCoords<FCoords>(m_left, m_location.t);
		}
		break;
	case Lower:
		if (m_remaining_in_row) {
			if (m_location.t == TCoords<FCoords>::D)
				m_location.t = TCoords<FCoords>::R;
			else
				m_location =
					TCoords<FCoords>(map.r_n(m_location), TCoords<FCoords>::D);
		} else {
			if (--m_remaining_rows_in_lower_phase) {
				assert(m_row_length >= 2);
				m_remaining_in_row = m_row_length -= 2;
				m_left = map.br_n(m_left);
			}
			else if (m_location.t == TCoords<FCoords>::R) {
				m_phase = Bottom;
				m_remaining_in_row = m_row_length / 2;
				m_left = map.br_n(m_left);
			}
			m_location = TCoords<FCoords>(m_left, m_location.t);
		}
		break;
	case Bottom:
		if (m_remaining_in_row)
			map.get_rn(m_location, &m_location);
		break;
	default:
		assert(false);
		break;
	}
	assert(m_remaining_in_row < 10000); //  Catch wrapping (integer underflow)
	return true;
}

}
