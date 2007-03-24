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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "maptriangleregion.h"

template <> MapTriangleRegion<>::MapTriangleRegion
(const Map & map, TCoords<> first, const Uint16 radius)
: m_radius_is_odd(radius & 1)
{
	assert(first.t == TCoords<>::R or first.t == TCoords<>::D);
	const unsigned short radius_plus_1 = radius + 1;
	const unsigned short half_radius_rounded_down = radius / 2;
	m_row_length = radius_plus_1;
	for (unsigned short i = 0; i < half_radius_rounded_down; ++i)
		map.get_tln(first, &first);
	if (first.t == TCoords<>::R) {
		m_left = first;
		if (radius) {
			m_remaining_rows_in_upper_phase = half_radius_rounded_down + 1;
			m_remaining_rows_in_lower_phase = (radius - 1) / 2;
			if (m_radius_is_odd) {
				map.get_trn(first, &first);
				m_phase = Top;
				m_row_length = radius + 2;
				m_remaining_in_row = radius_plus_1 / 2;
				first.t = TCoords<>::D;
			} else {
				m_phase = Upper;
				m_remaining_in_row = m_row_length = radius_plus_1;
				first.t = TCoords<>::R;
			}
		} else {
			assert(radius == 0);
			m_phase = Bottom;
			m_remaining_in_row = 1;
			first.t = TCoords<>::R;
		}
	} else {
		m_remaining_rows_in_upper_phase = radius_plus_1 / 2;
		m_remaining_rows_in_lower_phase = half_radius_rounded_down;
		if (m_radius_is_odd) {
			map.get_ln(first, &first);
			m_left = first;
			m_phase = Upper;
			m_remaining_in_row = m_row_length = radius + 2;
			first.t = TCoords<>::R;
		} else {
			map.get_bln(first, &m_left);
			m_phase = Top;
			m_row_length = radius + 3;
			m_remaining_in_row = half_radius_rounded_down + 1;
			first.t = TCoords<>::D;
		}
	}
	m_location = first;
}


/// Traverse the region by row.
template <> bool MapTriangleRegion<>::advance(const Map & map) throw () {
	assert(m_remaining_in_row < 10000); //  Catch wrapping (integer underflow)
	if (m_remaining_in_row == 0) return false;
	--m_remaining_in_row;
	switch (m_phase) {
	case Top:
		if (m_remaining_in_row) map.get_rn(m_location, &m_location);
		else if (m_remaining_rows_in_upper_phase) {
			m_phase = Upper;
			m_remaining_in_row = m_row_length;
			assert(m_remaining_in_row);
			m_location = TCoords<>(m_left, m_location.t);
		}
		break;
	case Upper:
		if (m_remaining_in_row) {
			if (m_location.t == TCoords<>::D) m_location.t = TCoords<>::R;
			else m_location = TCoords<>(map.r_n(m_location), TCoords<>::D);
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
			if (m_location.t == TCoords<>::D) m_location.t = TCoords<>::R;
			else m_location = TCoords<>(map.r_n(m_location), TCoords<>::D);
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
		if (m_remaining_in_row) map.get_rn(m_location, &m_location);
		break;
	default:
		assert(0);
	}
	assert(m_remaining_in_row < 10000); //  Catch wrapping (integer underflow)
	return true;
}
