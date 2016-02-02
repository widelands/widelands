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

#ifndef WL_MAP_IO_WIDELANDS_MAP_LOADER_H
#define WL_MAP_IO_WIDELANDS_MAP_LOADER_H

#include <boost/algorithm/string.hpp>
#include <memory>
#include <string>

#include "map_io/map_loader.h"

class FileSystem;
class LuaInterface;

namespace Widelands {

class EditorGameBase;
class MapObjectLoader;

/// Takes ownership of the filesystem that is passed to it.
struct WidelandsMapLoader : public MapLoader {
	// Takes ownership of 'fs'.
	WidelandsMapLoader(FileSystem* fs, Map *);
	virtual ~WidelandsMapLoader();

	int32_t preload_map(bool) override;
	int32_t load_map_complete(EditorGameBase &, MapLoader::LoadType load_type) override;

	MapObjectLoader * get_map_object_loader() {return m_mol.get();}

	static bool is_widelands_map(const std::string & filename) {
		return boost::iends_with(filename, WLMF_SUFFIX);
	}

	// If this was made pre one-world, the name of the world.
	const std::string& old_world_name() const {
		return m_old_world_name;
	}

private:
	FileSystem* m_fs;  // not owned (owned by Map).
	std::string m_filename;
	std::unique_ptr<MapObjectLoader> m_mol;
	std::string m_old_world_name;
};

}

#endif  // end of include guard: WL_MAP_IO_WIDELANDS_MAP_LOADER_H
