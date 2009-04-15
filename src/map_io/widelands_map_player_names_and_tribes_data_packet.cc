/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_player_names_and_tribes_data_packet.h"

#include "editor_game_base.h"
#include "map.h"
#include "profile.h"
#include "widelands_map_data_packet_ids.h"
#include "world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


Map_Player_Names_And_Tribes_Data_Packet::
~Map_Player_Names_And_Tribes_Data_Packet
	()
{}

/*
 * Read Function
 *
 * this is a scenario packet, it might be that we have to skip it
 */
void Map_Player_Names_And_Tribes_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool                    const skip,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	Pre_Read(fs, egbase.get_map(), skip);
}


void Map_Player_Names_And_Tribes_Data_Packet::Pre_Read
	(FileSystem & fs, Map * const map, bool const skip)
{
	if (skip) return;

	Profile prof;
	prof.read("player_names", 0, fs);

	try {
		int32_t const packet_version =
			prof.get_safe_section("global").get_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			Player_Number const nr_players = map->get_nrplayers();
			iterate_player_numbers(p, nr_players) {
				char buffer[10];
				snprintf(buffer, sizeof(buffer), "player_%u", p);
				Section & s = prof.get_safe_section(buffer);
				map->set_scenario_player_name (p, s.get_string("name",  ""));
				map->set_scenario_player_tribe(p, s.get_string("tribe", ""));
			}
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (_wexception const & e) {
		throw wexception("player names and tribes: %s", e.what());
	}
}


void Map_Player_Names_And_Tribes_Data_Packet::Write
	(FileSystem           &       fs,
	 Editor_Game_Base     &       egbase,
	 Map_Map_Object_Saver * const)
throw (_wexception)
{
	Profile prof;

	prof.create_section("global").set_int
		("packet_version", CURRENT_PACKET_VERSION);

	Map const & map = egbase.map();
	std::string name, tribe;
	Player_Number const nr_players = map.get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "player_%u", p);
		Section & s = prof.create_section(buffer);
		s.set_string("name",  map.get_scenario_player_name (p));
		s.set_string("tribe", map.get_scenario_player_tribe(p));
	}

	prof.write("player_names", false, fs);
}

};
