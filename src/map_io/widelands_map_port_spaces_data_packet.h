/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#ifndef WIDELANDS_MAP_PORT_SPACES_DATA_PACKET_H
#define WIDELANDS_MAP_PORT_SPACES_DATA_PACKET_H

#include "map_io/widelands_map_data_packet.h"

namespace Widelands {

class Map;

/// The port data packet contains all port build spaces
struct Map_Port_Spaces_Data_Packet : public Map_Data_Packet {
	void Read
		(FileSystem &, Editor_Game_Base &, bool, Map_Map_Object_Loader &) override;
	void Write(FileSystem &, Editor_Game_Base &, Map_Map_Object_Saver &) override;

	//  The following function prereads a given map without the need of a
	//  properly configured Editor_Game_Base object.
	void Pre_Read(FileSystem &, Map*);

	uint32_t get_version() {return m_version;}

private:
	uint32_t m_version;
};

}

#endif
