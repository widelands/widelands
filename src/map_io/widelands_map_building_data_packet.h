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

#ifndef WIDELANDS_MAP_BUILDING_DATA_PACKET_H
#define WIDELANDS_MAP_BUILDING_DATA_PACKET_H

#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_data_packet.h"

namespace Widelands {

class Building;

/*
 * This packet cares for the existence of buildings
 * on the map, the data is parsed somewhere else
 */
struct Map_Building_Data_Packet : public Map_Data_Packet {
	void Read
		(FileSystem &, Editor_Game_Base &, bool, Map_Map_Object_Loader &) override
	;
	void Write(FileSystem &, Editor_Game_Base &, Map_Map_Object_Saver &) override
	;

protected:
	void read_priorities (Building       &, FileRead  &);
	void write_priorities(const Building &, FileWrite &);
};

}

#endif
