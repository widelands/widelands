/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "game_io/game_player_info_packet.h"

#include <memory>

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "wui/interactive_player.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 30;

void GamePlayerInfoPacket::read(FileSystem& fs, Game& game, MapObjectLoader* /* mol */) {
	try {
		FileRead fr;
		fr.open(fs, "binary/player_info");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersion && packet_version >= 22) {
			uint32_t const max_players = fr.unsigned_16();
			for (uint32_t i = 1; i < max_players + 1; ++i) {
				game.remove_player(i);
				if (fr.unsigned_8()) {
					bool const see_all = fr.unsigned_8();

					int32_t const plnum = fr.unsigned_8();
					if (plnum < 1 || kMaxPlayers < plnum) {
						throw GameDataError(
						   "player number (%i) is out of range (1 .. %u)", plnum, kMaxPlayers);
					}

					// TODO(Nordfriese): Savegame compatibility, remove after v1.0
					const uint8_t playercolor_r =
					   packet_version >= 28 ? fr.unsigned_8() : kPlayerColors[i - 1].r;
					const uint8_t playercolor_g =
					   packet_version >= 28 ? fr.unsigned_8() : kPlayerColors[i - 1].g;
					const uint8_t playercolor_b =
					   packet_version >= 28 ? fr.unsigned_8() : kPlayerColors[i - 1].b;

					Widelands::TeamNumber team = fr.unsigned_8();
					char const* const tribe_name = fr.c_string();

					std::string const name = fr.c_string();

					game.add_player(plnum, 0, RGBColor(playercolor_r, playercolor_g, playercolor_b),
					                tribe_name, name, team);
					Player* player = game.get_player(plnum);
					player->set_see_all(see_all);

					player->set_ai(fr.c_string());

					if (packet_version >= 30) {
						player->set_random_tribe(fr.unsigned_8());
					}

					if (packet_version >= 23) {
						player->forbid_attack_.clear();
						uint8_t size = fr.unsigned_8();
						for (uint8_t j = 0; j < size; ++j) {
							player->forbid_attack_.emplace(fr.unsigned_8());
						}
					}

					player->read_statistics(fr, packet_version);
					player->read_remaining_shipnames(fr);

					player->casualties_ = fr.unsigned_32();
					player->kills_ = fr.unsigned_32();
					player->msites_lost_ = fr.unsigned_32();
					player->msites_defeated_ = fr.unsigned_32();
					player->civil_blds_lost_ = fr.unsigned_32();
					player->civil_blds_defeated_ = fr.unsigned_32();

					// TODO(Nordfriese): Savegame compatibility, remove after v1.0
					if (packet_version >= 24) {
						for (size_t j = fr.unsigned_32(); j; --j) {
							player->muted_building_types_.insert(fr.unsigned_32());
						}
					}
					if (packet_version >= 25) {
						player->is_picking_custom_starting_position_ = fr.unsigned_8();
						player->initialization_index_ = fr.unsigned_8();
					}
					if (packet_version >= 26) {
						player->allow_additional_expedition_items_ = fr.unsigned_8();
					}
					if (packet_version >= 27) {
						player->hidden_from_general_statistics_ = fr.unsigned_8();
					}
				}
			}

			// Result screen
			PlayersManager* manager = game.player_manager();
			const uint8_t no_endstatus = fr.unsigned_8();
			for (uint8_t i = 0; i < no_endstatus; ++i) {
				PlayerEndStatus status;
				status.player = fr.unsigned_8();
				status.result = static_cast<PlayerEndResult>(fr.unsigned_8());
				status.time = Time(fr);
				status.info = fr.c_string();
				manager->set_player_end_status(status);
			}

			game.read_statistics(fr);
		} else {
			throw UnhandledVersionError("GamePlayerInfoPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("player info: %s", e.what());
	}
}

void GamePlayerInfoPacket::write(FileSystem& fs, Game& game, MapObjectSaver* /* mos */) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	// Number of (potential) players
	PlayerNumber const nr_players = game.map().get_nrplayers();
	fw.unsigned_16(nr_players);
	iterate_players_existing_const(p, nr_players, game, plr) {
		fw.unsigned_8(1);  // Player is in game.

		fw.unsigned_8(plr->see_all_);

		fw.unsigned_8(plr->player_number_);
		fw.unsigned_8(plr->get_playercolor().r);
		fw.unsigned_8(plr->get_playercolor().g);
		fw.unsigned_8(plr->get_playercolor().b);
		fw.unsigned_8(plr->team_number());

		fw.c_string(plr->tribe().name().c_str());

		// Seen fields is in a map packet
		// Allowed buildings is in a map packet

		// Economies are in a packet after map loading

		fw.c_string(plr->name_.c_str());
		fw.c_string(plr->ai_.c_str());
		fw.unsigned_8(plr->has_random_tribe() ? 1 : 0);

		fw.unsigned_8(plr->forbid_attack_.size());
		for (const auto& it : plr->forbid_attack_) {
			fw.unsigned_8(it);
		}

		plr->write_statistics(fw);
		plr->write_remaining_shipnames(fw);
		fw.unsigned_32(plr->casualties());
		fw.unsigned_32(plr->kills());
		fw.unsigned_32(plr->msites_lost());
		fw.unsigned_32(plr->msites_defeated());
		fw.unsigned_32(plr->civil_blds_lost());
		fw.unsigned_32(plr->civil_blds_defeated());

		fw.unsigned_32(plr->muted_building_types_.size());
		for (const DescriptionIndex& di : plr->muted_building_types_) {
			fw.unsigned_32(di);
		}
		fw.unsigned_8(plr->is_picking_custom_starting_position() ? 1 : 0);
		fw.unsigned_8(plr->initialization_index_);
		fw.unsigned_8(plr->allow_additional_expedition_items_ ? 1 : 0);
		fw.unsigned_8(plr->hidden_from_general_statistics_ ? 1 : 0);
	}
	else {
		fw.unsigned_8(0);  //  Player is NOT in game.
	}

	// Result screen
	const std::vector<PlayerEndStatus>& end_status_list =
	   game.player_manager()->get_players_end_status();
	fw.unsigned_8(end_status_list.size());
	for (const PlayerEndStatus& status : end_status_list) {
		fw.unsigned_8(status.player);
		fw.unsigned_8(static_cast<uint8_t>(status.result));
		status.time.save(fw);
		fw.c_string(status.info.c_str());
	}

	game.write_statistics(fw);

	fw.write(fs, "binary/player_info");
}
}  // namespace Widelands
