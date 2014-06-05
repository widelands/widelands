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

// NOCOM(#sirver): remove
class LuaTable;

namespace Widelands {

class World;
struct MapGenInfo;

/// Holds world and area specific information for the map generator.
/// Areas are: Water, Land, Wasteland and Mountains.
struct MapGenAreaInfo {
	// NOCOM(#sirver): convert to enum class
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
	uint32_t getWeight() const {return m_weight;}

private:

	void readTerrains
		(const LuaTable& table, std::vector<Terrain_Index> & list, char const * value_name);

	std::vector<Terrain_Index>  m_Terrains1; //  ocean, coast, inner or foot
	std::vector<Terrain_Index>  m_Terrains2; //  shelf, land, outer or mountain
	std::vector<Terrain_Index>  m_Terrains3; //  shallow, upper, snow

	uint32_t m_weight;
	MapGenAreaType m_areaType;
	const World& m_world;
};

struct MapGenBobKind {
	MapGenBobKind(const LuaTable& table);

	size_t getNumImmovableBobs() const {return m_ImmovableBobs.size();}
	size_t getNumMoveableBobs() const {return m_MoveableBobs.size();}

	const std::string & getImmovableBob(size_t index) const {
		return m_ImmovableBobs[index];
	};
	const std::string & getMoveableBob(size_t index) const {
		return m_MoveableBobs[index];
	};

private:
	std::vector<std::string> m_ImmovableBobs;
	std::vector<std::string> m_MoveableBobs;

};

struct MapGenBobArea {
	MapGenBobArea(const LuaTable& table, MapGenInfo& mapGenInfo);

	uint32_t getWeight() const {return weight_;};
	const MapGenBobKind * getBobKind
		(MapGenAreaInfo::MapGenTerrainType terrType) const;

	uint8_t getImmovableDensity() const {return immovable_density_;};
	uint8_t getMoveableDensity() const {return critter_density_;};

private:
	uint32_t        weight_;
	uint8_t         immovable_density_; // In percent
	uint8_t         critter_density_;  // In percent
	const MapGenBobKind * m_LandCoastBobKind;
	const MapGenBobKind * m_LandInnerBobKind;
	const MapGenBobKind * m_LandUpperBobKind;
	const MapGenBobKind * m_WastelandInnerBobKind;
	const MapGenBobKind * m_WastelandOuterBobKind;
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
	const MapGenBobKind * getBobKind(const std::string & bobKindName) const;

	uint8_t getWaterOceanHeight  () const {return m_ocean_height;}
	uint8_t getWaterShelfHeight  () const {return m_shelf_height;}
	uint8_t getWaterShallowHeight() const {return m_shallow_height;}
	uint8_t getLandCoastHeight   () const {return m_coast_height;}
	uint8_t getLandUpperHeight   () const {return m_upperland_height;}
	uint8_t getMountainFootHeight() const {return m_mountainfoot_height;}
	uint8_t getMountainHeight    () const {return m_mountain_height;}
	uint8_t getSnowHeight        () const {return m_snow_height;}
	uint8_t getSummitHeight      () const {return m_summit_height;}

	uint32_t getSumLandWeight() const;

	size_t getNumBobAreas() const;
	const MapGenBobArea & getBobArea(size_t index) const;
	uint32_t getSumBobAreaWeight() const;

private:
	const World& m_world;

	std::vector<MapGenAreaInfo> m_WaterAreas;
	std::vector<MapGenAreaInfo> m_LandAreas;
	std::vector<MapGenAreaInfo> m_WasteLandAreas;
	std::vector<MapGenAreaInfo> m_MountainAreas;

	std::vector<MapGenBobArea>           m_BobAreas;
	std::map<std::string, MapGenBobKind> m_BobKinds;

	uint8_t m_ocean_height;
	uint8_t m_shelf_height;
	uint8_t m_shallow_height;
	uint8_t m_coast_height;
	uint8_t m_upperland_height;
	uint8_t m_mountainfoot_height;
	uint8_t m_snow_height;
	uint8_t m_mountain_height;
	uint8_t m_summit_height;

	mutable int32_t  m_land_weight;
	mutable bool m_land_weight_valid;

	mutable int32_t m_sum_bob_area_weights;
	mutable bool m_sum_bob_area_weights_valid;

};

}  // namespace Widelands

#endif /* end of include guard: MAP_GEN_H */
