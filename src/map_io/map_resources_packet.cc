/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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

constexpr uint16_t kCurrentPacketVersion = 2;

void MapResourcesPacket::read(FileSystem& fs, EditorGameBase& egbase) {
	FileRead fr;
	fr.open(fs, "binary/resource");

	Map* map = egbase.mutable_map();

	try {
		const uint16_t packet_version = fr.unsigned_16();
		// We need to keep support for older versions around forever!!
		if (packet_version >= 1 && packet_version <= kCurrentPacketVersion) {
			uint32_t const nr_res = (packet_version >= 2) ? fr.unsigned_32() : fr.unsigned_16();

			// construct ids and map
			std::map<uint32_t, uint32_t> smap;
			for (uint32_t i = 0; i < nr_res; ++i) {
				uint32_t const id = (packet_version >= 2) ? fr.unsigned_32() : fr.unsigned_16();
				smap[id] = egbase.mutable_descriptions()->load_resource(fr.c_string());
			}

			for (uint16_t y = 0; y < map->get_height(); ++y) {
				for (uint16_t x = 0; x < map->get_width(); ++x) {
					const uint32_t id = (packet_version >= 2) ? fr.unsigned_32() : fr.unsigned_8();
					const uint32_t amount = (packet_version >= 2) ? fr.unsigned_32() : fr.unsigned_8();
					const uint32_t start_amount =
					   (packet_version >= 2) ? fr.unsigned_32() : fr.unsigned_8();
					const FCoords& fcoords = map->get_fcoords(Coords(x, y));
					const auto it = smap.find(id);
					map->initialize_resources(
					   fcoords, it == smap.end() ? kNoResource : it->second, start_amount);
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
	const uint32_t nr_res = descriptions.nr_resources();
	fw.unsigned_32(nr_res);

	//  write all resources names and their id's
	for (uint32_t i = 0; i < nr_res; ++i) {
		const ResourceDescription& res = *descriptions.get_resource_descr(i);
		fw.unsigned_32(i);
		fw.c_string(res.name().c_str());
	}

	for (uint16_t y = 0; y < map.get_height(); ++y) {
		for (uint16_t x = 0; x < map.get_width(); ++x) {
			const Field& f = map[Coords(x, y)];
			DescriptionIndex res = f.get_resources();
			ResourceAmount const amount = f.get_resources_amount();
			ResourceAmount const start_amount = f.get_initial_res_amount();
			fw.unsigned_32(res);
			fw.unsigned_32(amount);
			fw.unsigned_32(start_amount);
		}
	}

	fw.write(fs, "binary/resource");
}
}  // namespace Widelands
