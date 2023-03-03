/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

// We need to keep compatibility with older maps around indefinitely.
constexpr uint16_t kCurrentPacketVersion = 2;

void MapTerrainPacket::read(FileSystem& fs, EditorGameBase& egbase) {
	FileRead fr;
	fr.open(fs, "binary/terrain");

	const Map& map = egbase.map();
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == 1) {
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
		} else if (packet_version == kCurrentPacketVersion) {
			std::map<DescriptionIndex /* index in binary */, DescriptionIndex /* actual index */>
			   mappings;
			MapIndex const max_index = map.max_index();
			for (MapIndex i = 0; i < max_index; ++i) {
				Field& f = map[i];

				uint16_t saved_index = fr.unsigned_16();
				auto lookup = mappings.find(saved_index);
				if (lookup == mappings.end()) {
					lookup =
					   mappings
					      .emplace(saved_index, egbase.mutable_descriptions()->load_terrain(fr.string()))
					      .first;
				}
				f.set_terrain_r(lookup->second);

				saved_index = fr.unsigned_16();
				lookup = mappings.find(saved_index);
				if (lookup == mappings.end()) {
					lookup =
					   mappings
					      .emplace(saved_index, egbase.mutable_descriptions()->load_terrain(fr.string()))
					      .first;
				}
				f.set_terrain_d(lookup->second);
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

	const Map& map = egbase.map();
	std::set<DescriptionIndex> written_terrains;
	const MapIndex max_index = map.max_index();

	for (MapIndex i = 0; i < max_index; ++i) {
		const Field& f = map[i];
		const DescriptionIndex tr = f.terrain_r();
		const DescriptionIndex td = f.terrain_d();

		fw.unsigned_16(tr);
		if (written_terrains.count(tr) == 0) {
			written_terrains.insert(tr);
			fw.string(egbase.descriptions().get_terrain_descr(tr)->name());
		}

		fw.unsigned_16(td);
		if (written_terrains.count(td) == 0) {
			written_terrains.insert(td);
			fw.string(egbase.descriptions().get_terrain_descr(td)->name());
		}
	}

	fw.write(fs, "binary/terrain");
}
}  // namespace Widelands
