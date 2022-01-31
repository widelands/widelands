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

#include "logic/map_objects/terrain_affinity.h"

#include "logic/field.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/widelands_geometry.h"
#include "scripting/lua_table.h"

namespace Widelands {

namespace {

// Literature on cross-platform floating point precision-problems:
// https://arxiv.org/abs/cs/0701192
// Monniaux, David (2008): "The pitfalls of verifying floating-point computations",
// in: ACM Transactions on Programming Languages and Systems 30, 3 (2008) 12.
//
// Recommends using heximal float constants, but we'd need to switch to C++17 for that.
//
// https://randomascii.wordpress.com/2012/03/21/intermediate-floating-point-precision/

constexpr double square(const double& a) {
	return a * a;
}

// Helper function for probability_to_grow
// Calculates the probability to grow for the given affinity and terrain values
inline unsigned int calculate_probability_to_grow(const TerrainAffinity& affinity,
                                                  int terrain_humidity,
                                                  int terrain_fertility,
                                                  int terrain_temperature) {
	constexpr double kHumidityWeight = 5.00086642549548;
	constexpr double kFertilityWeight = 5.292268046607387;
	constexpr double kTemperatureWeight = 0.6131300863608306;

	// Avoid division by 0
	assert(affinity.pickiness() < 100);
	const double sigma = std::floor(100.0 - affinity.pickiness());

	// Unlike real numbers, floating point multiplication/division is neither associative nor
	// commutative. Fortunately, execution order is well-defined by the C++ standard.
	const double result = exp(
	   -(square((affinity.preferred_fertility() - terrain_fertility) / (kFertilityWeight * sigma)) +
	     square((affinity.preferred_humidity() - terrain_humidity) / (kHumidityWeight * sigma)) +
	     square((affinity.preferred_temperature() - terrain_temperature) /
	            (kTemperatureWeight * sigma))) /
	   2.0);

	return static_cast<unsigned int>(
	   std::max(0.0, std::floor(result * static_cast<double>(TerrainAffinity::kPrecisionFactor))));
}

}  // namespace

TerrainAffinity::TerrainAffinity(const LuaTable& table, const std::string& immovable_name)
   : preferred_fertility_(table.get_int("preferred_fertility")),
     preferred_humidity_(table.get_int("preferred_humidity")),
     preferred_temperature_(table.get_int("preferred_temperature")),
     pickiness_(table.get_int("pickiness")) {
	if (!(0 <= preferred_fertility_ && preferred_fertility_ <= 1000)) {
		throw GameDataError("%s: preferred_fertility is not in [0, 1000].", immovable_name.c_str());
	}
	if (!(0 <= preferred_humidity_ && preferred_humidity_ <= 1000)) {
		throw GameDataError("%s: preferred_humidity is not in [0, 1000].", immovable_name.c_str());
	}
	if (!(0 <= pickiness_ && pickiness_ < 100)) {
		throw GameDataError("%s: pickiness is not in [0, 99].", immovable_name.c_str());
	}
	if (preferred_temperature_ < 0) {
		throw GameDataError("%s: preferred_temperature is not possible.", immovable_name.c_str());
	}
}

int TerrainAffinity::preferred_temperature() const {
	return preferred_temperature_;
}

int TerrainAffinity::preferred_fertility() const {
	return preferred_fertility_;
}

int TerrainAffinity::preferred_humidity() const {
	return preferred_humidity_;
}

int TerrainAffinity::pickiness() const {
	return pickiness_;
}

unsigned int probability_to_grow(const TerrainAffinity& affinity,
                                 const FCoords& fcoords,
                                 const Map& map,
                                 const DescriptionMaintainer<TerrainDescription>& terrains) {
	// Initialize with 3 to get proper rounding with the integer division in the return statement
	int terrain_humidity = 3;
	int terrain_fertility = 3;
	int terrain_temperature = 3;

	const auto sum_up_values = [&terrain_humidity, &terrain_fertility, &terrain_temperature,
	                            &terrains](const int terrain_index) {
		const TerrainDescription& t = terrains.get(terrain_index);
		terrain_humidity += t.humidity();
		terrain_temperature += t.temperature();
		terrain_fertility += t.fertility();
	};

	sum_up_values(fcoords.field->terrain_d());
	sum_up_values(fcoords.field->terrain_r());
	{
		FCoords tln;
		map.get_tln(fcoords, &tln);
		sum_up_values(tln.field->terrain_d());
		sum_up_values(tln.field->terrain_r());
	}

	{
		FCoords trn;
		map.get_trn(fcoords, &trn);
		sum_up_values(trn.field->terrain_d());
	}

	{
		FCoords ln;
		map.get_ln(fcoords, &ln);
		sum_up_values(ln.field->terrain_r());
	}

	return calculate_probability_to_grow(
	   affinity, terrain_humidity / 6, terrain_fertility / 6, terrain_temperature / 6);
}

unsigned int probability_to_grow(const TerrainAffinity& affinity,
                                 const TerrainDescription& terrain) {

	return calculate_probability_to_grow(
	   affinity, terrain.humidity(), terrain.fertility(), terrain.temperature());
}

}  // namespace Widelands
