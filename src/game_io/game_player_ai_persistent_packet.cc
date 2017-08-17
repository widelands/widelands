/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

void GamePlayerAiPersistentPacket::read(FileSystem& fs, Game& game, MapObjectLoader*) {
	try {
		const Map& map = game.map();
		PlayerNumber const nr_players = map.get_nrplayers();

		FileRead fr;
		fr.open(fs, "binary/player_ai");
		uint16_t const packet_version = fr.unsigned_16();
		// TODO(GunChleoc): Savegame compatibility, remove after Build20
		if (packet_version >= 2 && packet_version <= kCurrentPacketVersion) {
			iterate_players_existing(p, nr_players, game, player) try {
				if (packet_version == 2) {

					// zero here says the AI has not been initialized
					player->ai_data.initialized = 0;
					// we will just read other variables
					fr.unsigned_8();
					fr.unsigned_32();
					fr.unsigned_32();
					fr.unsigned_32();
					fr.unsigned_16();
					fr.unsigned_8();
					fr.signed_16();
					fr.unsigned_32();
					fr.unsigned_32();
					fr.signed_16();
					fr.signed_32();
					fr.unsigned_32();
					fr.signed_32();
					fr.unsigned_32();
					fr.unsigned_32();
				} else {
					player->ai_data.initialized = fr.unsigned_8();
					player->ai_data.colony_scan_area = fr.unsigned_32();
					player->ai_data.trees_around_cutters = fr.unsigned_32();
					player->ai_data.expedition_start_time = fr.unsigned_32();
					player->ai_data.ships_utilization = fr.unsigned_16();
					player->ai_data.no_more_expeditions = fr.unsigned_8();
					player->ai_data.last_attacked_player = fr.signed_16();
					player->ai_data.least_military_score = fr.unsigned_32();
					player->ai_data.target_military_score = fr.unsigned_32();
					player->ai_data.ai_productionsites_ratio = fr.unsigned_32();
					player->ai_data.ai_personality_mil_upper_limit = fr.signed_32();
					// Magic numbers
					player->ai_data.magic_numbers_size = fr.unsigned_32();
					for (uint16_t i = 0; i < player->ai_data.magic_numbers_size; ++i) {
						player->ai_data.magic_numbers.push_back(fr.signed_16());
					}
					assert(player->ai_data.magic_numbers_size == player->ai_data.magic_numbers.size());
					// Neurons
					player->ai_data.neuron_pool_size = fr.unsigned_32();
					for (uint16_t i = 0; i < player->ai_data.neuron_pool_size; ++i) {
						player->ai_data.neuron_weights.push_back(fr.signed_8());
					}
					for (uint16_t i = 0; i < player->ai_data.neuron_pool_size; ++i) {
						player->ai_data.neuron_functs.push_back(fr.signed_8());
					}
					assert(player->ai_data.neuron_pool_size == player->ai_data.neuron_weights.size());
					assert(player->ai_data.neuron_pool_size == player->ai_data.neuron_functs.size());

					// F-neurons
					player->ai_data.f_neuron_pool_size = fr.unsigned_32();
					for (uint16_t i = 0; i < player->ai_data.f_neuron_pool_size; ++i) {
						player->ai_data.f_neurons.push_back(fr.unsigned_32());
					}
					assert(player->ai_data.f_neuron_pool_size == player->ai_data.f_neurons.size());

					// remaining buildings for basic economy
					player->ai_data.remaining_buildings_size = fr.unsigned_32();
					for (uint16_t i = 0; i < player->ai_data.remaining_buildings_size; ++i) {
						player->ai_data.remaining_basic_buildings[fr.unsigned_32()] = fr.unsigned_32();
					}
					assert(player->ai_data.remaining_buildings_size ==
					       player->ai_data.remaining_basic_buildings.size());
				}
			} catch (const WException& e) {
				throw GameDataError("player %u: %s", p, e.what());
			}
		} else {
			throw UnhandledVersionError(
			   "GamePlayerAiPersistentPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("ai data: %s", e.what());
	}
}

/*
 * Write Function
 */
void GamePlayerAiPersistentPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = game.map();
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
		fw.unsigned_32(player->ai_data.ai_productionsites_ratio);
		fw.signed_32(player->ai_data.ai_personality_mil_upper_limit);

		// Magic numbers
		fw.unsigned_32(player->ai_data.magic_numbers_size);
		assert(player->ai_data.magic_numbers_size == player->ai_data.magic_numbers.size());
		for (uint16_t i = 0; i < player->ai_data.magic_numbers_size; ++i) {
			fw.signed_16(player->ai_data.magic_numbers[i]);
		}
		// Neurons
		fw.unsigned_32(player->ai_data.neuron_pool_size);
		assert(player->ai_data.neuron_pool_size == player->ai_data.neuron_weights.size());
		assert(player->ai_data.neuron_pool_size == player->ai_data.neuron_functs.size());
		for (uint16_t i = 0; i < player->ai_data.neuron_pool_size; ++i) {
			fw.signed_8(player->ai_data.neuron_weights[i]);
		}
		for (uint16_t i = 0; i < player->ai_data.neuron_pool_size; ++i) {
			fw.signed_8(player->ai_data.neuron_functs[i]);
		}

		// F-Neurons
		fw.unsigned_32(player->ai_data.f_neuron_pool_size);
		assert(player->ai_data.f_neuron_pool_size == player->ai_data.f_neurons.size());

		for (uint16_t i = 0; i < player->ai_data.f_neuron_pool_size; ++i) {
			fw.unsigned_32(player->ai_data.f_neurons[i]);
		}

		// Remaining buildings for basic economy
		assert(player->ai_data.remaining_buildings_size ==
		       player->ai_data.remaining_basic_buildings.size());
		fw.unsigned_32(player->ai_data.remaining_buildings_size);
		for (auto bb : player->ai_data.remaining_basic_buildings) {
			fw.unsigned_32(bb.first);
			fw.unsigned_32(bb.second);
		}
	}

	fw.write(fs, "binary/player_ai");
}
}
