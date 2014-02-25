/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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

#include "economy/route.h"

#include "economy/flag.h"
#include "logic/editor_game_base.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"

/*
==============================================================================

Route IMPLEMENTATION

==============================================================================
*/

namespace Widelands {
Route::Route() : m_totalcost(0)
{}

/**
 * Completely clear the route and initialize it
 * to its totalcost
*/
void Route::init(int32_t totalcost)
{
	m_totalcost = totalcost;
	m_route.clear();
}

/**
 * Return the flag with the given number.
 * idx == 0 is the start flag, idx == get_nrsteps() is the end flag.
 * Every route has at least one flag.
*/
Flag & Route::get_flag
	(Editor_Game_Base & egbase, std::vector<Flag *>::size_type const idx)
{
	assert(idx < m_route.size());
	return *m_route[idx].get(egbase);
}

/**
 * Remove the first count steps from the route.
*/
void Route::starttrim(int32_t count)
{
	assert(count < static_cast<int32_t>(m_route.size()));

	m_route.erase(m_route.begin(), m_route.begin() + count);
}

/**
 * Keep the first count steps, truncate the rest.
*/
void Route::truncate(int32_t const count)
{
	assert(count < static_cast<int32_t>(m_route.size()));

	m_route.erase(m_route.begin() + count + 1, m_route.end());
}


/**
 * Preliminarily load the route from the given file.
 * Must call \ref load_pointers after \ref load
 * \param data the caller must provide and manage this buffer that
 * stores information for a later call to \ref load_pointers
 */
void Route::load(LoadData & data, FileRead & fr)
{
	m_route.clear();

	m_totalcost = fr.Signed32();
	uint32_t nsteps = fr.Unsigned16();
	for (uint32_t step = 0; step < nsteps; ++step)
		data.flags.push_back(fr.Unsigned32());
}


/**
 * load_pointers phase of loading: This is responsible for filling
 * in the \ref Flag pointers. Must be called after \ref load.
 */
void Route::load_pointers(const LoadData & data, Map_Map_Object_Loader & mol) {
	for (uint32_t i = 0; i < data.flags.size(); ++i) {
		uint32_t const flag_serial = data.flags.size();
		try {
			m_route.push_back(&mol.get<Flag>(flag_serial));
		} catch (const _wexception & e) {
			throw wexception("Route flag #%u (%u): %s", i, flag_serial, e.what());
		}
	}
}


/**
 * Save the route to the given file.
 */
void Route::save
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Signed32(get_totalcost());
	fw.Unsigned16(m_route.size());
	for
		(std::vector<Object_Ptr>::size_type idx = 0;
		 idx < m_route.size();
		 ++idx)
	{
		Flag & flag = get_flag(egbase, idx);
		assert(mos.is_object_known(flag));
		fw.Unsigned32(mos.get_object_file_index(flag));
	}
}

/**
 * Insert this node as the very first entry into the Route
 */
void Route::insert_as_first(RoutingNode * node) {
	// we are sure that node is a Flag, since it is the only
	// RoutingNode ever used in the path finder (outside tests)
	// That's why we can make this cast
	m_route.insert(m_route.begin(), static_cast<Flag *>(node));
}

}
