/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_MAP_IO_MAP_SAVER_H
#define WL_MAP_IO_MAP_SAVER_H

#include "base/wexception.h"

class FileSystem;

namespace Widelands {

class EditorGameBase;
struct MapObjectSaver;

/*
===========================

This class saves a widelands map into a file system


NOTE: The widelands map format is mostly binary, but some stuff is
ascii. The binary is done for speeds sake, and the ASCII for easier
debugability

===========================
*/
struct MapSaver {
	MapSaver(FileSystem&, EditorGameBase&);
	~MapSaver();

	void save();
	MapObjectSaver* get_map_object_saver() {
		return mos_;
	}

private:
	EditorGameBase& egbase_;
	FileSystem& fs_;
	MapObjectSaver* mos_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_SAVER_H
