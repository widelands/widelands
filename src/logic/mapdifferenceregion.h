/*
 * Copyright (C) 2007-2008, 2010 by the Widelands Development Team
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

#ifndef MAPDIFFERENCEREGION_H
#define MAPDIFFERENCEREGION_H

#include "logic/map.h"

namespace Widelands {

/// Producer/Coroutine struct that iterates over the set of nodes that is the
/// difference between the areas A and B around neighboring nodes a and b. The
/// constructor takes a Direction parameter, which is the walking direction
/// when going from a to b.
///
/// It first iterates over the set of nodes that are in A but not in B. When
/// that iteration is completed (advance has returned false),
/// move_to_other_side can be called to prepare the region for iterating over
/// the set of nodes in B but not in A. (Because of symmetry, it is after that
/// iteration again possible to call move_to_other_side to iterate over the
/// nodes that are in A but not in B again, and so on.)
///
/// \note The order in which nodes are returned is not guarantueed.
template <typename Area_type = Area<> > struct MapDifferenceRegion {
	MapDifferenceRegion
		(const Map & map, Area_type area, Direction direction)
		: m_area(area), m_remaining_in_edge(area.radius), m_passed_corner(false)
	{
		assert(1 <= direction);
		assert     (direction <= 6);
		--direction; if (not direction) direction = 6;
		--direction; if (not direction) direction = 6;
		switch (direction) {
#define DIRECTION_CASE(dir, neighbour_function)                               \
      case dir:                                                               \
         for (; area.radius; --area.radius)                                   \
            map.neighbour_function(m_area, &m_area);                          \
         break;                                                               \

		DIRECTION_CASE(WALK_NW, get_tln);
		DIRECTION_CASE(WALK_NE, get_trn);
		DIRECTION_CASE(WALK_E,  get_rn);
		DIRECTION_CASE(WALK_SE, get_brn);
		DIRECTION_CASE(WALK_SW, get_bln);
		DIRECTION_CASE(WALK_W,  get_ln);
#undef DIRECTION_CASE
		}
		--direction; if (not direction) direction = 6;
		--direction; if (not direction) direction = 6;
		m_direction = direction;
	}

	typename Area_type::Coords_type & location() const {return m_area;}

	/**
	 * Moves on to the next location. The return value indicates whether the new
	 * location has not yet been reached during this iteration. Note that when
	 * the area is so large that it overlaps itself because of wrapping, the same
	 * location may be reached several times during an iteration, while advance
	 * keeps returning true. When finally advance returns false, it means that
	 * the iteration is done.
	 */
	bool advance(const Map & map);

	void move_to_other_side(const Map & map);

	typename Area_type::Radius_type radius() const {return m_area.radius;}
private:
	Area_type                       m_area;
	typename Area_type::Radius_type m_remaining_in_edge;
	bool                            m_passed_corner;
	Direction                       m_direction;
};

}

#endif
