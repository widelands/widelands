/*
 * Copyright (C) 2002-2004, 2007-2008, 2010-2011 by the Widelands Development Team
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

#include "map_io/widelands_map_map_object_loader.h"

#include "logic/editor_game_base.h"
#include "logic/instances.h"
#include "wexception.h"

namespace Widelands {

/*
 * Returns true if this object has already been inserted
 */
bool Map_Map_Object_Loader::is_object_known(Serial const n) {
	return m_objects.find(n) != m_objects.end();
}


/*
 * mark this object as saved
 */
void Map_Map_Object_Loader::mark_object_as_loaded(Map_Object & obj)
{
	m_loaded_obj[&obj] = true;
}

/*
 * Return the number of unsaved objects
 */
int32_t Map_Map_Object_Loader::get_nr_unloaded_objects()
{
	int32_t result = 0;
	std::map<Map_Object *, bool>::const_iterator const loaded_obj_end =
		m_loaded_obj.end();
	for
		(std::map<Map_Object *, bool>::const_iterator it = m_loaded_obj.begin();
		 it != loaded_obj_end;
		 ++it)
		if (!it->second)
			++result;
	return result;
}

/**
 * Mark this object to be scheduled for destruction after loading has finished.
 *
 * \note Only use this for compatibility hacks!
 */
void Map_Map_Object_Loader::schedule_destroy(Map_Object & obj)
{
	m_schedule_destroy.push_back(&obj);
}

/**
 * Mark this bob to have a forced act scheduled, so that it can refresh itself
 * and update states properly.
 *
 * \note Only use this for compatibility hacks!
 */
void Map_Map_Object_Loader::schedule_act(Bob & bob)
{
	m_schedule_act.push_back(&bob);
}

/**
 * Part of compatibility hacks that need to be run after the load has finished.
 *
 * \note Only use this for compatibility hacks!
 */
void Map_Map_Object_Loader::load_finish_game(Game & g)
{
	while (!m_schedule_destroy.empty()) {
		m_schedule_destroy.back()->schedule_destroy(g);
		m_schedule_destroy.pop_back();
	}

	while (!m_schedule_act.empty()) {
		m_schedule_act.back()->schedule_act(g, 1);
		m_schedule_act.pop_back();
	}
}

}
