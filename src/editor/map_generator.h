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

#ifndef WL_EDITOR_MAP_GENERATOR_H
#define WL_EDITOR_MAP_GENERATOR_H

#include <memory>

#include "base/random.h"
#include "logic/map_objects/world/map_gen.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

class Map;
class EditorGameBase;

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

	enum ResourceAmount { raLow = 0, raMedium = 1, raHigh = 2 };

	uint32_t mapNumber;
	uint32_t w;
	uint32_t h;
	ResourceAmount resource_amount;
	std::string world_name;

	double waterRatio;        //  How much of the map is water?
	double landRatio;         //  How much of the map is land?
	double wastelandRatio;    //  How much of the "land" is wasteland?
	PlayerNumber numPlayers;  //  number of player to generate
	bool islandMode;          //  whether the world will be an island

	//  other stuff
	static bool set_from_id_string(UniqueRandomMapInfo& mapInfo_out,
	                               const std::string& mapIdString,
	                               const std::vector<std::string>& world_names);
	static void generate_id_string(std::string& mapIdsString_out,
	                               const UniqueRandomMapInfo& mapInfo);

private:
	static int map_id_char_to_number(char);
	static char map_id_number_to_char(int32_t);
	static uint16_t generate_world_name_hash(const std::string&);
};

struct MapGenerator {

	MapGenerator(Map& map, const UniqueRandomMapInfo& mapInfo, EditorGameBase& egbase);

	// Returns true if everything went well and all players have a good starting position
	bool create_random_map();

private:
	void generate_bobs(std::unique_ptr<uint32_t[]> const* random_bobs,
	                   const Coords&,
	                   RNG&,
	                   MapGenAreaInfo::Terrain terrType);

	void generate_resources(uint32_t const* const random1,
	                        uint32_t const* const random2,
	                        uint32_t const* const random3,
	                        uint32_t const* const random4,
	                        const FCoords& fc);

	uint8_t make_node_elevation(double elevation, const Coords&);

	static uint32_t* generate_random_value_map(uint32_t w, uint32_t h, RNG& rng);

	DescriptionIndex figure_out_terrain(const uint32_t* random2,
	                                    const uint32_t* random3,
	                                    const uint32_t* random4,
	                                    const Coords& c0,
	                                    const Coords& c1,
	                                    const Coords& c2,
	                                    uint32_t const h1,
	                                    uint32_t const h2,
	                                    uint32_t const h3,
	                                    RNG& rng,
	                                    MapGenAreaInfo::Terrain& terrType);

	std::unique_ptr<const MapGenInfo> map_gen_info_;
	Map& map_;
	const UniqueRandomMapInfo& map_info_;
	EditorGameBase& egbase_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_EDITOR_MAP_GENERATOR_H
