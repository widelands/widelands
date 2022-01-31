/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TERRAIN_AFFINITY_H
#define WL_LOGIC_MAP_OBJECTS_TERRAIN_AFFINITY_H

#include "base/macros.h"
#include "logic/map_objects/description_maintainer.h"

class LuaTable;

namespace Widelands {

class Map;
class TerrainDescription;
struct FCoords;

// Describes the parameters and the pickiness of Immovables towards terrain
// parameters. Alls immovables that use 'grow' in any of their programs must
// define this.
class TerrainAffinity {
public:
	static constexpr int kPrecisionFactor = 1 << 26;

	explicit TerrainAffinity(const LuaTable& table, const std::string& immovable_name);

	// Preferred temperature is in arbitrary units.
	int preferred_temperature() const;

	// Preferred fertility, ranging from 0 to 1000.
	int preferred_fertility() const;

	// Preferred humidity, ranging from 0 to 1000.
	int preferred_humidity() const;

	// A value in [0, 99] that defines how well this can deal with non-ideal
	// situations. Lower means it is less picky, i.e. it can deal better.
	int pickiness() const;

private:
	const int preferred_fertility_;
	const int preferred_humidity_;
	const int preferred_temperature_;
	const int pickiness_;

	DISALLOW_COPY_AND_ASSIGN(TerrainAffinity);
};

/**
 * Returns a value in [0, TerrainAffinity::kPrecisionFactor] that describes the suitability for the
 * 'immovable_affinity' for all 6 terrains around 'field'.
 * Higher is better suited, with TerrainAffinity::kPrecisionFactor representing a probability of 1.
 * */
unsigned int probability_to_grow(const TerrainAffinity& immovable_affinity,
                                 const FCoords& fcoords,
                                 const Map& map,
                                 const DescriptionMaintainer<TerrainDescription>& terrains);

/**
 * Returns a value in [0, TerrainAffinity::kPrecisionFactor] that describes the suitability for the
 * 'immovable_affinity' for a single 'terrain'.
 * Higher is better suited, with TerrainAffinity::kPrecisionFactor representing a probability of 1.
 * */
unsigned int probability_to_grow(const TerrainAffinity& immovable_affinity,
                                 const TerrainDescription& terrain);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TERRAIN_AFFINITY_H
