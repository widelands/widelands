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

#include "map_io/map_terrain_packet.h"

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/world/terrain_description.h"

namespace Widelands {

constexpr uint16_t kCurrentPacketVersion = 1;

void MapTerrainPacket::read(FileSystem& fs, EditorGameBase& egbase) {
	FileRead fr;
	fr.open(fs, "binary/terrain");

	const Map& map = egbase.map();
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			uint16_t const nr_terrains = fr.unsigned_16();

			using TerrainIdMap = std::map<const uint16_t, DescriptionIndex>;
			TerrainIdMap smap;
			for (uint16_t i = 0; i < nr_terrains; ++i) {
				const uint16_t id = fr.unsigned_16();
				TerrainIdMap::const_iterator const it = smap.find(id);
				if (it != smap.end()) {
					throw GameDataError(
					   "MapTerrainPacket::read: WARNING: Found duplicate terrain id %i.", id);
				}
				smap[id] = egbase.mutable_descriptions()->load_terrain(fr.c_string());
			}

			MapIndex const max_index = map.max_index();
			for (MapIndex i = 0; i < max_index; ++i) {
				Field& f = map[i];
				f.set_terrain_r(smap[fr.unsigned_8()]);
				f.set_terrain_d(smap[fr.unsigned_8()]);
			}
		} else {
			throw UnhandledVersionError("MapTerrainPacket", packet_version, kCurrentPacketVersion);
		}
	} catch (const WException& e) {
		throw GameDataError("terrain: %s", e.what());
	}
}

void MapTerrainPacket::write(FileSystem& fs, EditorGameBase& egbase) {

	FileWrite fw;

	fw.unsigned_16(kCurrentPacketVersion);

	//  This is a bit more complicated saved so that the order of loading of the
	//  terrains at run time does not matter. This is slow like hell.
	const Map& map = egbase.map();
	const Descriptions& descriptions = egbase.descriptions();
	DescriptionIndex const nr_terrains = descriptions.terrains().size();
	fw.unsigned_16(nr_terrains);

	std::map<const char* const, DescriptionIndex> smap;
	for (DescriptionIndex i = 0; i < nr_terrains; ++i) {
		const char* const name = descriptions.get_terrain_descr(i)->name().c_str();
		smap[name] = i;
		fw.unsigned_16(i);
		fw.c_string(name);
	}

	MapIndex const max_index = map.max_index();
	for (MapIndex i = 0; i < max_index; ++i) {
		Field& f = map[i];
		fw.unsigned_8(smap[descriptions.get_terrain_descr(f.terrain_r())->name().c_str()]);
		fw.unsigned_8(smap[descriptions.get_terrain_descr(f.terrain_d())->name().c_str()]);
	}

	fw.write(fs, "binary/terrain");
}
}  // namespace Widelands
