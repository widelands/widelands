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

#include "map_generator.h"
#include "map.h"
#include "logic/editor_game_base.h"

#define MAX_ELEVATION      (0xffffffff)
#define MAX_ELEVATION_3_4  (0xc0000000)
#define MAX_ELEVATION_HALF (0x80000000)

namespace Widelands
{

MapGenerator::MapGenerator
	(Map & map, UniqueRandomMapInfo const & mapInfo,
	 Editor_Game_Base & egbase)
	: m_map(map), m_mapInfo(mapInfo), m_egbase(egbase)
{

}

void MapGenerator::generate_bobs
	(MapGenInfo & mapGenInfo,
	 uint32_t          const * const * const random_bobs,
	 Coords const fc,
	 RNG  &       rng,
	 MapGenAreaInfo::MapGenTerrainType const terrType)
{
	// Figure out wich bob area is due here...

	size_t num = mapGenInfo.getNumBobAreas();
	size_t found = num;
	uint32_t sum_weight = mapGenInfo.getSumBobAreaWeight();
	uint32_t max_val = 0;
	for (size_t ix = 0; ix < num; ++ix) {
		uint32_t val = random_bobs[ix][fc.x + m_mapInfo.w * fc.y];
		val = (val / sum_weight) * mapGenInfo.getBobArea(ix).getWeight();
		if (val >= max_val) {
			found = ix;
			max_val = val;
		}
	}
	if (found >= num)
		return;

	// Figure out if we really need to set a bob here...

	MapGenBobArea const & bobArea = mapGenInfo.getBobArea(found);

	const MapGenBobKind * bobKind = bobArea.getBobKind(terrType);

	if (not bobKind) //  no bobs defined here...
		return;

	uint32_t immovDens = bobArea.getImmovableDensity();
	uint32_t movDens   = bobArea.getMoveableDensity();

	immovDens *= max_val / 100;
	movDens   *= max_val / 100;

	immovDens = immovDens >= MAX_ELEVATION_HALF ? MAX_ELEVATION : immovDens * 2;
	movDens   = movDens   >= MAX_ELEVATION_HALF ? MAX_ELEVATION : movDens   * 2;

	uint32_t val = rng.rand();
	bool set_immovable = (val <= immovDens);
	val = rng.rand();
	bool set_moveable  = (val <= movDens);


	// Set bob according to bob area

	if (set_immovable and (num = bobKind->getNumImmovableBobs()))
		m_egbase.create_immovable
			(fc,
			 bobKind->getImmovableBob
			 	(static_cast<size_t>(rng.rand() / (MAX_ELEVATION / num))),
			 0);

	if (set_moveable and (num = bobKind->getNumMoveableBobs()))
		m_egbase.create_bob
			(fc,
			 m_map.world().get_bob
			 	(bobKind->getMoveableBob
			 	 	(static_cast<size_t>(rng.rand() / (MAX_ELEVATION / num)))
			 	 .c_str()),
			 0);
}

};

// TODO: Also take mountain and water areas into bob generation
// (we have ducks and chamois)
// TODO: Move other map generation functions from Map to MapGenerator
// TODO: Define the "none"-bob to weigh other bobs lower within BobKinds...
// TODO: Clean up code
// TODO: Improve mapgenconf files for nicer generated worlds
