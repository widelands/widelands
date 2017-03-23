/*
 * Copyright (C) 2011-2017 by the Widelands Development Team
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

#include "map_io/map_port_spaces_packet.h"

#include <string>

#include <boost/algorithm/string.hpp>

#include "base/log.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "map_io/coords_profile.h"
#include "profile/profile.h"

namespace Widelands {

constexpr int32_t kCurrentPacketVersion = 1;

void MapPortSpacesPacket::read(FileSystem& fs, EditorGameBase& egbase, bool, MapObjectLoader&) {
	Profile prof;
	prof.read("port_spaces", nullptr, fs);
	Section& s1 = prof.get_safe_section("global");

	Map& map = egbase.map();
	Extent ext(map.extent());

	try {
		int32_t const packet_version = s1.get_int("packet_version");
		if (packet_version == kCurrentPacketVersion) {
			const uint16_t num = s1.get_int("number_of_port_spaces", 0);
			if (!num)
				return;

			Section& s2 = prof.get_safe_section("port_spaces");
			for (uint16_t i = 0; i < num; ++i) {
				map.set_port_space(
				   get_safe_coords(std::to_string(static_cast<unsigned int>(i)), ext, &s2), egbase,
				   true);
			}
		} else {
			throw UnhandledVersionError("MapPortSpacesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("port_spaces data: %s", e.what());
	}
}

void MapPortSpacesPacket::write(FileSystem& fs, EditorGameBase& egbase, MapObjectSaver&)

{
	Profile prof;
	Section& s1 = prof.create_section("global");
	s1.set_int("packet_version", kCurrentPacketVersion);

	Map& map = egbase.map();
	const uint16_t num = map.get_port_spaces().size();
	s1.set_int("number_of_port_spaces", num);

	Section& s2 = prof.create_section("port_spaces");
	int i = 0;
	for (const Coords& c : map.get_port_spaces()) {
		set_coords(std::to_string(i++), c, &s2);
	}
	prof.write("port_spaces", false, fs);
}
}
