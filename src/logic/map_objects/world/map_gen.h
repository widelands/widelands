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

#ifndef WL_LOGIC_MAP_OBJECTS_WORLD_MAP_GEN_H
#define WL_LOGIC_MAP_OBJECTS_WORLD_MAP_GEN_H

#include <map>

#include "logic/map_objects/world/terrain_description.h"

namespace Widelands {

struct MapGenInfo;

/// Holds world and area specific information for the map generator.
/// Areas are: Water, Land, Wasteland and Mountains.
struct MapGenAreaInfo {
	enum class Area { kWater, kLand, kWasteland, kMountains };

	enum class Terrain {
		kWaterOcean,
		kWaterShelf,
		kWaterShallow,

		kLandCoast,
		kLandLand,
		kLandUpper,

		kWastelandInner,
		kWastelandOuter,

		kMountainsFoot,
		kMountainsMountain,
		kMountainsSnow
	};

	MapGenAreaInfo(const LuaTable& table, const Descriptions& descriptions, Area area_type);

	size_t get_num_terrains(Terrain) const;
	DescriptionIndex get_terrain(Terrain terrain_type, uint32_t index) const;
	uint32_t get_weight() const {
		return weight_;
	}

private:
	std::vector<DescriptionIndex> terrains1_;  //  ocean, coast, inner or foot
	std::vector<DescriptionIndex> terrains2_;  //  shelf, land, outer or mountain
	std::vector<DescriptionIndex> terrains3_;  //  shallow, upper, snow

	uint32_t weight_;
};

struct MapGenBobCategory {
	explicit MapGenBobCategory(const LuaTable& table);

	size_t num_immovables() const {
		return immovables_.size();
	}
	size_t num_critters() const {
		return critters_.size();
	}

	const std::string& get_immovable(size_t index) const {
		return immovables_[index];
	}
	const std::string& get_critter(size_t index) const {
		return critters_[index];
	}

private:
	std::vector<std::string> immovables_;
	std::vector<std::string> critters_;
};

struct MapGenLandResource {
	MapGenLandResource(const LuaTable& table, MapGenInfo& map_gen_info);

	uint32_t get_weight() const {
		return weight_;
	}
	const MapGenBobCategory* get_bob_category(MapGenAreaInfo::Terrain terrain_type) const;

	uint8_t get_immovable_density() const {
		return immovable_density_;
	}
	uint8_t get_moveable_density() const {
		return critter_density_;
	}

private:
	uint32_t weight_;
	uint8_t immovable_density_;  // In percent
	uint8_t critter_density_;    // In percent
	const MapGenBobCategory* land_coast_bob_category_;
	const MapGenBobCategory* land_inner_bob_category_;
	const MapGenBobCategory* land_upper_bob_category_;
	const MapGenBobCategory* wasteland_inner_bob_category_;
	const MapGenBobCategory* wasteland_outer_bob_category_;
};

/** struct MapGenInfo
 *
 * This class holds world specific information for the map generator.
 * This info is usually read from the file "mapgeninfo" of a world.
 */
struct MapGenInfo {
	MapGenInfo(const LuaTable& table, const Descriptions& descriptions);

	size_t get_num_areas(MapGenAreaInfo::Area area_type) const;
	const MapGenAreaInfo& get_area(MapGenAreaInfo::Area area_type, uint32_t index) const;
	const MapGenBobCategory* get_bob_category(const std::string& bob_category) const;

	uint8_t get_water_ocean_height() const {
		return ocean_height_;
	}
	uint8_t get_water_shelf_height() const {
		return shelf_height_;
	}
	uint8_t get_water_shallow_height() const {
		return shallow_height_;
	}
	uint8_t get_land_coast_height() const {
		return coast_height_;
	}
	uint8_t get_land_upper_height() const {
		return upperland_height_;
	}
	uint8_t get_mountain_foot_height() const {
		return mountainfoot_height_;
	}
	uint8_t get_mountain_height() const {
		return mountain_height_;
	}
	uint8_t get_snow_height() const {
		return snow_height_;
	}
	uint8_t get_summit_height() const {
		return summit_height_;
	}

	uint32_t get_sum_land_weight() const;

	size_t get_num_land_resources() const;
	const MapGenLandResource& get_land_resource(size_t index) const;
	uint32_t get_sum_land_resource_weight() const;

private:
	std::vector<MapGenAreaInfo> water_areas_;
	std::vector<MapGenAreaInfo> land_areas_;
	std::vector<MapGenAreaInfo> wasteland_areas_;
	std::vector<MapGenAreaInfo> mountain_areas_;

	std::vector<MapGenLandResource> land_resources_;
	std::map<std::string, MapGenBobCategory> bob_categories_;

	uint8_t ocean_height_;
	uint8_t shelf_height_;
	uint8_t shallow_height_;
	uint8_t coast_height_;
	uint8_t upperland_height_;
	uint8_t mountainfoot_height_;
	uint8_t snow_height_;
	uint8_t mountain_height_;
	uint8_t summit_height_;

	mutable int32_t land_weight_;
	mutable bool land_weight_valid_;

	mutable int32_t sum_bob_area_weights_;
	mutable bool sum_bob_area_weights_valid_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_WORLD_MAP_GEN_H
