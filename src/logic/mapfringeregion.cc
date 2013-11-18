/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "logic/mapfringeregion.h"

namespace Widelands {

template <>
bool MapFringeRegion<Area<FCoords> >::advance(const Map & map) {
	switch (m_phase) {
	case 0:
		if (m_area.radius) {
			m_remaining_in_phase = m_area.radius;
			m_phase              = 6;
		}
		else
			return false;
		/* no break */
	case 1: map.get_trn(m_area, &m_area); break;
	case 2: map.get_tln(m_area, &m_area); break;
	case 3: map. get_ln(m_area, &m_area); break;
	case 4: map.get_bln(m_area, &m_area); break;
	case 5: map.get_brn(m_area, &m_area); break;
	case 6: map. get_rn(m_area, &m_area); break;
	default:
		assert(false);
		break;
	}
	if (--m_remaining_in_phase == 0) {
		m_remaining_in_phase = m_area.radius;
		--m_phase;
	}
	return m_phase;
}

template <>
bool MapFringeRegion<Area<> >::advance(const Map & map) {
	switch (m_phase) {
	case 0:
		if (m_area.radius) {
			m_remaining_in_phase = m_area.radius;
			m_phase              = 6;
		}
		else
			return false;
		/* no break */
	case 1: map.get_trn(m_area, &m_area); break;
	case 2: map.get_tln(m_area, &m_area); break;
	case 3: map. get_ln(m_area, &m_area); break;
	case 4: map.get_bln(m_area, &m_area); break;
	case 5: map.get_brn(m_area, &m_area); break;
	case 6: map. get_rn(m_area, &m_area); break;
	default:
		assert(false);
		break;
	}
	if (--m_remaining_in_phase == 0) {
		m_remaining_in_phase = m_area.radius;
		--m_phase;
	}
	return m_phase;
}

}
