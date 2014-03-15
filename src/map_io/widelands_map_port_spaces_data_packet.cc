/*
 * Copyright (C) 2011-2013 by the Widelands Development Team
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

#include "map_io/widelands_map_port_spaces_data_packet.h"

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

#include "container_iterate.h"
#include "log.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Port_Spaces_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader &)
{
	Profile prof;
	prof.read("port_spaces", nullptr, fs);
	Section & s1 = prof.get_safe_section("global");

	Map & map = egbase.map();
	Extent ext(map.extent());

	try {
		int32_t const packet_version = s1.get_int("packet_version");
		if (packet_version == CURRENT_PACKET_VERSION) {
			const uint16_t num = s1.get_int("number_of_port_spaces", 0);
			if (!num)
				return;

			char buf[8]; // there won't be that many port spaces... definitely!
			Section & s2 = prof.get_safe_section("port_spaces");
			for (uint16_t i = 0; i < num; ++i) {
				snprintf(buf, sizeof(buf), "%u", i);
				map.set_port_space(s2.get_safe_Coords(buf, ext), true);
			}
		} else
			throw game_data_error
				("unknown/unhandled version %i", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("port_spaces data: %s", e.what());
	}
}


void Map_Port_Spaces_Data_Packet::Write(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)

{
	Profile prof;
	Section & s1 = prof.create_section("global");
	s1.set_int("packet_version", CURRENT_PACKET_VERSION);


	// Clean up before saving: Delete port build spaces that are defined for a
	// FCoord, that can in no way be a building of size big.
	//
	// This clean up might interfer with scenarios that alter the terrain or the
	// height of the map. However those types of scenarios can be seen to be a
	// rare case in which the port spaces can be handled by rewriting port
	// spaces via a LUA script once the terrain is changed.
	Map::PortSpacesSet port_spaces;
	Map& map = egbase.map();
	BOOST_FOREACH(const Coords & c, map.get_port_spaces()) {
		FCoords fc = map.get_fcoords(c);
		if
			((map.get_max_nodecaps(fc) & BUILDCAPS_SIZEMASK) != BUILDCAPS_BIG
			 ||
			 map.find_portdock(fc).empty())
		{
			continue;
		}
		port_spaces.insert(c);
	}

	const uint16_t num = port_spaces.size();
	char buf[8]; // there won't be that many port spaces... Definitely!
	s1.set_int("number_of_port_spaces", num);

	Section & s2 = prof.create_section("port_spaces");
	int i = 0;
	BOOST_FOREACH(const Coords& c, port_spaces) {
		snprintf(buf, sizeof(buf), "%u", i++);
		s2.set_Coords(buf, c);
	}
	prof.write("port_spaces", false, fs);
}

}
