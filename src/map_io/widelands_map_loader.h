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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WIDELANDS_MAP_LOADER_H
#define WIDELANDS_MAP_LOADER_H

#include <cstring>
#include <string>

#include "map_io/map_loader.h"

class FileSystem;

namespace Widelands {

class Editor_Game_Base;
class Map_Map_Object_Loader;

/// Takes ownership of the filesystem that is passed to it.
struct WL_Map_Loader : public Map_Loader {
	// Takes ownership of 'fs'.
	WL_Map_Loader(FileSystem* fs, Map *);
	virtual ~WL_Map_Loader();

	virtual int32_t preload_map(bool) override;
	void load_world() override;
	virtual int32_t load_map_complete(Editor_Game_Base &, bool) override;

	Map_Map_Object_Loader * get_map_object_loader() {return m_mol;}

	static bool is_widelands_map(const std::string & filename) {
		return !strcasecmp(&filename.c_str()[filename.size() - 4], WLMF_SUFFIX);
	}

private:
	FileSystem* m_fs;  // not owned (owned by Map).
	std::string m_filename;
	Map_Map_Object_Loader * m_mol;
};

}

#endif
