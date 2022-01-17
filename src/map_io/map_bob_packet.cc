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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "map_io/map_bob_packet.h"

#include "io/fileread.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/critter.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapBobPacket::read_bob(FileRead& fr,
                            EditorGameBase& egbase,
                            MapObjectLoader&,
                            const Coords& coords,
                            uint16_t /* packet_version */) {
	const std::string owner = fr.c_string();
	const std::string read_name = fr.c_string();
	uint8_t subtype = fr.unsigned_8();
	constexpr uint8_t kLegacyCritterType = 0;
	Serial const serial = fr.unsigned_32();

	if (subtype != kLegacyCritterType || owner != "world") {
		throw GameDataError("unknown legacy bob %s/%s", owner.c_str(), read_name.c_str());
	}

	try {
		Descriptions* descriptions = egbase.mutable_descriptions();
		const CritterDescr& descr =
		   *descriptions->get_critter_descr(descriptions->load_critter(read_name));
		descr.create(egbase, nullptr, coords);
		// We do not register this object as needing loading. This packet is only
		// in fresh maps, that are just started. As soon as the game saves
		// itself, it will write a map_objects packet instead of binary/bob that
		// properly saves all state. Critters when create()ed have a valid state
		// already (starting to roam), so we do not need to load anything
		// further.
	} catch (const WException& e) {
		throw GameDataError("%u (owner = \"%s\", name = \"%s\"): %s", serial, owner.c_str(),
		                    read_name.c_str(), e.what());
	}
}

void MapBobPacket::read(FileSystem& fs, EditorGameBase& egbase, MapObjectLoader& mol) {
	FileRead fr;
	fr.open(fs, "binary/bob");

	Map* map = egbase.mutable_map();
	map->recalc_whole_map(egbase);  //  for movecaps checks in ReadBob
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			for (uint16_t y = 0; y < map->get_height(); ++y) {
				for (uint16_t x = 0; x < map->get_width(); ++x) {
					uint32_t const nr_bobs = fr.unsigned_32();
					for (uint32_t i = 0; i < nr_bobs; ++i) {
						read_bob(fr, egbase, mol, Coords(x, y), packet_version);
					}
				}
			}
		} else {
			throw UnhandledVersionError("MapBobPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("bobs: %s", e.what());
	}
}
}  // namespace Widelands
