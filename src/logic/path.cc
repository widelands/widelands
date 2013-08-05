/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#include "logic/path.h"

#include <algorithm>

#include "logic/game_data_error.h"
#include "logic/instances.h"
#include "logic/map.h"

namespace Widelands {

Path::Path(CoordPath & o)
	: m_start(o.get_start()), m_end(o.get_end()), m_path(o.steps())
{
	std::reverse(m_path.begin(), m_path.end()); //  path stored in reverse order
}

/*
===============
Change the path so that it goes in the opposite direction
===============
*/
void Path::reverse()
{
	std::swap(m_start, m_end);
	std::reverse(m_path.begin(), m_path.end());

	for (uint32_t i = 0; i < m_path.size(); ++i)
		m_path[i] = get_reverse_dir(m_path[i]);
}

/*
===============
Add the given step at the end of the path.
===============
*/
void Path::append(const Map & map, const Direction dir) {
	m_path.insert(m_path.begin(), dir); // stores in reversed order!
	map.get_neighbour(m_end, dir, &m_end);
}

/**
 * Save the given path in the given file
 */
void Path::save(FileWrite & fw) const
{
	fw.Unsigned8(1); // version number
	fw.Coords32(m_start);

	// Careful: steps are stored in the reverse order in m_path
	// However, we save them in the forward order, to make loading easier
	fw.Unsigned32(m_path.size());
	for (uint32_t i = m_path.size(); i > 0; --i)
		fw.Direction8(m_path[i - 1]);
}

/**
 * Load the path from the given file.
 *
 * The path previously contained in \p this object is entirely
 * replaced by the path from the file.
 */
void Path::load(FileRead & fr, const Map & map)
{
	uint8_t version = fr.Unsigned8();
	if (version != 1)
		throw game_data_error("path: unknown version %u", version);

	m_start = m_end = fr.Coords32(map.extent());
	m_path.clear();
	uint32_t steps = fr.Unsigned32();
	while (steps--)
		append(map, fr.Direction8());
}

/*
===============
Initialize from a path, calculating the coordinates as needed
===============
*/
CoordPath::CoordPath(const Map & map, const Path & path) {
	m_coords.clear();
	m_path.clear();

	m_coords.push_back(path.get_start());

	Coords c = path.get_start();

	Path::Step_Vector::size_type nr_steps = path.get_nsteps();
	for (Path::Step_Vector::size_type i = 0; i < nr_steps; ++i) {
		const char dir = path[i];

		m_path.push_back(dir);
		map.get_neighbour(c, dir, &c);
		m_coords.push_back(c);
	}
}


/// After which step does the node appear in this path?
/// \return -1 if node is not part of this path.
int32_t CoordPath::get_index(Coords const c) const
{
	for (uint32_t i = 0; i < m_coords.size(); ++i)
		if (m_coords[i] == c)
			return i;

	return -1;
}


/*
===============
Reverse the direction of the path.
===============
*/
void CoordPath::reverse()
{
	std::reverse(m_path.begin(), m_path.end());
	std::reverse(m_coords.begin(), m_coords.end());

	for (uint32_t i = 0; i < m_path.size(); ++i)
		m_path[i] = get_reverse_dir(m_path[i]);
}


/*
===============
Truncate the path after the given number of steps
===============
*/
void CoordPath::truncate(const std::vector<char>::size_type after) {
	assert(after <= m_path.size());

	m_path.erase(m_path.begin() + after, m_path.end());
	m_coords.erase(m_coords.begin() + after + 1, m_coords.end());
}

/*
===============
Opposite of truncate: remove the first n steps of the path.
===============
*/
void CoordPath::starttrim(const std::vector<char>::size_type before) {
	assert(before <= m_path.size());

	m_path.erase(m_path.begin(), m_path.begin() + before);
	m_coords.erase(m_coords.begin(), m_coords.begin() + before);
}

/*
===============
Append the given path. Automatically created the necessary coordinates.
===============
*/
void CoordPath::append(const Map & map, const Path & tail) {
	assert(tail.get_start() == get_end());

	Coords c = get_end();

	const Path::Step_Vector::size_type nr_steps = tail.get_nsteps();
	for (CoordPath::Step_Vector::size_type i = 0; i < nr_steps; ++i) {
		const char dir = tail[i];

		map.get_neighbour(c, dir, &c);
		m_path.push_back(dir);
		m_coords.push_back(c);
	}
}

/*
===============
Append the given path.
===============
*/
void CoordPath::append(const CoordPath & tail)
{
	assert(tail.get_start() == get_end());

	m_path.insert(m_path.end(), tail.m_path.begin(), tail.m_path.end());
	m_coords.insert
		(m_coords.end(), tail.m_coords.begin() + 1, tail.m_coords.end());
}

}
