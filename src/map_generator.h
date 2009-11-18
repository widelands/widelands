/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef MAP_GENERATOR_H_
#define MAP_GENERATOR_H_

#include "world.h"

// This is the first step of separating map generation from
// map.
// TODO: Put other generation stuff here too...

struct RNG;

namespace Widelands {

struct Map;
struct Editor_Game_Base;
struct UniqueRandomMapInfo;

struct MapGenerator {

	MapGenerator
		(Map & map, UniqueRandomMapInfo const & mapInfo,
		 Editor_Game_Base & egbase);

	void generate_bobs
		(MapGenInfo                      &,
		 uint32_t          const * const * random_bobs,
		 Coords,
		 RNG                             &,
		 MapGenAreaInfo::MapGenTerrainType terrType);

private:

	Map &                        m_map;
	UniqueRandomMapInfo const & m_mapInfo;
	Editor_Game_Base &           m_egbase;
};

}

#endif /* MAP_GENERATOR_H_ */
