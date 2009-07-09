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

#include "widelands_map_flag_data_packet.h"

#include "economy/flag.h"
#include "logic/editor_game_base.h"
#include "map.h"
#include "logic/player.h"
#include "upcast.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include <map>

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Flag_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const ol)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/flag");} catch (...) {return;}

	Map const & map = egbase.map();
	Player_Number const nr_players = map.get_nrplayers();

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			for (uint16_t y = 0; y < map.get_height(); ++y)
				for (uint16_t x = 0; x < map.get_width(); ++x) {
					if (fr.Unsigned8()) {
						Player_Number const owner  = fr.Player_Number8(nr_players);
						Serial        const serial = fr.Unsigned32();

						//  No flag lives on more than one place.

						//  Now, create this Flag. Directly create it, do not call
						//  the player class since we recreate the data in another
						//  packet. We always create this, no matter what skip is
						//  since we have to read the data packets. We delete this
						//  object later again, if it is not wanted.
						try {
							ol->register_object<Flag>
								(serial,
								 *new Flag
								 	(egbase, egbase.player(owner), Coords(x, y)));
						} catch (_wexception const & e) {
							throw wexception
								("%u (at (%i, %i), owned by player %u): %s",
								 serial, x, y, owner, e.what());
						}
					}
				}
		} else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("flags: %s", e.what());
	}
}


void Map_Flag_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const os)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  Write flags and owner, register this with the map_object_saver so that
	//  it's data can be saved later.
	Map   const & map        = egbase.map();
	Field const & fields_end = map[map.max_index()];
	for (Field const * field = &map[0]; field < &fields_end; ++field)
		//  we only write flags, so the upcast is safe
		if (upcast(Flag const, flag, field->get_immovable())) {
			//  Flags can't life on multiply positions, therefore this flag
			//  shouldn't be registered.
			assert(!os->is_object_known(*flag));

			fw.Unsigned8(1);
			fw.Unsigned8(flag->owner().player_number());
			fw.Unsigned32(os->register_object(*flag));
		} else //  no existence, no owner
			fw.Unsigned8(0);

	fw.Write(fs, "binary/flag");
}

};
