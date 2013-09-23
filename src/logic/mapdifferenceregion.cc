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

namespace Widelands {

template <> bool MapDifferenceRegion<Area<FCoords> >::advance(const Map & map)
{
	assert(1 <= m_direction);
	assert     (m_direction <= 6);
	if (m_remaining_in_edge) {
		map.get_neighbour(m_area, m_direction, &m_area);
		--m_remaining_in_edge;
		return true;
	} else {
		if (not m_passed_corner) {
			m_passed_corner = true;
			--m_direction; if (not m_direction) m_direction = 6;
			m_remaining_in_edge = m_area.radius;
			return advance(map);
		}
	}
	return false;
}

template <>
void MapDifferenceRegion<Area<FCoords> >::move_to_other_side(const Map & map)
{
	assert(1 <= m_direction);
	assert     (m_direction <= 6);
	assert(m_passed_corner);
	--m_direction; if (not m_direction) m_direction = 6;
	Area<FCoords>::Radius_type steps_left = m_area.radius + 1;
	switch (m_direction) {
#define DIRECTION_CASE(dir, neighbour_function)                               \
   case dir:                                                                  \
      for (; steps_left; --steps_left)                                        \
         map.neighbour_function(m_area, &m_area);                             \
      break;                                                                  \

	DIRECTION_CASE(WALK_NW, get_tln);
	DIRECTION_CASE(WALK_NE, get_trn);
	DIRECTION_CASE(WALK_E,  get_rn);
	DIRECTION_CASE(WALK_SE, get_brn);
	DIRECTION_CASE(WALK_SW, get_bln);
	DIRECTION_CASE(WALK_W,  get_ln);
	default: assert(false);
	}
	--m_direction; if (not m_direction) m_direction = 6;
	m_remaining_in_edge = m_area.radius;
	m_passed_corner     = false;
}

}
