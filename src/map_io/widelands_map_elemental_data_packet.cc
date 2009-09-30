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

#include "widelands_map_elemental_data_packet.h"

#include "logic/editor_game_base.h"
#include "map.h"
#include "profile/profile.h"
#include "world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Elemental_Data_Packet::Pre_Read(FileSystem & fs, Map * map)
throw (_wexception)
{
	Profile prof;
	prof.read("elemental", 0, fs);
	Section & s = prof.get_safe_section("global");

	try {
		int32_t const packet_version = s.get_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			map->m_width       = s.get_int   ("map_w");
			map->m_height      = s.get_int   ("map_h");
			map->set_nrplayers  (s.get_int   ("nr_players"));
			map->set_world_name (s.get_string("world"));
			map->set_name       (s.get_string("name"));
			map->set_author     (s.get_string("author"));
			map->set_description(s.get_string("descr"));
			map->set_background (s.get_string("background"));
		} else
			throw wexception("unknown/unhandled version %i", packet_version);
	} catch (_wexception const & e) {
		throw wexception("elemental data: %s", e.what());
	}
}


void Map_Elemental_Data_Packet::Read
	(FileSystem            &       fs,
	 Editor_Game_Base      &       egbase,
	 bool,
	 Map_Map_Object_Loader * const)
throw (_wexception)
{
	Pre_Read(fs, &egbase.map());
}


void Map_Elemental_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver * const)
throw (_wexception)
{

	Profile prof;
	Section & s = prof.create_section("global");

	s.set_int   ("packet_version", CURRENT_PACKET_VERSION);
	Map const & map = egbase.map();
	s.set_int   ("map_w",          map.get_width      ());
	s.set_int   ("map_h",          map.get_height     ());
	s.set_int   ("nr_players",     map.get_nrplayers  ());
	s.set_string("world",          map.get_world_name ());
	s.set_string("name",           map.get_name       ());
	s.set_string("author",         map.get_author     ());
	s.set_string("descr",          map.get_description());

	prof.write("elemental", false, fs);
}

}
