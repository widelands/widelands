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
#include "logic/playersmanager.h"
#include "wui/interactive_player.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 20;

void GamePlayerInfoPacket::read
	(FileSystem & fs, Game & game, MapObjectLoader *) {
	try {
		FileRead fr;
		fr.open(fs, "binary/player_info");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version >= 19 && packet_version <= kCurrentPacketVersion) {
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
					player.read_remaining_shipnames(fr);

					player.casualties_ = fr.unsigned_32();
					player.kills_      = fr.unsigned_32();
					player.msites_lost_         = fr.unsigned_32();
					player.msites_defeated_     = fr.unsigned_32();
					player.civil_blds_lost_     = fr.unsigned_32();
					player.civil_blds_defeated_ = fr.unsigned_32();
				}
			}

			// Result screen
			if (packet_version > 19) {
				PlayersManager* manager = game.player_manager();
				const uint8_t no_endstatus = fr.unsigned_8();
				for (uint8_t i = 0; i < no_endstatus; ++i) {
					PlayerEndStatus status;
					status.player = fr.unsigned_8();
					status.result = static_cast<PlayerEndResult>(fr.unsigned_8());
					status.time = fr.unsigned_32();
					status.info = fr.c_string();
					manager->set_player_end_status(status);
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

		fw.unsigned_8(plr->see_all_);

		fw.unsigned_8(plr->player_number_);
		fw.unsigned_8(plr->team_number());

		fw.c_string(plr->tribe().name().c_str());

		// Seen fields is in a map packet
		// Allowed buildings is in a map packet

		// Economies are in a packet after map loading

		fw.c_string(plr->name_.c_str());
		fw.c_string(plr->ai_.c_str());

		plr->write_statistics(fw);
		plr->write_remaining_shipnames(fw);
		fw.unsigned_32(plr->casualties());
		fw.unsigned_32(plr->kills     ());
		fw.unsigned_32(plr->msites_lost        ());
		fw.unsigned_32(plr->msites_defeated    ());
		fw.unsigned_32(plr->civil_blds_lost    ());
		fw.unsigned_32(plr->civil_blds_defeated());

	} else {
		fw.unsigned_8(0); //  Player is NOT in game.
	}

	// Result screen
	const std::vector<PlayerEndStatus>& end_status_list = game.player_manager()->get_players_end_status();
	fw.unsigned_8(end_status_list.size());
	for (const PlayerEndStatus& status : end_status_list) {
		fw.unsigned_8(status.player);
		fw.unsigned_8(static_cast<uint8_t>(status.result));
		fw.unsigned_32(status.time);
		fw.c_string(status.info.c_str());
	}

	game.write_statistics(fw);

	fw.write(fs, "binary/player_info");
}

}
