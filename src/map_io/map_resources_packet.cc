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

#include "map_io/map_resources_packet.h"

#include "base/log.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/world/resource_description.h"
#include "logic/world/world.h"
#include "map_io/world_legacy_lookup_table.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapResourcesPacket::read
	(FileSystem & fs, EditorGameBase & egbase, const WorldLegacyLookupTable& lookup_table)
{
	FileRead fr;
	fr.open(fs, "binary/resource");

	Map   & map   = egbase.map();
	const World & world = egbase.world();

	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			int32_t const nr_res = fr.unsigned_16();
			if (world.get_nr_resources() < nr_res)
				log
					("WARNING: Number of resources in map (%i) is bigger than in world "
					 "(%i)",
					 nr_res, world.get_nr_resources());

			// construct ids and map
			std::map<uint8_t, uint8_t> smap;
			for (uint8_t i = 0; i < nr_res; ++i) {
				uint8_t const id = fr.unsigned_16();
				const std::string resource_name = lookup_table.lookup_resource(fr.c_string());
				int32_t const res = world.get_resource(resource_name.c_str());
				if (res == Widelands::INVALID_INDEX)
					throw GameDataError
						("resource '%s' exists in map but not in world", resource_name.c_str());
				smap[id] = res;
			}

			for (uint16_t y = 0; y < map.get_height(); ++y) {
				for (uint16_t x = 0; x < map.get_width(); ++x) {
					uint8_t const id           = fr.unsigned_8();
					uint8_t const found_amount = fr.unsigned_8();
					uint8_t const amount       = found_amount;
					uint8_t const start_amount = fr.unsigned_8();

					uint8_t set_id, set_amount, set_start_amount;
					//  if amount is zero, theres nothing here
					if (!amount) {
						set_id           = 0;
						set_amount       = 0;
						set_start_amount = 0;
					} else {
						set_id           = smap[id];
						set_amount       = amount;
						set_start_amount = start_amount;
					}

					if (0xa < set_id)
						throw "Unknown resource in map file. It is not in world!\n";
					map[Coords(x, y)].set_resources(set_id, set_amount);
					map[Coords(x, y)].set_initial_res_amount(set_start_amount);
				}
			}
		} else {
			throw UnhandledVersionError(packet_version, kCurrentPacketVersion);
		}
	} catch (const WException & e) {
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
void MapResourcesPacket::write
	(FileSystem & fs, EditorGameBase & egbase)
{
	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	// This is a bit more complicated saved so that the order of loading
	// of the resources at run time doesn't matter.
	// (saved like terrains)
	// Write the number of resources
	const Map   & map   = egbase.map  ();
	const World& world = egbase.world();
	uint8_t const nr_res = world.get_nr_resources();
	fw.unsigned_16(nr_res);

	//  write all resources names and their id's
	for (int32_t i = 0; i < nr_res; ++i) {
		const ResourceDescription & res = *world.get_resource(i);
		fw.unsigned_16(i);
		fw.c_string(res.name().c_str());
	}

	//  Now, all resouces as uint8_ts in order
	//  - resource id
	//  - amount
	for (uint16_t y = 0; y < map.get_height(); ++y) {
		for (uint16_t x = 0; x < map.get_width(); ++x) {
			const Field & f = map[Coords(x, y)];
			int32_t       res          = f.get_resources          ();
			int32_t const       amount = f.get_resources_amount   ();
			int32_t const start_amount = f.get_initial_res_amount();
			if (!amount)
				res = 0;
			fw.unsigned_8(res);
			fw.unsigned_8(amount);
			fw.unsigned_8(start_amount);
		}
	}

	fw.write(fs, "binary/resource");
}

}
