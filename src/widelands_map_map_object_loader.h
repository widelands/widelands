/*
 * Copyright (C) 2002-2004, 2007-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WIDELANDS_MAP_MAP_OBJECT_LOADER_H
#define WIDELANDS_MAP_MAP_OBJECT_LOADER_H

namespace Widelands {

class Map_Object;
class Editor_Game_Base;

/*
 * This class helps to
 *   - keep track of map objects on the map (to be loaded)
 *   - translate file index pointers into Map_Objects
 */
struct Map_Map_Object_Loader {
	bool is_object_known(uint32_t);
	void register_object(Editor_Game_Base*, uint32_t, Map_Object*);

	Map_Object* get_object_by_file_index(uint32_t);

	int32_t get_nr_unloaded_objects();
	bool is_object_loaded(Map_Object* obj) {return m_loaded_obj[obj];}

	void mark_object_as_loaded(Map_Object* obj);

private:
	typedef std::map<uint32_t, Map_Object*> Reverse_Map_Object_Map;

	std::map<Map_Object*, bool> m_loaded_obj;
	Reverse_Map_Object_Map m_objects;
};

};

#endif
