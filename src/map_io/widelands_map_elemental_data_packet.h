/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#ifndef WL_MAP_IO_WIDELANDS_MAP_ELEMENTAL_DATA_PACKET_H
#define WL_MAP_IO_WIDELANDS_MAP_ELEMENTAL_DATA_PACKET_H

#include <string>

#include <stdint.h>

#include "map_io/widelands_map_data_packet.h"

namespace Widelands {

class Map;

/**
 * The elemental data packet contains all basic and elemental data
 * like number of players, map size, world name, magic bytes and so on
 */
struct Map_Elemental_Data_Packet {
	void Read(FileSystem&, Editor_Game_Base&, bool, Map_Map_Object_Loader&);
	void Write(FileSystem&, Editor_Game_Base&, Map_Map_Object_Saver&);

	/// The following function prereads a given map without the need of a
	/// properly configured Editor_Game_Base object.
	void Pre_Read(FileSystem &, Map *);

	uint32_t get_version() {return m_version;}

	/// If this map was created before the one_world merge was done, this returns
	/// the old world name, otherwise "".
	const std::string& old_world_name() const {
		return old_world_name_;
	}

private:
	std::string old_world_name_;
	uint32_t m_version;
};

}

#endif  // end of include guard: WL_MAP_IO_WIDELANDS_MAP_ELEMENTAL_DATA_PACKET_H
