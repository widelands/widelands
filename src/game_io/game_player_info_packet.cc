/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

/* Changelog:
 * Version 30: Release 1.1
 * Version 31: Added warehouse names.
 * Version 32: Added fleet statistics (in Game::read_statistics),
 *             naval attack stats, and detected port spaces.
 */
constexpr uint16_t kCurrentPacketVersion = 32;

void GamePlayerInfoPacket::read(FileSystem& fs, Game& game, MapObjectLoader* /* mol */) {
	try {
		FileRead fr;
		fr.open(fs, "binary/player_info");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version <= kCurrentPacketVersion && packet_version >= 30) {
			uint32_t const max_players = fr.unsigned_16();
			for (uint32_t i = 1; i < max_players + 1; ++i) {
				game.remove_player(i);
				if (fr.unsigned_8() != 0u) {
					bool const see_all = fr.unsigned_8() != 0u;

					int32_t const plnum = fr.unsigned_8();
					if (plnum < 1 || kMaxPlayers < plnum) {
						throw GameDataError(
						   "player number (%i) is out of range (1 .. %u)", plnum, kMaxPlayers);
					}

					const uint8_t playercolor_r = fr.unsigned_8();
					const uint8_t playercolor_g = fr.unsigned_8();
					const uint8_t playercolor_b = fr.unsigned_8();

					Widelands::TeamNumber team = fr.unsigned_8();
					char const* const tribe_name = fr.c_string();

					std::string const name = fr.c_string();

					game.add_player(plnum, 0, RGBColor(playercolor_r, playercolor_g, playercolor_b),
					                tribe_name, name, team);
					Player* player = game.get_player(plnum);
					player->set_see_all(see_all);

					player->set_ai(fr.c_string());
					player->set_random_tribe(fr.unsigned_8() != 0u);

					player->forbid_attack_.clear();
					uint8_t size = fr.unsigned_8();
					for (uint8_t j = 0; j < size; ++j) {
						player->forbid_attack_.emplace(fr.unsigned_8());
					}

					player->read_statistics(fr, packet_version);
					player->read_remaining_shipnames(fr);
					if (packet_version >= 31) {
						// TODO(Nordfriese): Savegame compatibility, remove after v1.2
						player->read_remaining_warehousenames(fr);
					}

					player->casualties_ = fr.unsigned_32();
					player->kills_ = fr.unsigned_32();
					player->msites_lost_ = fr.unsigned_32();
					player->msites_defeated_ = fr.unsigned_32();
					player->civil_blds_lost_ = fr.unsigned_32();
					player->civil_blds_defeated_ = fr.unsigned_32();
					player->naval_victories_ = packet_version >= 32 ? fr.unsigned_32() : 0;
					player->naval_losses_ = packet_version >= 32 ? fr.unsigned_32() : 0;

					for (uint32_t j = packet_version >= 32 ? fr.unsigned_32() : 0; j > 0; --j) {
						std::unique_ptr<DetectedPortSpace> dps(new DetectedPortSpace());
						dps->set_serial(fr.unsigned_32());
						dps->coords.x = fr.signed_16();
						dps->coords.y = fr.signed_16();
						for (uint32_t k = fr.unsigned_32(); k > 0; --k) {
							int16_t x = fr.signed_16();
							int16_t y = fr.signed_16();
							dps->dockpoints.emplace_back(x, y);
						}
						dps->owner = fr.unsigned_8();
						dps->time_discovered = Time(fr);
						dps->discovering_ship = fr.string();
						dps->nearest_portdock = fr.string();
						dps->direction_from_portdock = static_cast<CompassDir>(fr.unsigned_8());
						player->detected_port_spaces_.push_back(std::move(dps));
					}

					for (size_t j = fr.unsigned_32(); j > 0; --j) {
						player->muted_building_types_.insert(fr.unsigned_32());
					}
					player->is_picking_custom_starting_position_ = (fr.unsigned_8() != 0u);
					player->initialization_index_ = fr.unsigned_8();
					player->allow_additional_expedition_items_ = (fr.unsigned_8() != 0u);
					player->hidden_from_general_statistics_ = (fr.unsigned_8() != 0u);
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
				manager->add_player_end_status(status, true);
			}

			game.read_statistics(fr, packet_version);
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

		fw.unsigned_8(static_cast<uint8_t>(plr->see_all_));

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
		plr->write_remaining_warehousenames(fw);
		fw.unsigned_32(plr->casualties());
		fw.unsigned_32(plr->kills());
		fw.unsigned_32(plr->msites_lost());
		fw.unsigned_32(plr->msites_defeated());
		fw.unsigned_32(plr->civil_blds_lost());
		fw.unsigned_32(plr->civil_blds_defeated());
		fw.unsigned_32(plr->naval_victories());
		fw.unsigned_32(plr->naval_losses());

		fw.unsigned_32(plr->detected_port_spaces_.size());
		for (const auto& dps : plr->detected_port_spaces_) {
			fw.unsigned_32(dps->serial);
			fw.signed_16(dps->coords.x);
			fw.signed_16(dps->coords.y);
			fw.unsigned_32(dps->dockpoints.size());
			for (const Coords& c : dps->dockpoints) {
				fw.signed_16(c.x);
				fw.signed_16(c.y);
			}
			fw.unsigned_8(dps->owner);
			dps->time_discovered.save(fw);
			fw.string(dps->discovering_ship);
			fw.string(dps->nearest_portdock);
			fw.unsigned_8(static_cast<uint8_t>(dps->direction_from_portdock));
		}

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
	const std::map<Widelands::PlayerNumber, Widelands::PlayerEndStatus>& end_status_map =
	   game.player_manager()->get_all_players_end_status();
	fw.unsigned_8(end_status_map.size());
	for (const auto& it : end_status_map) {
		const Widelands::PlayerEndStatus& status = it.second;
		fw.unsigned_8(status.player);
		fw.unsigned_8(static_cast<uint8_t>(status.result));
		status.time.save(fw);
		fw.c_string(status.info.c_str());
	}

	game.write_statistics(fw);

	fw.write(fs, "binary/player_info");
}
}  // namespace Widelands
