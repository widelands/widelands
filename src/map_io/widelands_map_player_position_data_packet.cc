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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "widelands_map_player_position_data_packet.h"

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Map_Player_Position_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader &)
throw (_wexception)
{
	Profile prof;
	prof.read("player_position", 0, fs);
	Section & s = prof.get_safe_section("global");

	try {
		uint32_t const packet_version = s.get_safe_positive("packet_version");
		if (packet_version <= CURRENT_PACKET_VERSION) {
			//  Read all the positions
			//  This could bring trouble if one player position/ is not set (this
			//  is possible in the editor), is also -1, -1.
			Map               & map        = egbase.map();
			Extent        const extent     = map.extent       ();
			Player_Number const nr_players = map.get_nrplayers();
			iterate_player_numbers(p, nr_players)
				try {
					if (packet_version == 1) {
						char buf_x[12], buf_y[12];
						snprintf(buf_x, sizeof(buf_x), "player_%u_x", p);
						snprintf(buf_y, sizeof(buf_y), "player_%u_y", p);
						map.set_starting_pos
							(p, Coords(s.get_int(buf_x), s.get_int(buf_y)));
					} else {
						char buffer[10];
						snprintf(buffer, sizeof(buffer), "player_%u", p);
						map.set_starting_pos(p, s.get_safe_Coords(buffer, extent));
					}
				} catch (_wexception const & e) {
					throw game_data_error(_("player %u: %s"), p, e.what());
				}
		} else
			throw game_data_error
				(_("unknown/unhandled version %u"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("player positions: %s"), e.what());
	}
}


void Map_Player_Position_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
throw (_wexception)
{
	Profile prof;
	Section & s = prof.create_section("global");

	s.set_int("packet_version", CURRENT_PACKET_VERSION);

	// Now, all positions in order
	Map const & map = egbase.map();
	const Player_Number nr_players = map.get_nrplayers();
	iterate_player_numbers(p, nr_players) {
		char buffer[10];
		snprintf(buffer, sizeof(buffer), "player_%u", p);
		s.set_Coords(buffer, map.get_starting_pos(p));
	}

	prof.write("player_position", false, fs);
}

}
