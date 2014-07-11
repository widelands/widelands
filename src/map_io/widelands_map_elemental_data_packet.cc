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

#include "map_io/widelands_map_elemental_data_packet.h"

#include <boost/algorithm/string.hpp>

#include "base/deprecated.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Elemental_Data_Packet::Pre_Read(FileSystem & fs, Map * map)
{
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	Profile prof;
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	prof.read("elemental", nullptr, fs);
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
	Section & s = prof.get_safe_section("global");
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

	try {
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		int32_t const packet_version = s.get_int("packet_version");
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		if (packet_version == CURRENT_PACKET_VERSION) {
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			log("#sirver s.get_int('map_w'): %d\n", s.get_int("map_w"));
			log("#sirver map: %p\n", map);
			map->m_width       = s.get_int   ("map_w");
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->m_height      = s.get_int   ("map_h");
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->set_nrplayers  (s.get_int   ("nr_players"));
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->set_name       (s.get_string("name"));
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->set_author     (s.get_string("author"));
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->set_description(s.get_string("descr"));
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->set_hint       (s.get_string("hint", ""));
			log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			map->set_background (s.get_string("background"));
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			old_world_name_ = s.get_string("world", "");
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			std::string t = s.get_string("tags", "");
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
			if (t != "") {
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
				std::vector<std::string> tags;
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
				boost::split(tags, t, boost::is_any_of(","));
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
				for (std::vector<std::string>::const_iterator ci = tags.begin(); ci != tags.end(); ++ci) {
					std::string tn = *ci;
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
					boost::trim(tn);
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
					map->add_tag(tn);
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
				}
			}
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);
		} else
			throw game_data_error
				("unknown/unhandled version %i", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("elemental data: %s", e.what());
	}
}


void Map_Elemental_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader &)
{
	Pre_Read(fs, &egbase.map());
}


void Map_Elemental_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
{

	Profile prof;
	Section & s = prof.create_section("global");

	s.set_int   ("packet_version", CURRENT_PACKET_VERSION);
	const Map & map = egbase.map();
	s.set_int   ("map_w",          map.get_width      ());
	s.set_int   ("map_h",          map.get_height     ());
	s.set_int   ("nr_players",     map.get_nrplayers  ());
	s.set_string("name",           map.get_name       ());
	s.set_string("author",         map.get_author     ());
	s.set_string("descr",          map.get_description());
	s.set_string("hint",           map.get_hint       ());
	s.set_string("tags", boost::algorithm::join(map.get_tags(), ","));

	prof.write("elemental", false, fs);
}

}
