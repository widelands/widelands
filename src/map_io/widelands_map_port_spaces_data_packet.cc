/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include <boost/algorithm/string.hpp>

#include "container_iterate.h"

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "profile/profile.h"

#include "log.h"

#include "widelands_map_port_spaces_data_packet.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Port_Spaces_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader &) throw (_wexception)
{
	Profile prof;
	prof.read("port_spaces", 0, fs);
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
				(_("unknown/unhandled version %i"), packet_version);
	} catch (_wexception const & e) {
		throw game_data_error(_("port_spaces data: %s"), e.what());
	}
}


void Map_Port_Spaces_Data_Packet::Write(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
	throw (_wexception)
{
	Profile prof;
	Section & s1 = prof.create_section("global");
	s1.set_int("packet_version", CURRENT_PACKET_VERSION);

	Map & map = egbase.map();
	std::vector<Coords> port_spaces = map.get_port_spaces();
	const uint16_t num = port_spaces.size();
	char buf[8]; // there won't be that many port spaces... definitely!
	s1.set_int("number_of_port_spaces", num);

	Section & s2 = prof.create_section("port_spaces");
	for (uint16_t i = 0; i < num; ++i) {
		snprintf(buf, sizeof(buf), "%u", i);
		s2.set_Coords(buf, port_spaces.at(i));
	}
	prof.write("port_spaces", false, fs);
}

}
