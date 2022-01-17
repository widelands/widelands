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

constexpr uint16_t kCurrentPacketVersion = 5;

void GamePlayerAiPersistentPacket::read(FileSystem& fs, Game& game, MapObjectLoader*) {
	try {
		PlayerNumber const nr_players = game.map().get_nrplayers();

		FileRead fr;
		fr.open(fs, "binary/player_ai");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			iterate_players_existing(p, nr_players, game, player) try {
				// Make sure that all containers are reset properly etc.
				player->ai_data.initialize();
				// Contains Genetic algorithm data
				player->ai_data.initialized = fr.unsigned_8();
				player->ai_data.colony_scan_area = fr.unsigned_32();
				player->ai_data.trees_around_cutters = fr.unsigned_32();
				player->ai_data.expedition_start_time = Time(fr);
				player->ai_data.ships_utilization = fr.unsigned_16();
				player->ai_data.no_more_expeditions = fr.unsigned_8();
				player->ai_data.last_attacked_player = fr.signed_16();
				player->ai_data.least_military_score = fr.unsigned_32();
				player->ai_data.target_military_score = fr.unsigned_32();
				player->ai_data.ai_productionsites_ratio = fr.unsigned_32();
				player->ai_data.ai_personality_mil_upper_limit = fr.signed_32();

				// Magic numbers
				const size_t magic_numbers_size = fr.unsigned_32();
				if (magic_numbers_size > Widelands::Player::AiPersistentState::kMagicNumbersSize) {
					throw GameDataError(
					   "Too many magic numbers: We have %" PRIuS " but only %" PRIuS "are allowed",
					   magic_numbers_size, Widelands::Player::AiPersistentState::kMagicNumbersSize);
				}
				assert(player->ai_data.magic_numbers.size() ==
				       Widelands::Player::AiPersistentState::kMagicNumbersSize);
				for (size_t i = 0; i < magic_numbers_size; ++i) {
					player->ai_data.magic_numbers.at(i) = fr.signed_16();
				}

				// Neurons
				const size_t neuron_pool_size = fr.unsigned_32();
				if (neuron_pool_size > Widelands::Player::AiPersistentState::kNeuronPoolSize) {
					throw GameDataError(
					   "Too many neurons: We have %" PRIuS " but only %" PRIuS "are allowed",
					   neuron_pool_size, Widelands::Player::AiPersistentState::kNeuronPoolSize);
				}
				assert(player->ai_data.neuron_weights.size() ==
				       Widelands::Player::AiPersistentState::kNeuronPoolSize);
				for (size_t i = 0; i < neuron_pool_size; ++i) {
					player->ai_data.neuron_weights.at(i) = fr.signed_8();
				}
				assert(player->ai_data.neuron_functs.size() ==
				       Widelands::Player::AiPersistentState::kNeuronPoolSize);
				for (size_t i = 0; i < neuron_pool_size; ++i) {
					player->ai_data.neuron_functs.at(i) = fr.signed_8();
				}

				// F-neurons
				const size_t f_neuron_pool_size = fr.unsigned_32();
				if (f_neuron_pool_size > Widelands::Player::AiPersistentState::kFNeuronPoolSize) {
					throw GameDataError(
					   "Too many f neurons: We have %" PRIuS " but only %" PRIuS "are allowed",
					   f_neuron_pool_size, Widelands::Player::AiPersistentState::kFNeuronPoolSize);
				}
				assert(player->ai_data.f_neurons.size() ==
				       Widelands::Player::AiPersistentState::kFNeuronPoolSize);
				for (size_t i = 0; i < f_neuron_pool_size; ++i) {
					player->ai_data.f_neurons.at(i) = fr.unsigned_32();
				}

				// Remaining buildings for basic economy
				assert(player->ai_data.remaining_basic_buildings.empty());

				size_t remaining_basic_buildings_size = fr.unsigned_32();
				for (uint16_t i = 0; i < remaining_basic_buildings_size; ++i) {
					// Buildings saved as strings
					const std::string building_string = fr.string();
					const Widelands::DescriptionIndex bld_idx =
					   player->tribe().safe_building_index(building_string);
					player->ai_data.remaining_basic_buildings.emplace(bld_idx, fr.unsigned_32());
				}
				// Basic sanity check for remaining basic buildings
				assert(player->ai_data.remaining_basic_buildings.size() <
				       player->tribe().buildings().size());

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

	PlayerNumber const nr_players = game.map().get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		fw.unsigned_8(player->ai_data.initialized ? 1 : 0);
		fw.unsigned_32(player->ai_data.colony_scan_area);
		fw.unsigned_32(player->ai_data.trees_around_cutters);
		player->ai_data.expedition_start_time.save(fw);
		fw.unsigned_16(player->ai_data.ships_utilization);
		fw.unsigned_8(player->ai_data.no_more_expeditions ? 1 : 0);
		fw.signed_16(player->ai_data.last_attacked_player);
		fw.unsigned_32(player->ai_data.least_military_score);
		fw.unsigned_32(player->ai_data.target_military_score);
		fw.unsigned_32(player->ai_data.ai_productionsites_ratio);
		fw.signed_32(player->ai_data.ai_personality_mil_upper_limit);

		// Magic numbers
		assert(player->ai_data.magic_numbers.size() ==
		       Widelands::Player::AiPersistentState::kMagicNumbersSize);
		fw.unsigned_32(player->ai_data.magic_numbers.size());
		for (int16_t magic_number : player->ai_data.magic_numbers) {
			fw.signed_16(magic_number);
		}
		// Neurons
		fw.unsigned_32(Widelands::Player::AiPersistentState::kNeuronPoolSize);
		assert(player->ai_data.neuron_weights.size() ==
		       Widelands::Player::AiPersistentState::kNeuronPoolSize);
		assert(player->ai_data.neuron_functs.size() ==
		       Widelands::Player::AiPersistentState::kNeuronPoolSize);
		for (size_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
			fw.signed_8(player->ai_data.neuron_weights.at(i));
		}
		for (size_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
			fw.signed_8(player->ai_data.neuron_functs.at(i));
		}

		// F-Neurons
		assert(player->ai_data.f_neurons.size() ==
		       Widelands::Player::AiPersistentState::kFNeuronPoolSize);
		fw.unsigned_32(player->ai_data.f_neurons.size());
		for (uint32_t f_neuron : player->ai_data.f_neurons) {
			fw.unsigned_32(f_neuron);
		}

		// Remaining buildings for basic economy
		fw.unsigned_32(player->ai_data.remaining_basic_buildings.size());
		for (auto bb : player->ai_data.remaining_basic_buildings) {
			const std::string bld_name = game.descriptions().get_building_descr(bb.first)->name();
			fw.string(bld_name);
			fw.unsigned_32(bb.second);
		}
	}

	fw.write(fs, "binary/player_ai");
}
}  // namespace Widelands
