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
constexpr uint16_t kPacketVersion2 = 2; // TODO(TiborB): what exactly does this version stand for -> please rename

/**
 * skip and ignore old data, we will just read other variables
 */
static void skipVersion2(FileRead& fr) {
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
}

    static void readCurrentVersion(FileRead& fr, Player::AiPersistentState& ai_data) {
    ai_data.initialized                    = fr.unsigned_8();
    ai_data.colony_scan_area               = fr.unsigned_32();
    ai_data.trees_around_cutters           = fr.unsigned_32();
    ai_data.expedition_start_time          = fr.unsigned_32();
    ai_data.ships_utilization              = fr.unsigned_16();
    ai_data.no_more_expeditions            = fr.unsigned_8();
    ai_data.last_attacked_player           = fr.signed_16();
    ai_data.least_military_score           = fr.unsigned_32();
    ai_data.target_military_score          = fr.unsigned_32();
    ai_data.ai_productionsites_ratio       = fr.unsigned_32();
    ai_data.ai_personality_mil_upper_limit = fr.signed_32();
    // Magic numbers
    ai_data.magic_numbers_size = fr.unsigned_32();
    for (uint16_t i = 0; i < ai_data.magic_numbers_size; ++i) {
        ai_data.magic_numbers.push_back(fr.signed_16());
    }
    assert(ai_data.magic_numbers_size == ai_data.magic_numbers.size());
    // Neurons
    ai_data.neuron_pool_size = fr.unsigned_32();
    for (uint16_t i = 0; i < ai_data.neuron_pool_size; ++i) {
        ai_data.neuron_weights.push_back(fr.signed_8());
    }
    for (uint16_t i = 0; i < ai_data.neuron_pool_size; ++i) {
        ai_data.neuron_functs.push_back(fr.signed_8());
    }
    assert(ai_data.neuron_pool_size == ai_data.neuron_weights.size());
    assert(ai_data.neuron_pool_size == ai_data.neuron_functs.size());

    // F-neurons
    ai_data.f_neuron_pool_size = fr.unsigned_32();
    for (uint16_t i = 0; i < ai_data.f_neuron_pool_size; ++i) {
        ai_data.f_neurons.push_back(fr.unsigned_32());
    }
    assert(ai_data.f_neuron_pool_size == ai_data.f_neurons.size());

    // remaining buildings for basic economy
    ai_data.remaining_buildings_size = fr.unsigned_32();
    for (uint16_t i = 0; i < ai_data.remaining_buildings_size; ++i) {
        ai_data.remaining_basic_buildings.emplace(static_cast<Widelands::DescriptionIndex>(fr.unsigned_32()), fr.unsigned_32());
    }
    assert(ai_data.remaining_buildings_size ==
           ai_data.remaining_basic_buildings.size());

}

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

					player->ai_data.initialized = 0; // AI has not been initialized
                    skipVersion2(fr);
                }
                else // kCurrentPacketVersion
                {
                    readCurrentVersion(fr, player->ai_data);
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

static void writeCurrentVersion(FileWrite& fw, const Player::AiPersistentState& ai_data) {
    fw.unsigned_8(ai_data.initialized);
    fw.unsigned_32(ai_data.colony_scan_area);
    fw.unsigned_32(ai_data.trees_around_cutters);
    fw.unsigned_32(ai_data.expedition_start_time);
    fw.unsigned_16(ai_data.ships_utilization);
    fw.unsigned_8(ai_data.no_more_expeditions);
    fw.signed_16(ai_data.last_attacked_player);
    fw.unsigned_32(ai_data.least_military_score);
    fw.unsigned_32(ai_data.target_military_score);
    fw.unsigned_32(ai_data.ai_productionsites_ratio);
    fw.signed_32(ai_data.ai_personality_mil_upper_limit);

    // Magic numbers
    fw.unsigned_32(ai_data.magic_numbers_size);
    assert(ai_data.magic_numbers_size == ai_data.magic_numbers.size());
    for (uint16_t i = 0; i < ai_data.magic_numbers_size; ++i) {
        fw.signed_16(ai_data.magic_numbers[i]);
    }
    // Neurons
    fw.unsigned_32(ai_data.neuron_pool_size);
    assert(ai_data.neuron_pool_size == ai_data.neuron_weights.size());
    assert(ai_data.neuron_pool_size == ai_data.neuron_functs.size());
    for (uint16_t i = 0; i < ai_data.neuron_pool_size; ++i) {
        fw.signed_8(ai_data.neuron_weights[i]);
    }
    for (uint16_t i = 0; i < ai_data.neuron_pool_size; ++i) {
        fw.signed_8(ai_data.neuron_functs[i]);
    }

    // F-Neurons
    fw.unsigned_32(ai_data.f_neuron_pool_size);
    assert(ai_data.f_neuron_pool_size == ai_data.f_neurons.size());

    for (uint16_t i = 0; i < ai_data.f_neuron_pool_size; ++i) {
        fw.unsigned_32(ai_data.f_neurons[i]);
    }

    // Remaining buildings for basic economy
    assert(ai_data.remaining_buildings_size ==
           ai_data.remaining_basic_buildings.size());
    fw.unsigned_32(ai_data.remaining_buildings_size);
    for (auto bb : ai_data.remaining_basic_buildings) {
        fw.unsigned_32(bb.first);
        fw.unsigned_32(bb.second);
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
        writeCurrentVersion(fw, player->ai_data);
	}

	fw.write(fs, "binary/player_ai");
}
}
