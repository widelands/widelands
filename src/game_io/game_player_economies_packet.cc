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

constexpr uint16_t kCurrentPacketVersion = 4;

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
		if (packet_version == 3 || packet_version == kCurrentPacketVersion) {
			iterate_players_existing(p, nr_players, game, player) try {
				// In packet_version 4 we dump the number of economies a player had at
				// save time to debug
				// https://bugs.launchpad.net/widelands/+bug/1654897 which is likely
				// caused by players having more economies at load than they had at
				// save.
				Player::Economies& economies = player->economies_;
				if (packet_version > 3) {
					const size_t num_economies = fr.unsigned_16();
					if (num_economies != economies.size()) {
						throw GameDataError("Num economies on save (%ld) != Num economies on load (%ld)",
						                    num_economies, economies.size());
					}
				}

				for (uint32_t i = 0; i < economies.size(); ++i) {
					uint32_t value = fr.unsigned_32();
					if (value < 0xffffffff) {
						if (upcast(Flag const, flag, map[value].get_immovable())) {
							assert(flag->get_economy()->owner().player_number() ==
							       player->player_number());
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
									assert(ship->get_economy()->owner().player_number() ==
									       player->player_number());
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
	PlayerNumber const nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		const Player::Economies& economies = player->economies_;
		fw.unsigned_16(economies.size());
		for (Economy* economy : economies) {
			Flag* arbitrary_flag = economy->get_arbitrary_flag();
			if (arbitrary_flag != nullptr) {
				fw.unsigned_32(map.get_fcoords(arbitrary_flag->get_position()).field - &map[0]);
				EconomyDataPacket d(economy);
				d.write(fw);
				continue;
			}

			// No flag found, let's look for a representative Ship. Expeditions
			// ships are special and have their own economy (which will not have a
			// flag), therefore we have to special case them.
			if (!write_expedition_ship_economy(economy, map, &fw)) {
				throw GameDataError("economy without representative");
			}
		}
	}

	fw.write(fs, "binary/player_economies");
}
}
