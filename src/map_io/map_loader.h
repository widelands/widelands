/*
 * Copyright (C) 2002-2004, 2007-2010 by the Widelands Development Team
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

#ifndef MAP_LOADER_H
#define MAP_LOADER_H

#include "logic/map.h"

namespace Widelands {

class Editor_Game_Base;

/// Loads a map from a file. It firsts only loads small chunks of information
/// like size, nr of players for the map select dialog. For this loading
/// function the same Map can be reused.  Then, when the player has a map
/// selected, the Map is completely filled with objects and information. When
/// now the player selects another map, this Map must be destroyed, a new one
/// must be selected.
class Map_Loader {
public:
	Map_Loader(char const * const filename, Map & M)
		: m_map(M), m_s(STATE_INIT) {m_map.set_filename(filename);}
	virtual ~Map_Loader() {};

	virtual int32_t preload_map(bool as_scenario) = 0;
	virtual void load_world() = 0;
	virtual int32_t load_map_complete(Editor_Game_Base &, bool as_scenario) = 0;

	Map & map() {return m_map;}

protected:
	enum State {
		STATE_INIT,
		STATE_PRELOADED,
		STATE_WORLD_LOADED,
		STATE_LOADED
	};
	void set_state(State const s) {m_s = s;}
	State get_state() const {return m_s;}
	Map & m_map;

private:
	State m_s;
};

}

#endif
