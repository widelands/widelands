/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
#include "economy/economy.h"
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
namespace {

constexpr uint16_t kCurrentPacketVersion = 5;

bool write_expedition_ship_economy(Economy* economy, const Map& map, FileWrite* fw) {
	for (Field const* field = &map[0]; field < &map[map.max_index()]; ++field) {
		Bob* bob = field->get_first_bob();
		while (bob) {
			if (upcast(Ship const, ship, bob)) {
				if (ship->get_economy() == economy) {
					// TODO(sirver): the 0xffffffff is ugly and fragile.
					fw->unsigned_32(0xffffffff);  // Sentinel value.
					fw->unsigned_32(field - &map[0]);
					EconomyDataPacket d(economy);
					d.write(*fw);
					return true;
				}
			}
			bob = bob->get_next_bob();
		}
	}
	return false;
}

}  // namespace

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
				const size_t num_economies = fr.unsigned_32();
				for (uint32_t i = 0; i < num_economies; ++i) {
					uint32_t value = fr.unsigned_32();
					if (value < 0xffffffff) {
						if (upcast(Flag const, flag, map[value].get_immovable())) {
							try {
								assert(flag->get_economy()->owner().player_number() ==
								       player->player_number());
								EconomyDataPacket d(flag->get_economy());
								d.read(fr);
							} catch (const GameDataError& e) {
								throw GameDataError(
								   "error reading economy data for flag at map index %d: %s", value,
								   e.what());
							}
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
									try {
										assert(ship->get_economy());
										assert(ship->get_economy()->owner().player_number() ==
										       player->player_number());
										EconomyDataPacket d(ship->get_economy());
										d.read(fr);
										read_this_economy = true;
										break;
									} catch (const GameDataError& e) {
										throw GameDataError("error reading economy data for ship %s: %s",
										                    ship->get_shipname().c_str(), e.what());
									}
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
	PlayerNumber const nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		const auto& economies = player->economies();
		fw.unsigned_32(economies.size());
		for (const auto& economy : economies) {
			Flag* arbitrary_flag = economy.second->get_arbitrary_flag();
			if (arbitrary_flag != nullptr) {
				fw.unsigned_32(map.get_fcoords(arbitrary_flag->get_position()).field - &map[0]);
				EconomyDataPacket d(economy.second.get());
				d.write(fw);
				continue;
			}

			// No flag found, let's look for a representative Ship. Expeditions
			// ships are special and have their own economy (which will not have a
			// flag), therefore we have to special case them.
			if (!write_expedition_ship_economy(economy.second.get(), map, &fw)) {
				throw GameDataError("Player %d: economy %d has no representative",
				                    player->player_number(), economy.first);
			}
		}
	}

	fw.write(fs, "binary/player_economies");
}
}  // namespace Widelands
