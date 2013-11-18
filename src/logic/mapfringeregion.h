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

#ifndef MAPFRINGEREGION_H
#define MAPFRINGEREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that iterates over every node on the fringe of an
 * area.
 *
 * Note that the order in which nodes are returned is not guarantueed (although
 * the current implementation begins at the top left node and then moves around
 * clockwise when advance is called repeatedly).
 */
template <typename Area_type = Area<> > struct MapFringeRegion {
	MapFringeRegion(const Map & map, Area_type area) :
		m_area              (area),
		m_remaining_in_phase(area.radius),
		m_phase             (area.radius ? 6 : 0)
	{
		for (typename Area_type::Radius_type r = area.radius; r; --r)
			map.get_tln(m_area, &m_area);
	}


	const typename Area_type::Coords_type & location() const {return m_area;}

	/**
	 * Moves on to the next location. The return value indicates whether the new
	 * location has not yet been reached during this iteration. Note that when
	 * the area is so large that it overlaps itself because of wrapping, the same
	 * location may be reached several times during an iteration, while advance
	 * keeps returning true. When finally advance returns false, it means that
	 * the iteration is done and location is the same as it was before the first
	 * call to advance. The iteration can then be redone by calling advance
	 * again, which will return true until it reaches the first location the next
	 * time around, and so on.
	 */
	bool advance(const Map &);

	/**
	 * When advance has returned false, iterating over the same fringe again is
	 * not the only possibility. It is also possible to call extend. This makes
	 * the region ready to iterate over the next layer of nodes.
	 */
	void extend(const Map & map) {
		map.get_tln(m_area, &m_area);
		++m_area.radius;
		m_remaining_in_phase = m_area.radius;
		m_phase = 6;
	}

	typename Area_type::Radius_type radius() const {return m_area.radius;}
private:
	Area_type                       m_area;
	typename Area_type::Radius_type m_remaining_in_phase;
	uint8_t   m_phase;
};

}

#endif
