/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef WIDELANDS_MAP_LOADER_H
#define WIDELANDS_MAP_LOADER_H

#include <string>
#include <cstring>
#include "map_loader.h"

class FileSystem;

namespace Widelands {

class Editor_Game_Base;
struct Map_Map_Object_Loader;

/// Takes ownership of the filesystem that is passed to it.
struct WL_Map_Loader : public Map_Loader {
	WL_Map_Loader(FileSystem &, Map *);
	virtual ~WL_Map_Loader();

	virtual int32_t preload_map(bool);
	void load_world();
	virtual int32_t load_map_complete(Editor_Game_Base *, bool);

	Map_Map_Object_Loader * get_map_object_loader() {return m_mol;}

	static bool is_widelands_map(std::string const & filename) {
		return !strcasecmp(&filename.c_str()[filename.size() - 4], WLMF_SUFFIX);
	}

private:
	FileSystem & m_fs;
	std::string m_filename;
	Map_Map_Object_Loader * m_mol;
};

};

#endif
