/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_LOGIC_TERRAIN_AFFINITY_H
#define WL_LOGIC_TERRAIN_AFFINITY_H

#include <string>

#include "base/macros.h"
#include "logic/description_maintainer.h"

class LuaTable;

namespace Widelands {

class Map;
class TerrainDescription;
class World;
struct FCoords;

// Describes the parameters and the pickiness of Immovables towards terrain
// parameters. Alls immovables that use 'grow' in any of their programs must
// define this.
class TerrainAffinity {
public:
	explicit TerrainAffinity(const LuaTable& table, const std::string& immovable_name);

	// Preferred temperature is in arbitrary units.
	double preferred_temperature() const;

	// Preferred fertility in percent [0, 1].
	double preferred_fertility() const;

	// Preferred humidity in percent [0, 1].
	double preferred_humidity() const;

	// A value in [0, 1] that defines how well this can deal with non-ideal
	// situations. Lower means it is less picky, i.e. it can deal better.
	double pickiness() const;

private:
	double preferred_fertility_;
	double preferred_humidity_;
	double preferred_temperature_;
	double pickiness_;

	DISALLOW_COPY_AND_ASSIGN(TerrainAffinity);
};

// Returns a value in [0., 1.] that describes the suitability for the
// 'immovable_affinity' for 'field'. Higher is better suited.
double probability_to_grow
	(const TerrainAffinity& immovable_affinity, const FCoords& fcoords,
	 const Map& map, const DescriptionMaintainer<TerrainDescription>& terrains);

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TERRAIN_AFFINITY_H
