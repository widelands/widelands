/*
 * Copyright (C) 2013-2021 by the Widelands Development Team
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

#include "map_io/map_version_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "io/profile.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;
// Map compatibility information for the website
constexpr const char* const kCurrentMinimumRequiredWidelandsVersion = "1.1";

void MapVersionPacket::read(FileSystem& fs,
                            EditorGameBase& egbase,
                            bool const skip,
                            bool is_post_one_world) {

	pre_read(fs, egbase.mutable_map(), skip, is_post_one_world);
}

void MapVersionPacket::pre_read(FileSystem& fs, Map* map, bool skip, bool is_post_one_world) {
	if (skip) {
		return;
	}

	Profile prof;
	try {
		prof.read("version", nullptr, fs);
	} catch (...) {
		map->map_version_.map_version_timestamp = 0;
		map->map_version_.map_creator_version = "unknown";
		return;
	}

	try {
		Section& globv = prof.get_safe_section("global");
		int32_t const packet_version = globv.get_safe_int("packet_version");
		int32_t const forward_compatibility = globv.get_safe_int("packet_compatibility");
		if ((packet_version == kCurrentPacketVersion) ||
		    (packet_version > kCurrentPacketVersion &&
		     forward_compatibility <= kCurrentPacketVersion)) {
			map->map_version_.map_source_url = globv.get_safe_string("map_source_url");
			map->map_version_.map_source_release = globv.get_safe_string("map_release");
			map->map_version_.map_creator_version = globv.get_safe_string("map_creator_version");
			map->map_version_.map_version_major = globv.get_safe_int("map_version_major");
			map->map_version_.map_version_minor = globv.get_safe_int("map_version_minor");
			uint32_t ts = static_cast<uint32_t>(globv.get_safe_int("map_version_timestamp"));
			map->map_version_.map_version_timestamp = ts;

			if (globv.has_val("minimum_required_widelands_version")) {
				map->map_version_.minimum_required_widelands_version =
				   globv.get_safe_string("minimum_required_widelands_version");
			} else if (globv.has_val("needs_widelands_version_after")) {
				map->map_version_.minimum_required_widelands_version = "build ";
				map->map_version_.minimum_required_widelands_version +=
				   std::to_string(globv.get_safe_int("needs_widelands_version_after") + 1);
			}
			map->calculate_minimum_required_widelands_version(is_post_one_world);
		} else {
			throw UnhandledVersionError("MapVersionPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("version: %s", e.what());
	}
}

void MapVersionPacket::write(FileSystem& fs, EditorGameBase& egbase) {
	Profile prof;
	Section& globs = prof.create_section("global");

	// This writes the map revision information to savegame.
	// revision information is put into a separate file, assuming that
	// revision information for bundled maps would be written to the maps
	// on-the-fly by the build script. Therefore, we need a file which
	// will not go to Software Configuration Management.

	// Maps come from three different sources:
	// - User makes those
	// - Maps are downloaded from widelands.org webpage
	// - Maps are bundled with releases.
	//
	// For maps that are downloaded from website,
	// map_source_url should be non-zero. I assume that
	// it is always widelands.org, but chose to put an url there for completeness.
	//
	// For maps bundled with a release, map_source_release should be non-empty,
	// preferably listing the release that the map came with.
	// For a map made by user, map_creator_version should list the version
	// that the map was done with.
	//
	// If there are many maps with a same name, the major version should be stepped.
	// This is mostly intended for maps downloaded from widelands.org
	// I also include minor number which is stepped at each save, and a timestamp.
	//
	// Note -- None of the version numbers are displayed anywhere. I intend to do something
	// about that in the future, and only make these pieces of data now so that most
	// running copies of widelands would be compatible and relay these data forward!
	//
	// For now, these are meaningless. Let's hope it will not stay that way!

	// TODO(unknown): -- we could store the unix time in uint32, as a partial fix to 2038 problem.
	// There seems to be a get_safe_natural method, but not corresponding setter.

	const Map& map = egbase.map();
	globs.set_string("map_source_url", map.map_version_.map_source_url);
	globs.set_string("map_release", map.map_version_.map_source_release);
	globs.set_string("map_creator_version", map.map_version_.map_creator_version);
	globs.set_int("map_version_major", map.map_version_.map_version_major);
	globs.set_int("map_version_minor", 1 + map.map_version_.map_version_minor);
	globs.set_int("map_version_timestamp", static_cast<uint32_t>(time(nullptr)));
	globs.set_int("packet_version", kCurrentPacketVersion);
	globs.set_int("packet_compatibility", kCurrentPacketVersion);
	globs.set_string("minimum_required_widelands_version", kCurrentMinimumRequiredWidelandsVersion);

	prof.write("version", false, fs);
}
}  // namespace Widelands
