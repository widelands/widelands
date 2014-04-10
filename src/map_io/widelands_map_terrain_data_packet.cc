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

#include "map_io/widelands_map_terrain_data_packet.h"

#include <map>

#include "log.h"
#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "logic/world/terrain_description.h"
#include "logic/world/world.h"
#include "map_io/one_world_legacy_lookup_table.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1

void Map_Terrain_Data_Packet::Read(FileSystem& fs,
                                   Editor_Game_Base& egbase,
                                   const OneWorldLegacyLookupTable& lookup_table) {
	FileRead fr;
	fr.Open(fs, "binary/terrain");

	Map & map = egbase.map();
	const World & world = egbase.world();

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			uint16_t const nr_terrains = fr.Unsigned16();

			typedef std::map<const uint16_t, Terrain_Index> terrain_id_map;
			terrain_id_map smap;
			for (uint16_t i = 0; i < nr_terrains; ++i) {
				const uint16_t id = fr.Unsigned16();
				char const* const old_terrain_name = fr.CString();
				terrain_id_map::const_iterator const it = smap.find(id);
				if (it != smap.end()) {
					throw game_data_error(
					   "Map_Terrain_Data_Packet::Read: WARNING: Found duplicate terrain id %i.", id);
				}
				const std::string new_terrain_name =
				   lookup_table.lookup_terrain(old_terrain_name);
				if (!world.get_ter(new_terrain_name.c_str())) {
					throw game_data_error("Terrain '%s' exists in map, not in world!", new_terrain_name.c_str());
				}
				smap[id] = world.index_of_terrain(new_terrain_name.c_str());
			}

			Map_Index const max_index = map.max_index();
			for (Map_Index i = 0; i < max_index; ++i) {
				Field & f = map[i];
				f.set_terrain_r(smap[fr.Unsigned8()]);
				f.set_terrain_d(smap[fr.Unsigned8()]);
			}
		} else {
			throw game_data_error("unknown/unhandled version %u", packet_version);
		}
	} catch (const _wexception & e) {
		throw game_data_error("terrain: %s", e.what());
	}
}


void Map_Terrain_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase)
{

	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  This is a bit more complicated saved so that the order of loading of the
	//  terrains at run time does not matter. This is slow like hell.
	const Map & map = egbase.map();
	const World & world = egbase.world();
	Terrain_Index const nr_terrains = world.get_nr_terrains();
	fw.Unsigned16(nr_terrains);

	std::map<const char * const, Terrain_Index> smap;
	for (Terrain_Index i = 0; i < nr_terrains; ++i) {
		const char * const name = world.terrain_descr(i).name().c_str();
		smap[name] = i;
		fw.Unsigned16(i);
		fw.CString(name);
	}

	Map_Index const max_index = map.max_index();
	for (Map_Index i = 0; i < max_index; ++i) {
		Field & f = map[i];
		fw.Unsigned8(smap[world.terrain_descr(f.terrain_r()).name().c_str()]);
		fw.Unsigned8(smap[world.terrain_descr(f.terrain_d()).name().c_str()]);
	}

	fw.Write(fs, "binary/terrain");
}

}
