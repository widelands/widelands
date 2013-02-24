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
	} catch (const _wexception & e) {
		throw game_data_error(_("port_spaces data: %s"), e.what());
	}
}


void Map_Port_Spaces_Data_Packet::Write(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
	throw (_wexception)
{
	Profile prof;
	prof.create_section("global").set_int("packet_version", CURRENT_PACKET_VERSION);

	Map & map = egbase.map();
	std::vector<Coords> port_spaces = map.get_port_spaces();
	uint32_t count = 0;
	Section & s2 = prof.create_section("port_spaces");
	for (uint32_t i = 0; i < port_spaces.size(); ++i) {
		const FCoords f = map.get_fcoords(port_spaces[i]);
		// Perform some implicit cleanup while saving
		if (f.field->get_caps() & BUILDCAPS_PORT) {
			char buf[8];
			snprintf(buf, sizeof(buf), "%u", count);
			s2.set_Coords(buf, f);
			++count;
		}
	}

	// Note that we do not keep a pointer to the global section around
	// because it may have become invalid by creating the "port_spaces" section
	prof.get_section("global")->set_int("number_of_port_spaces", count);

	prof.write("port_spaces", false, fs);
}

} // namespace Widelands
