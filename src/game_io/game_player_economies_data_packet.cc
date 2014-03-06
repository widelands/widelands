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

#include "game_io/game_player_economies_data_packet.h"

#include "economy/economy_data_packet.h"
#include "economy/flag.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "upcast.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 3


void Game_Player_Economies_Data_Packet::Read
	(FileSystem & fs, Game & game, Map_Map_Object_Loader *)
{
	try {
		const Map   &       map        = game.map();
		Map_Index     const max_index  = map.max_index();
		Player_Number const nr_players = map.get_nrplayers();

		FileRead fr;
		fr.Open(fs, "binary/player_economies");
		uint16_t const packet_version = fr.Unsigned16();
		if (3 <= packet_version and packet_version <= CURRENT_PACKET_VERSION) {
			iterate_players_existing(p, nr_players, game, player)
				try {
					Player::Economies & economies = player->m_economies;
					uint16_t const nr_economies = economies.size();
					Player::Economies ecos(nr_economies);
					container_iterate(Player::Economies, ecos, i) {
						uint32_t value = fr.Unsigned32();
						if (value < 0xffffffff) {
							if (upcast(Flag const, flag, map[value].get_immovable())) {
								*i.current = flag->get_economy();
								EconomyDataPacket d(flag->get_economy());
								d.Read(fr);
							} else {
								throw game_data_error("there is no flag at the specified location");
							}
						} else {
							bool read_this_economy = false;

							Bob* bob = map[fr.Map_Index32(max_index)].get_first_bob();
							while (bob) {
								if (upcast(Ship const, ship, bob)) {
									assert(ship->get_economy());
									EconomyDataPacket d(ship->get_economy());
									d.Read(fr);
									read_this_economy = true;
								}
								bob = bob->get_next_bob();
							}
							if (!read_this_economy) {
								throw game_data_error("there is no ship at this location.");
							}
						}
					}
				} catch (const _wexception & e) {
					throw game_data_error("player %u: %s", p, e.what());
				}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("economies: %s", e.what());
	}
}

/*
 * Write Function
 */
void Game_Player_Economies_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const)
{
	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	const Map & map = game.map();
	const Field & field_0 = map[0];
	Player_Number const nr_players = map.get_nrplayers();
	iterate_players_existing_const(p, nr_players, game, player) {
		const Player::Economies & economies = player->m_economies;
		container_iterate_const(Player::Economies, economies, i) {
			bool wrote_this_economy = false;

			// Walk the map so that we find a representative Flag.
			for (Field const * field = &field_0; field < &map[map.max_index()]; ++field) {
				if (upcast(Flag const, flag, field->get_immovable())) {
					if (flag->get_economy() == *i.current) {
						fw.Map_Index32(field - &field_0);

						EconomyDataPacket d(flag->get_economy());
						d.Write(fw);
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
							if (ship->get_economy() == *i.current) {
								// TODO(sirver): the 0xffffffff is ugly and fragile.
								fw.Unsigned32(0xffffffff); // Sentinel value.
								fw.Map_Index32(field - &field_0);

								EconomyDataPacket d(ship->get_economy());
								d.Write(fw);
								wrote_this_economy = true;
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

	fw.Write(fs, "binary/player_economies");
}

}
