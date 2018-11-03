/*
 * Copyright (C) 2006-2018 by the Widelands Development Team
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

#include "logic/map_objects/terrain_affinity.h"

#include <vector>

#include "logic/description_maintainer.h"
#include "logic/field.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/widelands_geometry.h"
#include "scripting/lua_table.h"

namespace Widelands {

namespace {

constexpr double pow2(const double& a) {
	return static_cast<double>(a) * static_cast<double>(a);
}

// Helper function for probability_to_grow
// Calculates the probability to grow for the given affinity and terrain values
double calculate_probability_to_grow(const TerrainAffinity& affinity,
                                     int terrain_humidity,
                                     int terrain_fertility,
                                     int terrain_temperature) {
	constexpr double kHumidityWeight = 5.00086642549548;
	constexpr double kFertilityWeight = 5.292268046607387;
	constexpr double kTemperatureWeight = 0.6131300863608306;

    // Lots of static_cast<double> to force double precision for all compilers to prevent desyncs here
    // See https://randomascii.wordpress.com/2012/03/21/intermediate-floating-point-precision/
	const double sigma = 100.0 - affinity.pickiness();

    const double fertility_value = static_cast<double>(affinity.preferred_fertility() - terrain_fertility) /
            static_cast<double>(static_cast<double>(kFertilityWeight) * static_cast<double>(sigma));
    const double humidity_value = static_cast<double>(affinity.preferred_humidity() - terrain_humidity) /
            static_cast<double>(static_cast<double>(kHumidityWeight) * static_cast<double>(sigma));
    const double temperature_value = static_cast<double>(affinity.preferred_temperature() - terrain_temperature) /
            static_cast<double>(static_cast<double>(kTemperatureWeight) * static_cast<double>(sigma));

	return exp(-(pow2(fertility_value) + pow2(humidity_value) + pow2(temperature_value)) / 2.0);
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
	if (!(0 <= pickiness_ && pickiness_ <= 100)) {
		throw GameDataError("%s: pickiness is not in [0, 100].", immovable_name.c_str());
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

double probability_to_grow(const TerrainAffinity& affinity,
                           const FCoords& fcoords,
                           const Map& map,
                           const DescriptionMaintainer<TerrainDescription>& terrains) {
	int terrain_humidity = 0;
	int terrain_fertility = 0;
	int terrain_temperature = 0;

	const auto average = [&terrain_humidity, &terrain_fertility, &terrain_temperature,
	                      &terrains](const int terrain_index) {
		const TerrainDescription& t = terrains.get(terrain_index);
		terrain_humidity += t.humidity() / 6;
		terrain_temperature += t.temperature() / 6;
		terrain_fertility += t.fertility() / 6;
	};

	average(fcoords.field->terrain_d());
	average(fcoords.field->terrain_r());
	{
		FCoords tln;
		map.get_tln(fcoords, &tln);
		average(tln.field->terrain_d());
		average(tln.field->terrain_r());
	}

	{
		FCoords trn;
		map.get_trn(fcoords, &trn);
		average(trn.field->terrain_d());
	}

	{
		FCoords ln;
		map.get_ln(fcoords, &ln);
		average(ln.field->terrain_r());
	}

	return calculate_probability_to_grow(
	   affinity, terrain_humidity, terrain_fertility, terrain_temperature);
}

double probability_to_grow(const TerrainAffinity& affinity, const TerrainDescription& terrain) {

	return calculate_probability_to_grow(
	   affinity, terrain.humidity(), terrain.fertility(), terrain.temperature());
}

}  // namespace Widelands
