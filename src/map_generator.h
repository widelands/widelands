/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef MAP_GENERATOR_H
#define MAP_GENERATOR_H

#include "logic/world.h"

// This is the first step of separating map generation from
// map.
// TODO: Put other generation stuff here too...

struct RNG;

namespace Widelands {

class Map;
class Editor_Game_Base;

/**
 * This helper class repesents the complete map initialization
 * info needed by the random map generator
 * It can be converted to and from an "Id-String". The Id-String is a
 * string-encoded version of the random map info. This Id-String can
 * be handelled by the user/player more easily.
 * The Id-String also contains some kind of check-sum to prevent ill
 * ids to be used.
 */
struct UniqueRandomMapInfo {

	enum Resource_Amount
	{
		raLow    = 0,
		raMedium = 1,
		raHigh   = 2
	};

	uint32_t mapNumber;
	uint32_t w;
	uint32_t h;
	Resource_Amount resource_amount;
	std::string worldName;

	double        waterRatio;     //  How much of the map is water?
	double        landRatio;      //  How much of the map is land?
	double        wastelandRatio; //  How much of the "land" is wasteland?
	Player_Number numPlayers;     //  number of player to generate
	bool          islandMode;     //  whether the world will be an island

	//  other stuff
	static bool setFromIdString
		(UniqueRandomMapInfo & mapInfo_out, const std::string & mapIdString,
		 const std::vector<std::string> & worlds);
	static void generateIdString
		(std::string & mapIdsString_out, const UniqueRandomMapInfo & mapInfo);

private:
	static int  mapIdCharToNumber(char);
	static char mapIdNumberToChar(int32_t);
	static uint16_t generateWorldNameHash(const std::string &);
};


struct MapGenerator {

	MapGenerator
		(Map & map, const UniqueRandomMapInfo & mapInfo,
		 Editor_Game_Base & egbase);

	void create_random_map();

private:

	void generate_bobs
		(std::unique_ptr<uint32_t[]> const * random_bobs,
		 Coords,
		 RNG                             &,
		 MapGenAreaInfo::MapGenTerrainType terrType);

	void generate_resources
		(uint32_t            const * const random1,
		 uint32_t            const * const random2,
		 uint32_t            const * const random3,
		 uint32_t            const * const random4,
		 FCoords                     const fc);

	uint8_t make_node_elevation
		(double elevation, Coords);

	static uint32_t * generate_random_value_map
		(uint32_t w, uint32_t h, RNG & rng);

	Terrain_Index figure_out_terrain
		(uint32_t                  * const random2,
		 uint32_t                  * const random3,
		 uint32_t                  * const random4,
		 Coords const c0, Coords const c1, Coords const c2,
		 uint32_t const h1, uint32_t const h2, uint32_t const h3,
		 RNG                       &       rng,
		 MapGenAreaInfo::MapGenTerrainType & terrType);

	Map &                        m_map;
	const UniqueRandomMapInfo & m_mapInfo;
	Editor_Game_Base &           m_egbase;
};

}

#endif
