/*
 * Copyright (C) 2013-2022 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_VERSION_PACKET_H
#define WL_MAP_IO_MAP_VERSION_PACKET_H

class FileSystem;

namespace Widelands {

class EditorGameBase;
class Map;

/*
 * This packet contains the version information of the map.
 */
class MapVersionPacket {
public:
	void read(FileSystem&, EditorGameBase&, bool skip, bool is_post_one_world);
	void write(FileSystem&, EditorGameBase&);

	void pre_read(FileSystem&, Map*, bool skip, bool is_post_one_world);
};

}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_VERSION_PACKET_H
