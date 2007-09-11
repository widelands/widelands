/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "widelands_map_player_position_data_packet.h"

#include "editor_game_base.h"
#include "filesystem.h"
#include "map.h"
#include "profile.h"
#include "widelands_map_data_packet_ids.h"


#define CURRENT_PACKET_VERSION 2

Widelands_Map_Player_Position_Data_Packet::
~Widelands_Map_Player_Position_Data_Packet
()
{}


void Widelands_Map_Player_Position_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base * egbase,
 const bool,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   Profile prof;
   prof.read("player_position", 0, fs);
   Section* s = prof.get_section("global");

   // read packet version
	const int packet_version=s->get_int("packet_version");

	if (1 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
      // Read all the positions
      // This could bring trouble if one player position
      // is not set (this is possible in the editor), is also
      // -1, -1
		Map & map = *egbase->get_map();
		const ushort nrplayers = map.get_nrplayers();
		for (ushort i = 1; i <= nrplayers; ++i) {
			if (packet_version == 1) {
				char buf_x[64], buf_y[64];
				snprintf(buf_x, sizeof(buf_x), "player_%i_x", i);
				snprintf(buf_y, sizeof(buf_y), "player_%i_y", i);
				map.set_starting_pos
					(i, Coords(s->get_int(buf_x), s->get_int(buf_y)));
			} else {
				char buffer[64];
				snprintf(buffer, sizeof(buffer), "player_%i", i);
				map.set_starting_pos(i, s->get_Coords(buffer));
			}
		}
      return;
	}
   assert(0); // never here
}


void Widelands_Map_Player_Position_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   Profile prof;
   Section* s = prof.create_section("global");

   // packet version
   s->set_int("packet_version", CURRENT_PACKET_VERSION);

	// Now, all positions in order
	const Map & map = *egbase->get_map();
	const ushort nrplayers = map.get_nrplayers();
	for (ushort i = 1; i <= nrplayers; ++i) {
		char buffer[64];
		snprintf(buffer, sizeof(buffer), "player_%i", i);
		s->set_Coords(buffer, map.get_starting_pos(i));
	}

   prof.write("player_position", false, fs);
}
