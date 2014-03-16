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

#ifndef MAPREGION_H
#define MAPREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that iterates over every node of an area.
 *
 * Note that the order in which fields are returned is not guaranteed.
 */
template <typename Area_type = Area<> > struct MapRegion {
	MapRegion(const Map & map, Area_type area) :
		m_area            (area),
		m_rowwidth        (area.radius + 1),
		m_remaining_in_row(m_rowwidth),
		m_remaining_rows  (m_rowwidth + area.radius)
	{
		for (typename Area_type::Radius_type r = area.radius; r; --r)
			map.get_tln(m_area, &m_area);
		m_left = m_area;
	}

	const typename Area_type::Coords_type & location() const {return m_area;}

	/// Moves on to the next location. The return value indicates whether the
	/// new location has not yet been reached during this iteration. Note that
	/// when the area is so large that it overlaps itself because of wrapping,
	/// the same location may be reached several times during an iteration,
	/// while advance keeps returning true. When finally advance returns false,
	/// it means that the iteration is done.
	bool advance(const Map & map) {
		if (--m_remaining_in_row)
			map.get_rn(m_area, &m_area);
		else if (m_area.radius < --m_remaining_rows) {
			map.get_bln(m_left, &m_area); m_left = m_area;
			m_remaining_in_row = ++m_rowwidth;
		} else if (m_remaining_rows) {
			map.get_brn(m_left, &m_area); m_left = m_area;
			m_remaining_in_row = --m_rowwidth;
		} else return false;
		return true;
	}

	typename Area_type::Radius_type radius() const {return m_area.radius;}
private:
	Area_type                       m_area;
	typename Area_type::Coords_type m_left;
	typename Area_type::Radius_type m_rowwidth;
	typename Area_type::Radius_type m_remaining_in_row;
	typename Area_type::Radius_type m_remaining_rows;
};

}

#endif
