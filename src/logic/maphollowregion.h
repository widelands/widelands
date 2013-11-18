/*
 * Copyright (C) 2004, 2007-2009 by the Widelands Development Team
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

#ifndef MAPHOLLOWREGION_H
#define MAPHOLLOWREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that iterates over every node for which the
 * distance to the center point is greater than hollow_area.hole_radius and
 * at most hollow_area.radius.
 *
 * \note The order in which fields are returned is not guarantueed.
 */
template <typename Area_type = Area<> > struct MapHollowRegion {
	MapHollowRegion(const Map & map, const HollowArea<Area_type> hollow_area);

	const typename Area_type::Coords_type& location() const {
		return m_hollow_area;
	}

	/**
	 * Moves on to the next location, traversing the region by row.
	 *
	 * I hope traversing by row results in slightly better cache behaviour
	 * than other algorithms (e.g. one could also walk concentric "circles"
	 * / hexagons).
	 *
	 * \return Whether the new location has not yet been reached during this
	 * iteration.
	 *
	 * \note When the area is so large that it overlaps itself because of
	 * wrapping, the same location may be reached several times during an
	 * iteration, while advance keeps returning true. When finally advance
	 * returns false, it means that the iteration is done.
	 */
	bool advance(const Map &);

private:
	enum Phase {
		None   = 0, // not initialized or completed
		Top    = 1, // above the hole
		Upper  = 2, // upper half
		Lower  = 4, // lower half
		Bottom = 8, // below the hole
	};

	HollowArea<Area_type> m_hollow_area;
	Phase m_phase;
	const uint32_t m_delta_radius;
	uint32_t m_row; // # of rows completed in this phase
	uint32_t m_rowwidth; // # of fields to return per row
	uint32_t m_rowpos; // # of fields we have returned in this row
	typename Area_type::Coords_type m_left; //  left-most node of current row
};

}

#endif
