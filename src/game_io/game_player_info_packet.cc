/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "game_io/game_player_info_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/constants.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "wui/interactive_player.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 18;

void GamePlayerInfoPacket::read
	(FileSystem & fs, Game & game, MapObjectLoader *) {
	try {
		FileRead fr;
		fr.open(fs, "binary/player_info");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			uint32_t const max_players = fr.unsigned_16();
			for (uint32_t i = 1; i < max_players + 1; ++i) {
				game.remove_player(i);
				if (fr.unsigned_8()) {
					bool const see_all = fr.unsigned_8();

					int32_t const plnum = fr.unsigned_8();
					if (plnum < 1 || MAX_PLAYERS < plnum)
						throw GameDataError
							("player number (%i) is out of range (1 .. %u)",
							 plnum, MAX_PLAYERS);

					Widelands::TeamNumber team = fr.unsigned_8();
					char const * const tribe_name = fr.c_string();

					std::string const name = fr.c_string();

					game.add_player(plnum, 0, tribe_name, name, team);
					Player & player = game.player(plnum);
					player.set_see_all(see_all);

					player.set_ai(fr.c_string());
					player.read_statistics(fr);

					player.m_casualties = fr.unsigned_32();
					player.m_kills      = fr.unsigned_32();
					player.m_msites_lost         = fr.unsigned_32();
					player.m_msites_defeated     = fr.unsigned_32();
					player.m_civil_blds_lost     = fr.unsigned_32();
					player.m_civil_blds_defeated = fr.unsigned_32();
					for (int32_t ai_pos = 0; ai_pos < kAIDataSize; ++ai_pos) {
						player.m_ai_data_int32[ai_pos] = fr.signed_32();
						player.m_ai_data_uint32[ai_pos] = fr.unsigned_32();
						player.m_ai_data_int16[ai_pos] = fr.unsigned_16();
					}
				}
			}
			game.read_statistics(fr);
		} else {
			throw UnhandledVersionError("GamePlayerInfoPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("player info: %s", e.what());
	}
}


void GamePlayerInfoPacket::write
	(FileSystem & fs, Game & game, MapObjectSaver *)
{
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	// Number of (potential) players
	PlayerNumber const nr_players = game.map().get_nrplayers();
	fw.unsigned_16(nr_players);
	iterate_players_existing_const(p, nr_players, game, plr) {
		fw.unsigned_8(1); // Player is in game.

		fw.unsigned_8(plr->m_see_all);

		fw.unsigned_8(plr->m_plnum);
		fw.unsigned_8(plr->team_number());

		fw.c_string(plr->tribe().name().c_str());

		// Seen fields is in a map packet
		// Allowed buildings is in a map packet

		// Economies are in a packet after map loading

		fw.c_string(plr->m_name.c_str());
		fw.c_string(plr->m_ai.c_str());

		plr->write_statistics(fw);
		fw.unsigned_32(plr->casualties());
		fw.unsigned_32(plr->kills     ());
		fw.unsigned_32(plr->msites_lost        ());
		fw.unsigned_32(plr->msites_defeated    ());
		fw.unsigned_32(plr->civil_blds_lost    ());
		fw.unsigned_32(plr->civil_blds_defeated());
		for (int32_t ai_pos = 0; ai_pos < kAIDataSize; ++ai_pos) {
			fw.signed_32(plr->m_ai_data_int32[ai_pos]);
			fw.unsigned_32(plr->m_ai_data_uint32[ai_pos]);
			fw.unsigned_16(plr->m_ai_data_int16[ai_pos]);
		}
	} else
		fw.unsigned_8(0); //  Player is NOT in game.

	game.write_statistics(fw);

	fw.write(fs, "binary/player_info");
}

}
