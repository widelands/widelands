/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include "map_io/map_player_position_packet.h"

#include "io/profile.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "map_io/coords_profile.h"

namespace Widelands {

constexpr uint32_t kCurrentPacketVersion = 2;

void MapPlayerPositionPacket::read(FileSystem& fs, EditorGameBase& egbase, bool, MapObjectLoader&) {
	Profile prof;
	prof.read("player_position", nullptr, fs);
	Section& s = prof.get_safe_section("global");

	try {
		uint32_t const packet_version = s.get_safe_positive("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			//  Read all the positions
			//  This could bring trouble if one player position/ is not set (this
			//  is possible in the editor), is also -1, -1 == Coords::null().
			Map* map = egbase.mutable_map();
			Extent const extent = map->extent();
			PlayerNumber const nr_players = map->get_nrplayers();
			iterate_player_numbers(p, nr_players) {
				try {
					map->set_starting_pos(
					   p,
					   get_safe_coords(bformat("player_%u", static_cast<unsigned int>(p)), extent, &s));
				} catch (const WException& e) {
					throw GameDataError("player %u: %s", p, e.what());
				}
			}
		} else {
			throw UnhandledVersionError(
			   "MapPlayerPositionPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("player positions: %s", e.what());
	}
}

void MapPlayerPositionPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&) {
	Profile prof;
	Section& s = prof.create_section("global");

	s.set_int("packet_version", kCurrentPacketVersion);

	// Now, all positions in order
	const Map& map = egbase.map();
	const PlayerNumber nr_players = map.get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		set_coords(bformat("player_%u", static_cast<unsigned int>(p)), map.get_starting_pos(p), &s);
	}

	prof.write("player_position", false, fs);
}
}  // namespace Widelands
