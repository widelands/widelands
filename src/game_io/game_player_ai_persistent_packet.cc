/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "game_io/game_player_ai_persistent_packet.h"

#include "base/macros.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 3;

void GamePlayerAiPersistentPacket::read
	(FileSystem & fs, Game & game, MapObjectLoader *)
{
	try {
		const Map   &       map        = game.map();
		PlayerNumber const nr_players = map.get_nrplayers();

		FileRead fr;
		fr.open(fs, "binary/player_ai");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			iterate_players_existing(p, nr_players, game, player)
				try {
					player->ai_data.initialized = fr.unsigned_8();
					player->ai_data.colony_scan_area = fr.unsigned_32();
					player->ai_data.trees_around_cutters = fr.unsigned_32();
					player->ai_data.expedition_start_time = fr.unsigned_32();
					player->ai_data.ships_utilization = fr.unsigned_16();
					player->ai_data.no_more_expeditions = fr.unsigned_8();
					player->ai_data.last_attacked_player = fr.signed_16();
					player->ai_data.least_military_score = fr.unsigned_32();
					player->ai_data.target_military_score = fr.unsigned_32();
					player->ai_data.ai_personality_military_dismatlement = fr.signed_16();
					player->ai_data.ai_personality_attack_margin = fr.signed_32();
					player->ai_data.ai_productionsites_ratio = fr.unsigned_32();
					player->ai_data.ai_personality_wood_difference = fr.signed_32();
					player->ai_data.ai_personality_early_militarysites = fr.unsigned_32();
					player->ai_data.last_soldier_trained = fr.unsigned_32();
					player->ai_data.ai_personality_mil_upper_limit = fr.signed_32();

				} catch (const WException & e) {
					throw GameDataError("player %u: %s", p, e.what());
				}
		} else {
			throw UnhandledVersionError("GamePlayerAiPersistentPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
		throw GameDataError("ai data: %s", e.what());
	}
}

/*
 * Write Function
 */
void GamePlayerAiPersistentPacket::write
	(FileSystem & fs, Game & game, MapObjectSaver * const)
{
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map & map = game.map();
	PlayerNumber const nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		fw.unsigned_8(player->ai_data.initialized);
		fw.unsigned_32(player->ai_data.colony_scan_area);
		fw.unsigned_32(player->ai_data.trees_around_cutters);
		fw.unsigned_32(player->ai_data.expedition_start_time);
		fw.unsigned_16(player->ai_data.ships_utilization);
		fw.unsigned_8(player->ai_data.no_more_expeditions);
		fw.signed_16(player->ai_data.last_attacked_player);
		fw.unsigned_32(player->ai_data.least_military_score);
		fw.unsigned_32(player->ai_data.target_military_score);
		fw.signed_16(player->ai_data.ai_personality_military_dismatlement);
		fw.signed_32(player->ai_data.ai_personality_attack_margin);
		fw.unsigned_32(player->ai_data.ai_productionsites_ratio);
		fw.signed_32(player->ai_data.ai_personality_wood_difference);
		fw.unsigned_32(player->ai_data.ai_personality_early_militarysites);
		fw.unsigned_32(player->ai_data.last_soldier_trained);
		fw.signed_32(player->ai_data.ai_personality_mil_upper_limit);
	}

	fw.write(fs, "binary/player_ai");
}

}
