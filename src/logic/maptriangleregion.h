/*
 * Copyright (C) 2006-2008 by the Widelands Development Team
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

#ifndef MAPTRIANGLEREGION_H
#define MAPTRIANGLEREGION_H

#include "logic/map.h"

namespace Widelands {

/**
 * Producer/Coroutine struct that returns every triangle which can be reached by
 * crossing at most \a radius edges.
 *
 * Each such location is returned exactly once via next(). But this does not
 * guarantee that a location is returned at most once when the radius is so
 * large that the area overlaps itself because of wrapping.
 *
 * Note that the order in which locations are returned is not guarantueed. (But
 * in fact the triangles are returned row by row from top to bottom and from
 * left to right in each row and I see no reason why that would ever change.)
 *
 * The initial coordinates must refer to a triangle
 * (TCoords<>::D or TCoords<>::R). Use MapRegion instead for nodes
 * (TCoords<>::None).
 */
template <typename Coords_type = TCoords<>, typename Radius_type = uint16_t>
struct MapTriangleRegion
{
	MapTriangleRegion(const Map &, Coords_type, uint16_t radius);

	const Coords_type & location() const;

	/**
	 * Moves on to the next location, traversing the region by row.
	 *
	 * \return Whether the new location has not yet been reached during this
	 * iteration.
	 *
	 * \note When the area is so large that it overlaps itself because of
	 * wrapping, the same location may be reached several times during an
	 * iteration, while advance keeps returning true. When finally advance
	 * returns false, it means that the iteration is done and location is
	 * the same as it was before the first call to advance. The iteration
	 * can then be redone by calling advance again, which will return true
	 * until it reaches the first location the next time around, and so on.
	 */
	bool advance(const Map &);
};
template <> struct MapTriangleRegion<FCoords> {
	MapTriangleRegion(const Map & map, const Area<FCoords> area) :
		m_area(TCoords<FCoords>(area, TCoords<FCoords>::D), area.radius + 1),
		m_rowwidth        (m_area.radius * 2 + 1),
		m_remaining_in_row(m_rowwidth),
		m_remaining_rows  (m_area.radius * 2)
	{
		for (uint8_t r = m_area.radius; r; --r) map.get_tln(m_area, &m_area);
		m_left = m_area;
	}

	const TCoords<FCoords> & location() const {return m_area;}

	bool advance(const Map & map) {
		if (--m_remaining_in_row) {
			if (m_area.t == TCoords<FCoords>::D)
				m_area.t = TCoords<FCoords>::R;
			else {
				m_area.t = TCoords<FCoords>::D;
				map.get_rn(m_area, &m_area);
			}
		} else if (m_area.radius < --m_remaining_rows) {
			map.get_bln(m_left, &m_area); m_left = m_area;
			m_area.t = TCoords<FCoords>::D;
			m_remaining_in_row = m_rowwidth += 2;
		} else if (m_remaining_rows) {
			map.get_brn(m_left, &m_area); m_left = m_area;
			m_area.t = TCoords<FCoords>::D;
			m_remaining_in_row = m_rowwidth -= 2;
		} else return false;
		return true;
	}

private:
	Area<TCoords<FCoords> > m_area;
	FCoords                 m_left;
	uint16_t                m_rowwidth;
	uint16_t                m_remaining_in_row;
	uint16_t                m_remaining_rows;
};
template <typename Coords_type> struct MapTriangleRegion<TCoords<Coords_type> >
{
	MapTriangleRegion(const Map &, Area<TCoords<Coords_type>, uint16_t>);

	const TCoords<Coords_type> & location() const {return m_location;}

	bool advance(const Map &);

private:
	const bool m_radius_is_odd;
	enum {Top, Upper, Lower, Bottom} m_phase;
	uint16_t m_remaining_rows_in_upper_phase;
	uint16_t m_remaining_rows_in_lower_phase;
	uint16_t m_row_length, m_remaining_in_row;
	Coords_type          m_left;
	TCoords<Coords_type> m_location;
};

}

#endif
