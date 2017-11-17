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

// Introduction of genetic algorithm with all structures that are needed for it
constexpr uint16_t kCurrentPacketVersion = 3;
// Old Version before using genetics
constexpr uint16_t kPacketVersion2 = 2;

void GamePlayerAiPersistentPacket::read(FileSystem& fs, Game& game, MapObjectLoader*) {
	try {
		PlayerNumber const nr_players = game.map().get_nrplayers();

		FileRead fr;
		fr.open(fs, "binary/player_ai");
		uint16_t const packet_version = fr.unsigned_16();
		// TODO(GunChleoc): Savegame compatibility, remove after Build20
		if (packet_version >= kPacketVersion2 && packet_version <= kCurrentPacketVersion) {
			iterate_players_existing(p, nr_players, game, player) try {
				if (packet_version == kPacketVersion2) {
					// Packet is not compatible. Consume without using the data.
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
					// Make the AI initialize itself
					player->ai_data.initialized = 0;
				} else {
					// kCurrentPacketVersion
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
					size_t magic_numbers_size = fr.unsigned_32();

					// TODO (GunChleoc): We allow for smaller size for savegame compatibility.
					// Investigate if we can make the size static after Build 20.
					if (magic_numbers_size > Widelands::Player::AiPersistentState::kMagicNumbersSize) {
						throw GameDataError(
						   "Too many magic numbers: We have %" PRIuS " but only %" PRIuS "are allowed",
						   magic_numbers_size, Widelands::Player::AiPersistentState::kMagicNumbersSize);
					}
					assert(magic_numbers_size <=
					       Widelands::Player::AiPersistentState::kMagicNumbersSize);
					assert(player->ai_data.magic_numbers.size() ==
					       Widelands::Player::AiPersistentState::kMagicNumbersSize);
					for (size_t i = 0; i < Widelands::Player::AiPersistentState::kMagicNumbersSize;
					     ++i) {
						player->ai_data.magic_numbers.at(i) =
						   (i < magic_numbers_size) ? fr.signed_16() : 0;
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
					for (size_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
						player->ai_data.neuron_weights.at(i) = (i < neuron_pool_size) ? fr.signed_8() : 0;
					}
					assert(player->ai_data.neuron_functs.size() ==
					       Widelands::Player::AiPersistentState::kNeuronPoolSize);
					for (size_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
						player->ai_data.neuron_functs.at(i) = (i < neuron_pool_size) ? fr.signed_8() : 0;
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
					for (size_t i = 0; i < Widelands::Player::AiPersistentState::kFNeuronPoolSize; ++i) {
						player->ai_data.f_neurons.at(i) = (i < neuron_pool_size) ? fr.unsigned_32() : 0;
					}

					// Remaining buildings for basic economy
					// NOCOM reset everything with a function in ai_data
					player->ai_data.remaining_basic_buildings.clear();

					size_t remaining_basic_buildings_size = fr.unsigned_32();
					for (uint16_t i = 0; i < remaining_basic_buildings_size; ++i) {
						player->ai_data.remaining_basic_buildings.emplace(
						   static_cast<Widelands::DescriptionIndex>(fr.unsigned_32()), fr.unsigned_32());
					}
					// Basic sanity check for remaining basic buildings
					assert(player->ai_data.remaining_basic_buildings.size() <
					       player->tribe().buildings().size());
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

	PlayerNumber const nr_players = game.map().get_nrplayers();
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
			fw.unsigned_32(bb.first);
			fw.unsigned_32(bb.second);
		}
	}

	fw.write(fs, "binary/player_ai");
}
}
