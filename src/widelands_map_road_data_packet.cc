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

#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "transport.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"
#include "widelands_map_road_data_packet.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "upcast.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Road_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip) return;

	FileRead fr;
	try {fr.Open(fs, "binary/road");} catch (...) {return;}

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			Serial serial;
			while ((serial = fr.Unsigned32()) != 0xffffffff) {
				try {
					//  If this is already known, get it.
					//  Road data is read somewhere else
					ol->register_object(serial, *new Road()).init(egbase);
				} catch (_wexception const & e) {
					throw wexception("%u: %s", serial, e.what());
				}
			}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("road: %s");
	}
}


void Map_Road_Data_Packet::Write
(FileSystem & fs, Editor_Game_Base * egbase, Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  Write roads. Register this with the map_object_saver so that its data
	//  can be saved later.
	Map const & map = egbase->map();
	Field * field = &map[0];
	Field const * const fields_end = field + map.max_index();
	for (; field < fields_end; ++field)
		if (upcast(Road const, road, field->get_immovable())) // only roads
			//  Roads can life on multiple positions.
			if (not os->is_object_known(road))
				fw.Unsigned32(os->register_object(road));
	fw.Unsigned32(0xffffffff);

	fw.Write(fs, "binary/road");
}

};
