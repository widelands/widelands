/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "widelands_map_seen_fields_data_packet.h"

#include "fileread.h"
#include "filewrite.h"
#include "editor_game_base.h"
#include "map.h"
#include "player.h"
#include "widelands_map_data_packet_ids.h"

#include "log.h"


#define CURRENT_PACKET_VERSION 2


Widelands_Map_Seen_Fields_Data_Packet::~Widelands_Map_Seen_Fields_Data_Packet()
{}


void Widelands_Map_Seen_Fields_Data_Packet::Read
(FileSystem & fs,
 Editor_Game_Base* egbase,
 const bool skip,
 Widelands_Map_Map_Object_Loader * const)
throw (_wexception)
{
   if (skip)
      return;

   FileRead fr;
   try {
      fr.Open(fs, "binary/seen_fields");
	} catch (...) {
      // not there, so skip
      return ;
	}

   // read packet version
   int32_t packet_version=fr.Unsigned16();

		compile_assert(MAX_PLAYERS < 32);
		Map & map = egbase->map();
		const Uint8 nr_players = map.get_nrplayers();
		const Map::Index max_index = map.max_index();
	if (packet_version == 1) for (Map::Index i = 0; i < max_index; ++i) {
		const Uint32 data = fr.Unsigned16();
		for (Uint8 j = 0; j < nr_players; ++j) {
			bool see = data & (1 << j);
			if (Player * const player = egbase->get_player(j+1))
				player->m_fields[i].vision = see ? 1 : 0;
			else if (see)
				log
					("Widelands_Map_Seen_Fields_Data_Packet::Read: WARNING: "
					 "Player %u, which does not exist, sees field %u.\n",
					 j + 1, i);
		}
	} else if (packet_version == CURRENT_PACKET_VERSION)
		for (Map::Index i = 0; i < max_index; ++i) {
			const Uint32 data = fr.Unsigned32();
			for (Uint8 j = 0; j < nr_players; ++j) {
				bool see = data & (1 << j);
				if (Player * const player = egbase->get_player(j+1))
					player->m_fields[i].vision = see ? 1 : 0;
				else if (see)
					log
						("Widelands_Map_Seen_Fields_Data_Packet::Read: WARNING: "
						 "Player %u, which does not exist, sees field %u.\n",
						 j + 1, i);
			}
		}
	else
		throw wexception
			("Unknown version in Widelands_Map_Seen_Fields_Data_Packet: %u",
			 packet_version);
}

/*
 * Write Function
 */
void Widelands_Map_Seen_Fields_Data_Packet::Write
(FileSystem & fs,
 Editor_Game_Base* egbase,
 Widelands_Map_Map_Object_Saver * const)
throw (_wexception)
{
   FileWrite fw;

   // Now packet version
   fw.Unsigned16(CURRENT_PACKET_VERSION);

	compile_assert(MAX_PLAYERS < 32);
	Map & map = egbase->map();
	const Uint8 nr_players = map.get_nrplayers();
	const Map::Index max_index = map.max_index();
	for (Map::Index i = 0; i < max_index; ++i) {
		Uint32 data = 0;
		for (Uint8 j = 0; j < nr_players; ++j) {
			const Uint8 player_index = j + 1;
			if (const Player * const player = egbase->get_player(player_index))
				data |= ((0 < player->vision(i)) << j);
		}
		fw.Unsigned32(data);
	}

   fw.Write(fs, "binary/seen_fields");
}
