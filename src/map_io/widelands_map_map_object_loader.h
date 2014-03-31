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

#ifndef WIDELANDS_MAP_MAP_OBJECT_LOADER_H
#define WIDELANDS_MAP_MAP_OBJECT_LOADER_H

#include <map>
#include <typeinfo>

#include <stdint.h>

#include "logic/game_data_error.h"
#include "logic/instances.h"
#include "logic/widelands.h"
#include "upcast.h"


namespace Widelands {
class Bob;
class Map_Object;
class Editor_Game_Base;

/*
 * This class helps to
 *   - keep track of map objects on the map (to be loaded)
 *   - translate file index pointers into Map_Objects
 */
class Map_Map_Object_Loader {
public:
	bool is_object_known(uint32_t);

	/// Registers the object as a new one.
	///
	/// \returns a reference to the object.
	/// \throws _wexception if there is already an object registered with the
	/// same serial. (not implemented: In that case, the object is deleted.)
	///
	/// \todo Currently the object must be passed as a parameter to this
	/// function. This should be changed so that the object is allocated here.
	/// The parameter object should then be removed and the function renamed to
	/// create_object. Then there will no longer be necessary to delete the
	/// object in case the serial number is alrealy known, since the object will
	/// never even be allocated then. But this change can only be done when all
	/// kinds of map objects have suitable default constructors.
	template<typename T> T & register_object(Serial const n, T & object) {
		Reverse_Map_Object_Map::const_iterator const existing =
			m_objects.find(n);
		if (existing != m_objects.end()) {
			//delete &object; can not do this
			throw game_data_error
				("already loaded (%s)", existing->second->type_name());
		}
		m_objects.insert(std::pair<Serial, Map_Object *>(n, &object));
		m_loaded_obj[&object] = false;
		return object;
	}

	template<typename T> T & get(Serial const serial) {
		Reverse_Map_Object_Map::iterator const it = m_objects.find(serial);
		if (it == m_objects.end())
			throw game_data_error("not found");
		else if (upcast(T, result, it->second))
			return *result;
		else
			throw game_data_error
				("is a %s, expected a %s",
				 it->second->type_name(), typeid(T).name());
	}

	int32_t get_nr_unloaded_objects();
	bool is_object_loaded(Map_Object & obj) {return m_loaded_obj[&obj];}

	void mark_object_as_loaded(Map_Object &);

	void schedule_destroy(Map_Object &);
	void schedule_act(Bob &);

	void load_finish_game(Game & g);

private:
	typedef std::map<Serial, Map_Object *> Reverse_Map_Object_Map;

	std::map<Map_Object *, bool> m_loaded_obj;
	Reverse_Map_Object_Map m_objects;

	std::vector<Map_Object *> m_schedule_destroy;
	std::vector<Bob *> m_schedule_act;
};

}

#endif
