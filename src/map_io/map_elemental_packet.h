/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_ELEMENTAL_PACKET_H
#define WL_MAP_IO_MAP_ELEMENTAL_PACKET_H

#include <cstdint>
#include <string>
#include <vector>

#include "map_io/map_data_packet.h"

namespace Widelands {

class Map;

/**
 * The elemental data packet contains all basic and elemental data
 * like number of players, map size, world name, magic bytes and so on
 */
struct MapElementalPacket {
	MapElementalPacket() = default;

	void read(FileSystem&, EditorGameBase&, bool, MapObjectLoader&);
	void write(FileSystem&, EditorGameBase&, MapObjectSaver&);

	/// The following function prereads a given map without the need of a
	/// properly configured EditorGameBase object.
	void pre_read(FileSystem&, Map*);

	[[nodiscard]] uint32_t get_version() const {
		return version_;
	}

	/// If this map was created before the one_world merge was done, this returns
	/// the old world name, otherwise "".
	[[nodiscard]] const std::string& old_world_name() const {
		return old_world_name_;
	}

private:
	std::string old_world_name_;
	uint32_t version_ = 0;
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_ELEMENTAL_PACKET_H
