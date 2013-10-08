/*
 * Copyright (C) 2004, 2007-2008 by the Widelands Development Team
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

#include "logic/maphollowregion.h"

namespace Widelands {

template <> MapHollowRegion<Area<> >::MapHollowRegion
	(const Map & map, HollowArea<Area<> > const hollow_area)
:
m_hollow_area (hollow_area),
m_phase       (Top),
m_delta_radius(hollow_area.radius - hollow_area.hole_radius),
m_row         (0),
m_rowwidth    (hollow_area.radius + 1),
m_rowpos      (0),
m_left        (hollow_area)
{
	assert(hollow_area.hole_radius < hollow_area.radius);
	for (uint16_t r = hollow_area.radius; r; --r)
		map.get_tln(m_hollow_area, &m_hollow_area);
	m_left = m_hollow_area;
}

template <> bool MapHollowRegion<Area<> >::advance(const Map & map) {
	if (m_phase == None)
		return false;
	++m_rowpos;
	if (m_rowpos < m_rowwidth) {
		map.get_rn(m_hollow_area, &m_hollow_area);
		if ((m_phase & (Upper|Lower)) and m_rowpos == m_delta_radius) {
			//  Jump over the hole.
			const uint32_t holewidth = m_rowwidth - 2 * m_delta_radius;
			for (uint32_t i = 0; i < holewidth; ++i)
				map.get_rn(m_hollow_area, &m_hollow_area);
			m_rowpos += holewidth;
		}
	} else {
		++m_row;
		if (m_phase == Top and m_row == m_delta_radius)
			m_phase = Upper;

		// If we completed the widest, center line, switch into lower mode
		// There are m_radius+1 lines in the upper "half", because the upper
		// half includes the center line.
		else if (m_phase == Upper and m_row > m_hollow_area.radius) {
			m_row = 1;
			m_phase = Lower;
		}

		if (m_phase & (Top|Upper)) {
			map.get_bln(m_left, &m_hollow_area);
			++m_rowwidth;
		} else {

			if (m_row > m_hollow_area.radius) {
				m_phase = None;
				return true; // early out
			} else if (m_phase == Lower and m_row > m_hollow_area.hole_radius)
				m_phase = Bottom;

			map.get_brn(m_left, &m_hollow_area);
			--m_rowwidth;
		}

		m_left = m_hollow_area;
		m_rowpos = 0;
	}

	return true;
}

}
