/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/resource_description.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapResourcesPacket::read(FileSystem& fs,
                              EditorGameBase& egbase) {
	FileRead fr;
	fr.open(fs, "binary/resource");

	Map* map = egbase.mutable_map();

	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			int32_t const nr_res = fr.unsigned_16();

			// construct ids and map
			std::map<uint8_t, uint8_t> smap;
			for (uint8_t i = 0; i < nr_res; ++i) {
				uint8_t const id = fr.unsigned_16();
				const std::string resource_name(egbase.descriptions().lookup_resource(fr.c_string()));
				const DescriptionIndex res =
				   egbase.mutable_descriptions()->load_resource(resource_name);
				if (res == Widelands::INVALID_INDEX) {
					throw GameDataError("Unknown resource '%s' in map", resource_name.c_str());
				}
				smap[id] = res;
			}

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
	const Descriptions& descriptions = egbase.descriptions();
	uint8_t const nr_res = descriptions.nr_resources();
	fw.unsigned_16(nr_res);

	//  write all resources names and their id's
	for (int32_t i = 0; i < nr_res; ++i) {
		const ResourceDescription& res = *descriptions.get_resource_descr(i);
		fw.unsigned_16(i);
		fw.c_string(res.name().c_str());
	}

	//  Now, all resouces as uint8_ts in order
	//  - resource id
	//  - amount
	for (uint16_t y = 0; y < map.get_height(); ++y) {
		for (uint16_t x = 0; x < map.get_width(); ++x) {
			const Field& f = map[Coords(x, y)];
			DescriptionIndex res = f.get_resources();
			ResourceAmount const amount = f.get_resources_amount();
			ResourceAmount const start_amount = f.get_initial_res_amount();
			fw.unsigned_8(res);
			fw.unsigned_8(amount);
			fw.unsigned_8(start_amount);
		}
	}

	fw.write(fs, "binary/resource");
}
}  // namespace Widelands
