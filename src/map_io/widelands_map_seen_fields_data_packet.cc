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

#include "widelands_map_seen_fields_data_packet.h"

#include "logic/editor_game_base.h"
#include "map.h"
#include "logic/player.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"

#include "log.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Seen_Fields_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	if (skip)
		return;

	FileRead fr;
	try {fr.Open(fs, "binary/seen_fields");} catch (...) {return;}


	compile_assert(MAX_PLAYERS < 32);
	Map & map = egbase.map();
	const Player_Number nr_players = map.get_nrplayers();
	Map_Index const max_index = map.max_index();
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == 1)
			for (Map_Index i = 0; i < max_index; ++i) {
				uint32_t const data = fr.Unsigned16();
				for (uint8_t j = 0; j < nr_players; ++j) {
					bool const see = data & (1 << j);
					if (Player * const player = egbase.get_player(j + 1))
						player->m_fields[i].vision = see ? 1 : 0;
					else if (see)
						log
							("Map_Seen_Fields_Data_Packet::Read: WARNING: Player %u, "
							 "which does not exist, sees field %u.\n",
							 j + 1, i);
				}
			}
		else if (packet_version == CURRENT_PACKET_VERSION)
			for (Map_Index i = 0; i < max_index; ++i) {
				uint32_t const data = fr.Unsigned32();
				for (uint8_t j = 0; j < nr_players; ++j) {
					bool see = data & (1 << j);
					if (Player * const player = egbase.get_player(j + 1))
						player->m_fields[i].vision = see ? 1 : 0;
					else if (see)
					log
						("Map_Seen_Fields_Data_Packet::Read: WARNING: Player %u, "
						 "which does not exist, sees field %u.\n",
						 j + 1, i);
				}
			}
		else
			throw wexception("unknown/unhandled version %u", packet_version);
	} catch (_wexception const & e) {
		throw wexception("seen: %s", e.what());
	}
}


void Map_Seen_Fields_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver * const)
throw (_wexception)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	compile_assert(MAX_PLAYERS < 32);
	Map & map = egbase.map();
	const Player_Number nr_players = map.get_nrplayers();
	Map_Index const max_index = map.max_index();
	for (Map_Index i = 0; i < max_index; ++i) {
		uint32_t data = 0;
		for (uint8_t j = 0; j < nr_players; ++j) {
			const uint8_t player_index = j + 1;
			if (Player const * const player = egbase.get_player(player_index))
				data |= ((0 < player->vision(i)) << j);
		}
		fw.Unsigned32(data);
	}

	fw.Write(fs, "binary/seen_fields");
}

}
