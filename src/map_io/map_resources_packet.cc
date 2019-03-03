/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include "map_io/map_resources_packet.h"

#include <boost/algorithm/string.hpp>

#include "base/log.h"
#include "base/scoped_timer.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "map_io/world_legacy_lookup_table.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 2;

void MapResourcesPacket::read(FileSystem& fs,
                              EditorGameBase& egbase,
                              const WorldLegacyLookupTable& lookup_table) {
	FileRead fr;
	fr.open(fs, "binary/resource");

	Map* map = egbase.mutable_map();
	const World& world = egbase.world();

	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version >= 1 && packet_version <= kCurrentPacketVersion) {
			int32_t const nr_res = fr.unsigned_16();
			if (world.get_nr_resources() < nr_res)
				log("WARNING: Number of resources in map (%i) is bigger than in world "
				    "(%i)",
				    nr_res, world.get_nr_resources());

			// construct ids and map
			std::map<uint8_t, uint8_t> smap;
			for (uint8_t i = 0; i < nr_res; ++i) {
				uint8_t const id = fr.unsigned_16();
				const std::string resource_name = lookup_table.lookup_resource(fr.c_string());
				int32_t const res = world.get_resource(resource_name.c_str());
				if (res == Widelands::INVALID_INDEX)
					throw GameDataError(
					   "MapResourcesPacket: Resource '%s' exists in map but not in world", resource_name.c_str());
				smap[id] = res;
			}

			if (packet_version == 1) {
				// TODO(GunChleoc): Savegame compatibility, remove after Build21
				for (uint16_t y = 0; y < map->get_height(); ++y) {
					for (uint16_t x = 0; x < map->get_width(); ++x) {
						DescriptionIndex const id = fr.unsigned_8();
						ResourceAmount const amount = fr.unsigned_8();
						ResourceAmount const start_amount = fr.unsigned_8();
						const auto fcoords = map->get_fcoords(Coords(x, y));
						map->initialize_resources(fcoords, smap[id], start_amount);
						map->set_resources(fcoords, amount);
					}
				}
			} else {
				// String of entries of the form id-amount-initial_amount|
				std::string input = fr.c_string();
				std::vector<std::string> entries;
				boost::split(entries, input, boost::is_any_of("|"));
				// Check for correct size. We have a dangling |, so we expect 1 extra entry.
				if (static_cast<int>((entries.size() - 1)) != (map->get_height() * map->get_width())) {
					throw GameDataError(
					   "MapResourcesPacket: There should be map resources for %d fields, but we have %" PRIuS,
								map->get_height() * map->get_width(),
								entries.size() - 1);
				}
				auto iterator = entries.begin();
				for (uint16_t y = 0; y < map->get_height(); ++y) {
					for (uint16_t x = 0; x < map->get_width(); ++x) {
						std::vector<std::string> addme;
						boost::split(addme, *iterator++, boost::is_any_of("-"));
						if (addme.size() != 3) {
							throw GameDataError(
							   "MapResourcesPacket: Resource info for (%d,%d) should have 3 entries (ID, amount, starting amount), but we have %" PRIuS,
										x, y, addme.size());
						}
						DescriptionIndex const id = static_cast<DescriptionIndex>(atoi(addme.at(0).c_str()));
						ResourceAmount const amount = static_cast<ResourceAmount>(atoi(addme.at(1).c_str()));
						ResourceAmount const initial_amount = static_cast<ResourceAmount>(atoi(addme.at(2).c_str()));
						const auto fcoords = map->get_fcoords(Coords(x, y));
						map->initialize_resources(fcoords, smap[id], initial_amount);
						map->set_resources(fcoords, amount);
					}
				}
			}
		} else {
			throw UnhandledVersionError("MapResourcesPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("port spaces: %s", e.what());
	}
}

/*
 * Ok, when we're called from the editor, the default resources
 * are not set, which is ok.
 * When we are called from a game, the default resources are set
 * which is also ok. But this is one reason why save game != saved map
 * in nearly all cases.
 */
void MapResourcesPacket::write(FileSystem& fs, EditorGameBase& egbase) {
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	// This is a bit more complicated saved so that the order of loading
	// of the resources at run time doesn't matter.
	// (saved like terrains)
	// Write the number of resources
	const Map& map = egbase.map();
	const World& world = egbase.world();
	uint8_t const nr_res = world.get_nr_resources();
	fw.unsigned_16(nr_res);

	//  write all resources names and their id's
	for (int32_t i = 0; i < nr_res; ++i) {
		const ResourceDescription& res = *world.get_resource(i);
		fw.unsigned_16(i);
		fw.c_string(res.name().c_str());
	}

	// Performance: We write our resource information into a single string to recude the number of write operations
	std::ostringstream oss;
	for (uint16_t y = 0; y < map.get_height(); ++y) {
		for (uint16_t x = 0; x < map.get_width(); ++x) {
			const Field& f = map[Coords(x, y)];
			oss << static_cast<unsigned int>(f.get_resources()) << "-";
			oss << static_cast<unsigned int>(f.get_resources_amount()) << "-";
			oss << static_cast<unsigned int>(f.get_initial_res_amount()) << "|";
		}
	}
	fw.c_string(oss.str());

	fw.write(fs, "binary/resource");
}
}  // namespace Widelands
