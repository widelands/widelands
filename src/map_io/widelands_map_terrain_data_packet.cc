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
#include "logic/world.h"

namespace Widelands {

#define CURRENT_PACKET_VERSION 1


void Map_Terrain_Data_Packet::Read
	(FileSystem & fs, Editor_Game_Base & egbase, bool, Map_Map_Object_Loader &)
{
	FileRead fr;
	fr.Open(fs, "binary/terrain");

	Map & map = egbase.map();
	World & world = map.world();

	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == CURRENT_PACKET_VERSION) {
			uint16_t const nr_terrains = fr.Unsigned16();

			typedef std::map<const uint16_t, Terrain_Index> terrain_id_map;
			terrain_id_map smap;
			for (uint16_t i = 0; i < nr_terrains; ++i) {
				uint16_t                       const id   = fr.Unsigned16();
				char                   const * const name = fr.CString   ();
				terrain_id_map::const_iterator const it   = smap.find(id);
				if (it != smap.end())
					log
						("Map_Terrain_Data_Packet::Read: WARNING: Found duplicate "
						 "terrain id %i: Previously defined as \"%s\", now as "
						 "\"%s\".",
						 id, world.terrain_descr(it->second).name().c_str(), name);
				if (not world.get_ter(name))
					throw game_data_error
						("Terrain '%s' exists in map, not in world!", name);
				smap[id] = world.index_of_terrain(name);
			}

			Map_Index const max_index = map.max_index();
			for (Map_Index i = 0; i < max_index; ++i) {
				Field & f = map[i];
				f.set_terrain_r(smap[fr.Unsigned8()]);
				f.set_terrain_d(smap[fr.Unsigned8()]);
			}
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("terrain: %s", e.what());
	}
}


void Map_Terrain_Data_Packet::Write
	(FileSystem & fs, Editor_Game_Base & egbase, Map_Map_Object_Saver &)
{

	FileWrite fw;

	fw.Unsigned16(CURRENT_PACKET_VERSION);

	//  This is a bit more complicated saved so that the order of loading of the
	//  terrains at run time does not matter. This is slow like hell.
	const Map & map = egbase.map();
	const World & world = map.world();
	Terrain_Index const nr_terrains = world.get_nr_terrains();
	fw.Unsigned16(nr_terrains);

	std::map<const char * const, Terrain_Index> smap;
	for (Terrain_Index i = 0; i < nr_terrains; ++i) {
		const char * const name = world.get_ter(i).name().c_str();
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
