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

#include "widelands_map_owned_fields_data_packet.h"

#include "editor_game_base.h"
#include "map.h"
#include "world.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_data_packet_ids.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Owned_Fields_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip) return;
	FileRead fr;
	try {fr.Open(fs, "binary/owned_fields");} catch (...) {return;}
	const uint16_t packet_version = fr.Unsigned16();
	if (packet_version == CURRENT_PACKET_VERSION) {
		Map & map = egbase->map();
		const Map::Index max_index = map.max_index();
		for (Map::Index i = 0; i < max_index; ++i)
			map[i].set_owned_by(fr.Unsigned8());
	} else
		throw wexception
			("Unknown version in Map_Owned_Fields_Data_Packet: %u",
			 packet_version);
}


void Map_Owned_Fields_Data_Packet::Write
(FileSystem & fs, Editor_Game_Base * egbase, Map_Map_Object_Saver * const)
throw (_wexception)
{
   FileWrite fw;

   // Now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

   // Now, all owned_fields as uint8_ts in order
	Map & map = egbase->map();
	const Map::Index max_index = map.max_index();
	for (Map::Index i = 0; i < max_index; ++i)
		fw.Unsigned8(map[i].get_owned_by());

   fw.Write(fs, "binary/owned_fields");
}

};
