/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "map_io/widelands_map_player_position_data_packet.h"

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Player_Position_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader &)
{
	Profile prof;
	prof.read("player_position", nullptr, fs);
	Section & s = prof.get_safe_section("global");

	try {
		uint32_t const packet_version = s.get_safe_positive("packet_version");
		if (2 <= packet_version && packet_version <= CURRENT_PACKET_VERSION) {
			//  Read all the positions
			//  This could bring trouble if one player position/ is not set (this
			//  is possible in the editor), is also -1, -1.
			Map               & map        = egbase.map();
			Extent        const extent     = map.extent       ();
			Player_Number const nr_players = map.get_nrplayers();
			iterate_player_numbers(p, nr_players)
				try {
					char buffer[10];
					snprintf(buffer, sizeof(buffer), "player_%u", p);
					map.set_starting_pos(p, s.get_safe_Coords(buffer, extent));
				} catch (const _wexception & e) {
					throw game_data_error("player %u: %s", p, e.what());
				}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("player positions: %s", e.what());
	}
}


void Map_Player_Position_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
{
	Profile prof;
	Section & s = prof.create_section("global");

	s.set_int("packet_version", CURRENT_PACKET_VERSION);

	// Now, all positions in order
	const Map & map = egbase.map();
	const Player_Number nr_players = map.get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "player_%u", p);
		s.set_Coords(buffer, map.get_starting_pos(p));
	}

	prof.write("player_position", false, fs);
}

}
