/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef MAP_GEN_H
#define MAP_GEN_H

#include <map>
#include <string>
#include <vector>

#include "logic/world/terrain_description.h"

class LuaTable;

namespace Widelands {

class World;
struct MapGenInfo;

/// Holds world and area specific information for the map generator.
/// Areas are: Water, Land, Wasteland and Mountains.
struct MapGenAreaInfo {
	enum MapGenAreaType {
		atWater,
		atLand,
		atWasteland,
		atMountains
	};

	enum MapGenTerrainType {
		ttWaterOcean,
		ttWaterShelf,
		ttWaterShallow,

		ttLandCoast,
		ttLandLand,
		ttLandUpper,

		ttWastelandInner,
		ttWastelandOuter,

		ttMountainsFoot,
		ttMountainsMountain,
		ttMountainsSnow
	};

	MapGenAreaInfo(const LuaTable& table, const World& world, MapGenAreaType areaType);

	size_t getNumTerrains(MapGenTerrainType) const;
	Terrain_Index getTerrain(MapGenTerrainType terrType, uint32_t index) const;
	uint32_t getWeight() const {return weight_;}

private:
	std::vector<Terrain_Index>  terrains1_; //  ocean, coast, inner or foot
	std::vector<Terrain_Index>  terrains2_; //  shelf, land, outer or mountain
	std::vector<Terrain_Index>  terrains3_; //  shallow, upper, snow

	uint32_t weight_;
	MapGenAreaType areaType_;
};

struct MapGenBobCategory {
	MapGenBobCategory(const LuaTable& table);

	size_t num_immovables() const {return immovables_.size();}
	size_t num_critters() const {return critters_.size();}

	const std::string & get_immovable(size_t index) const {
		return immovables_[index];
	};
	const std::string & get_critter(size_t index) const {
		return critters_[index];
	};

private:
	std::vector<std::string> immovables_;
	std::vector<std::string> critters_;
};

struct MapGenLandResource {
	MapGenLandResource(const LuaTable& table, MapGenInfo& mapGenInfo);

	uint32_t getWeight() const {return weight_;};
	const MapGenBobCategory * getBobCategory
		(MapGenAreaInfo::MapGenTerrainType terrType) const;

	uint8_t getImmovableDensity() const {return immovable_density_;};
	uint8_t getMoveableDensity() const {return critter_density_;};

private:
	uint32_t        weight_;
	uint8_t         immovable_density_; // In percent
	uint8_t         critter_density_;  // In percent
	const MapGenBobCategory * land_coast_bob_category_;
	const MapGenBobCategory * land_inner_bob_category_;
	const MapGenBobCategory * land_upper_bob_category_;
	const MapGenBobCategory * wasteland_inner_bob_category_;
	const MapGenBobCategory * wasteland_outer_bob_category_;
};

/** struct MapGenInfo
  *
  * This class holds world specific information for the map generator.
  * This info is usually read from the file "mapgeninfo" of a world.
  */
struct MapGenInfo {
	MapGenInfo(const LuaTable& table, const World& world);

	size_t getNumAreas(MapGenAreaInfo::MapGenAreaType areaType) const;
	const MapGenAreaInfo & getArea
		(MapGenAreaInfo::MapGenAreaType const areaType, uint32_t const index)
		const;
	const MapGenBobCategory * getBobCategory(const std::string & bobCategory) const;

	uint8_t getWaterOceanHeight  () const {return ocean_height_;}
	uint8_t getWaterShelfHeight  () const {return shelf_height_;}
	uint8_t getWaterShallowHeight() const {return shallow_height_;}
	uint8_t getLandCoastHeight   () const {return coast_height_;}
	uint8_t getLandUpperHeight   () const {return upperland_height_;}
	uint8_t getMountainFootHeight() const {return mountainfoot_height_;}
	uint8_t getMountainHeight    () const {return mountain_height_;}
	uint8_t getSnowHeight        () const {return snow_height_;}
	uint8_t getSummitHeight      () const {return summit_height_;}

	uint32_t getSumLandWeight() const;

	size_t getNumLandResources() const;
	const MapGenLandResource & getLandResource(size_t index) const;
	uint32_t getSumLandResourceWeight() const;

private:
	std::vector<MapGenAreaInfo> water_areas_;
	std::vector<MapGenAreaInfo> land_areas_;
	std::vector<MapGenAreaInfo> wasteland_areas_;
	std::vector<MapGenAreaInfo> mountain_areas_;

	std::vector<MapGenLandResource>           land_resources_;
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

	mutable int32_t  land_weight_;
	mutable bool land_weight_valid_;

	mutable int32_t sum_bob_area_weights_;
	mutable bool sum_bob_area_weights_valid_;

};

}  // namespace Widelands

#endif /* end of include guard: MAP_GEN_H */
