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

#include "map_io/widelands_map_exploration_data_packet.h"

#include "log.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Exploration_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader &)
{
	if (skip)
		return;

	FileRead fr;
	try {
		fr.Open(fs, "binary/exploration");
	} catch (...) {
		try {
			fr.Open(fs, "binary/seen_fields");
		} catch (...) {
			return;
		}
	}

	static_assert(MAX_PLAYERS < 32, "assert(MAX_PLAYERS < 32) failed.");
	Map & map = egbase.map();
	Player_Number const nr_players = map.get_nrplayers();
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
							("Map_Exploration_Data_Packet::Read: WARNING: Player %u, "
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
						("Map_Exploration_Data_Packet::Read: WARNING: Player %u, "
						 "which does not exist, sees field %u.\n",
						 j + 1, i);
				}
			}
		else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("seen: %s", e.what());
	}
}


void Map_Exploration_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	static_assert(MAX_PLAYERS < 32, "assert(MAX_PLAYERS < 32) failed.");
	Map & map = egbase.map();
	Player_Number const nr_players = map.get_nrplayers();
	Map_Index const max_index = map.max_index();
	for (Map_Index i = 0; i < max_index; ++i) {
		uint32_t data = 0;
		for (uint8_t j = 0; j < nr_players; ++j) {
			uint8_t const player_index = j + 1;
			if (Player const * const player = egbase.get_player(player_index))
				data |= ((0 < player->vision(i)) << j);
		}
		fw.Unsigned32(data);
	}

	fw.Write(fs, "binary/exploration");
}

}
