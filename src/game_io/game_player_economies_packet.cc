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

#include "game_io/game_player_economies_packet.h"

#include "base/macros.h"
#include "economy/economy_data_packet.h"
#include "economy/flag.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 3;

void GamePlayerEconomiesPacket::read(FileSystem& fs, Game& game, MapObjectLoader*) {
	try {
		const Map& map = game.map();
		MapIndex const max_index = map.max_index();
		PlayerNumber const nr_players = map.get_nrplayers();

		FileRead fr;
		fr.open(fs, "binary/player_economies");
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			iterate_players_existing(p, nr_players, game, player) try {
				Player::Economies& economies = player->economies_;
				for (uint32_t i = 0; i < economies.size(); ++i) {
					uint32_t value = fr.unsigned_32();
					if (value < 0xffffffff) {
						if (upcast(Flag const, flag, map[value].get_immovable())) {
							EconomyDataPacket d(flag->get_economy());
							d.read(fr);
						} else {
							throw GameDataError("there is no flag at the specified location");
						}
					} else {
						bool read_this_economy = false;

						Bob* bob = map[read_map_index_32(&fr, max_index)].get_first_bob();
						while (bob) {
							if (upcast(Ship const, ship, bob)) {

								// We are interested only in current player's ships
								if (ship->get_owner() == player) {
									assert(ship->get_economy());
									EconomyDataPacket d(ship->get_economy());
									d.read(fr);
									read_this_economy = true;
									break;
								}
							}
							bob = bob->get_next_bob();
						}
						if (!read_this_economy) {
							throw GameDataError("there is no ship at this location.");
						}
					}
				}
			} catch (const WException& e) {
				throw GameDataError("player %u: %s", p, e.what());
			}
		} else {
			throw UnhandledVersionError(
			   "GamePlayerEconomiesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("economies: %s", e.what());
	}
}

/*
 * Write Function
 */
void GamePlayerEconomiesPacket::write(FileSystem& fs, Game& game, MapObjectSaver* const) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	const Map& map = game.map();
	const Field& field_0 = map[0];
	PlayerNumber const nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		const Player::Economies& economies = player->economies_;
		for (Economy* temp_economy : economies) {
			bool wrote_this_economy = false;

			// Walk the map so that we find a representative Flag.
			for (Field const* field = &field_0; field < &map[map.max_index()]; ++field) {
				if (upcast(Flag const, flag, field->get_immovable())) {
					if (flag->get_economy() == temp_economy) {
						fw.unsigned_32(field - &field_0);

						EconomyDataPacket d(flag->get_economy());
						d.write(fw);
						wrote_this_economy = true;
						break;
					}
				}
			}
			if (wrote_this_economy)
				continue;

			// No flag found, let's look for a representative Ship. Expeditions
			// ships are special and have their own economy (which will not have a
			// flag), therefore we have to special case them.
			for (Field const* field = &field_0; field < &map[map.max_index()]; ++field) {
				Bob* bob = field->get_first_bob();
				while (bob) {
					if (upcast(Ship const, ship, bob)) {
						if (ship->get_economy() == temp_economy) {
							// TODO(sirver): the 0xffffffff is ugly and fragile.
							fw.unsigned_32(0xffffffff);  // Sentinel value.
							fw.unsigned_32(field - &field_0);
							EconomyDataPacket d(ship->get_economy());
							d.write(fw);
							wrote_this_economy = true;
							break;
						}
					}
					bob = bob->get_next_bob();
				}
			}

			// If we have not written this economy, it has no ship and no flag
			// associated. It should not exist.
			assert(wrote_this_economy);
		}
	}

	fw.write(fs, "binary/player_economies");
}
}
