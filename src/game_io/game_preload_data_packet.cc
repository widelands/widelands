/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "game_preload_data_packet.h"

#include "logic/game.h"
#include "wui/interactive_player.h"
#include "map.h"
#include "profile/profile.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 2


void Game_Preload_Data_Packet::Read
	(FileSystem & fs, Game &, Map_Map_Object_Loader * const)
throw (_wexception)
{
	try {
		Profile prof;
		prof.read("preload", 0, fs);
		Section & s = prof.get_safe_section("global");
		int32_t const packet_version = s.get_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			m_gametime   = s.get_safe_int   ("gametime");
			m_mapname    = s.get_safe_string("mapname");
			m_background = s.get_safe_string("background");
			m_player_nr  = s.get_safe_int   ("player_nr");
		} else if (packet_version == 1) {
			m_gametime   = s.get_safe_int   ("gametime");
			m_mapname    = s.get_safe_string("mapname");
			m_background = "pics/progress.png";
			// Of course this is wrong, but at least player 1 is always in game
			// so widelands won't crash with this setting.
			m_player_nr  = 1;
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (_wexception const & e) {
		throw wexception("preload: %s", e.what());
	}
}


void Game_Preload_Data_Packet::Write
	(FileSystem & fs, Game & game, Map_Map_Object_Saver * const)
throw (_wexception)
{

	Profile prof;
	Section & s = prof.create_section("global");

	Interactive_Player const * const ipl = game.get_ipl();

	s.set_int   ("packet_version", ipl ? CURRENT_PACKET_VERSION : 1);

	//  save some kind of header.
	s.set_int   ("gametime",       game.get_gametime());
	Map const & map = game.map();
	s.set_string("mapname",        map.get_name());  // Name of map
	if (ipl) {
		// player that saved the game.
		s.set_int   ("player_nr",   ipl->player_number());

		std::string bg             (map.get_background());
		if (bg.empty())
			bg =                     map.get_world_name();
		s.set_string("background",  bg);
	}


	prof.write("preload", false, fs);
}

};
