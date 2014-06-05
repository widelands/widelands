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

#include <string>
#include <vector>

#include "logic/world/map_gen.h"

#include "helper.h"
#include "logic/game_data_error.h"
#include "logic/world/world.h"
#include "profile/profile.h"

namespace Widelands {

namespace  {

// NOCOM(#sirver): kill
int split_string(std::vector<std::string>& strs, std::string& str) {
	strs = ::split_string(str, ",");
	return strs.size();
}

}  // namespace


MapGenBobKind::MapGenBobKind(const LuaTable& table) {
	// NOCOM(#sirver): fix
	// std::string str;
	// str = s.get_safe_string("immovables");
	// split_string(m_ImmovableBobs, str);

	// str = s.get_safe_string("critters");
	// split_string(m_MoveableBobs, str);
}

const MapGenBobKind * MapGenBobArea::getBobKind
	(MapGenAreaInfo::MapGenTerrainType terrType) const
{
	switch (terrType)
	{
		case MapGenAreaInfo::ttLandCoast:
			return m_LandCoastBobKind;
		case MapGenAreaInfo::ttLandLand:
			return m_LandInnerBobKind;
		case MapGenAreaInfo::ttLandUpper:
			return m_LandUpperBobKind;
		case MapGenAreaInfo::ttWastelandInner:
			return m_WastelandInnerBobKind;
		case MapGenAreaInfo::ttWastelandOuter:
			return m_WastelandOuterBobKind;
		default:
			return nullptr;
	};
	return nullptr;
}

MapGenBobArea::MapGenBobArea (const LuaTable& table, MapGenInfo & mapGenInfo)
{
	// NOCOM(#sirver): fix
	// weight_ = s.get_int("weight", 1);
	// immovable_density_ =
		// static_cast<uint8_t>
			// (s.get_safe_int("immovable_density"));
	// critter_density_ =
		// static_cast<uint8_t>
			// (s.get_safe_int("critter_density"));

	// std::string str;

	// str = s.get_safe_string("land_coast_bobs");
	// m_LandCoastBobKind      = str.size() ? mapGenInfo.getBobKind(str) : nullptr;

	// str = s.get_safe_string("land_inner_bobs");
	// m_LandInnerBobKind      = str.size() ? mapGenInfo.getBobKind(str) : nullptr;

	// str = s.get_safe_string("land_upper_bobs");
	// m_LandUpperBobKind      = str.size() ? mapGenInfo.getBobKind(str) : nullptr;

	// str = s.get_safe_string("wasteland_inner_bobs");
	// m_WastelandInnerBobKind = str.size() ? mapGenInfo.getBobKind(str) : nullptr;

	// str = s.get_safe_string("wasteland_outer_bobs");
	// m_WastelandOuterBobKind = str.size() ? mapGenInfo.getBobKind(str) : nullptr;
}

void MapGenAreaInfo::readTerrains(const LuaTable& table,
                                  std::vector<Terrain_Index>& list,
                                  char const* const value_name) {
	// NOCOM(#sirver): fix
	// std::string str = s.get_string(value_name, "");
	// if (str.empty())
		// throw game_data_error("terrain info \"%s\" missing in section \"%s\" mapgenconf for world.",
									 // value_name,
									 // s.get_name());
	// std::vector<std::string> strs;

	// split_string(strs, str);

	// for (uint32_t ix = 0; ix < strs.size(); ++ix) {
		// Terrain_Index const tix = m_world.terrains().get_index(strs[ix]);
		// if (tix > 128)
			// throw game_data_error("unknown terrain \"%s\" in section \"%s\" in mapgenconf for world.",
										 // value_name,
										 // s.get_name());
		// list.push_back(tix);
	// }
}

MapGenAreaInfo::MapGenAreaInfo(const LuaTable& table,
                                    const World& world,
                                    MapGenAreaType const areaType)
	: m_world(world) {
		// NOCOM(#sirver): fix
	// m_weight = s.get_positive("weight", 1);
	// switch (areaType) {
		// case atWater:
			// readTerrains(m_Terrains1, s, "ocean_terrains");
			// readTerrains(m_Terrains2, s, "shelf_terrains");
			// readTerrains(m_Terrains3, s, "shallow_terrains");
			// break;
		// case atLand:
			// readTerrains(m_Terrains1, s, "coast_terrains");
			// readTerrains(m_Terrains2, s, "land_terrains");
			// readTerrains(m_Terrains3, s, "upper_terrains");
			// break;
		// case atMountains:
			// readTerrains(m_Terrains1, s, "mountainfoot_terrains");
			// readTerrains(m_Terrains2, s, "mountain_terrains");
			// readTerrains(m_Terrains3, s, "snow_terrains");
			// break;
		// case atWasteland:
			// readTerrains(m_Terrains1, s, "inner_terrains");
			// readTerrains(m_Terrains2, s, "outer_terrains");
			// break;
		// default:
			// throw wexception("bad areaType");
	// }
}

size_t MapGenAreaInfo::getNumTerrains(MapGenTerrainType const terrType) const
{
	switch (terrType) {
		case ttWaterOcean:        return m_Terrains1.size();
		case ttWaterShelf:        return m_Terrains2.size();
		case ttWaterShallow:      return m_Terrains3.size();

		case ttLandCoast:         return m_Terrains1.size();
		case ttLandLand:          return m_Terrains2.size();
		case ttLandUpper:         return m_Terrains3.size();

		case ttWastelandInner:    return m_Terrains1.size();
		case ttWastelandOuter:    return m_Terrains2.size();

		case ttMountainsFoot:     return m_Terrains1.size();
		case ttMountainsMountain: return m_Terrains2.size();
		case ttMountainsSnow:     return m_Terrains3.size();

		default:                  return 0;
	}
}

Terrain_Index MapGenAreaInfo::getTerrain
	(MapGenTerrainType const terrType, uint32_t const index) const
{
	switch (terrType) {
		case ttWaterOcean:        return m_Terrains1[index];
		case ttWaterShelf:        return m_Terrains2[index];
		case ttWaterShallow:      return m_Terrains3[index];

		case ttLandCoast:         return m_Terrains1[index];
		case ttLandLand:          return m_Terrains2[index];
		case ttLandUpper:         return m_Terrains3[index];

		case ttWastelandInner:    return m_Terrains1[index];
		case ttWastelandOuter:    return m_Terrains2[index];

		case ttMountainsFoot:     return m_Terrains1[index];
		case ttMountainsMountain: return m_Terrains2[index];
		case ttMountainsSnow:     return m_Terrains3[index];

		default:                  return 0;
	}
}


uint32_t MapGenInfo::getSumLandWeight() const
{
	if (m_land_weight_valid)
		return m_land_weight;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < getNumAreas(MapGenAreaInfo::atLand); ++ix)
		sum += getArea(MapGenAreaInfo::atLand, ix).getWeight();
	m_land_weight = sum;
	m_land_weight_valid = true;

	return m_land_weight;
}

const MapGenBobArea & MapGenInfo::getBobArea(size_t index) const
{
	return m_BobAreas[index];
}

size_t MapGenInfo::getNumBobAreas() const
{
	return m_BobAreas.size();
}

uint32_t MapGenInfo::getSumBobAreaWeight() const
{
	if (m_sum_bob_area_weights_valid)
		return m_sum_bob_area_weights;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < m_BobAreas.size(); ++ix)
		sum += m_BobAreas[ix].getWeight();
	m_sum_bob_area_weights = sum;
	m_sum_bob_area_weights_valid = true;

	return m_sum_bob_area_weights;
}

size_t MapGenInfo::getNumAreas
	(MapGenAreaInfo::MapGenAreaType const areaType) const
{
	switch (areaType) {
	case MapGenAreaInfo::atWater:     return m_WaterAreas    .size();
	case MapGenAreaInfo::atLand:      return m_LandAreas     .size();
	case MapGenAreaInfo::atMountains: return m_MountainAreas .size();
	case MapGenAreaInfo::atWasteland: return m_WasteLandAreas.size();
	default:
		throw wexception("invalid MapGenAreaType %u", areaType);
	}
}

const MapGenAreaInfo & MapGenInfo::getArea
	(MapGenAreaInfo::MapGenAreaType const areaType,
	 uint32_t const index)
	const
{
	switch (areaType) {
	case MapGenAreaInfo::atWater:     return m_WaterAreas    .at(index);
	case MapGenAreaInfo::atLand:      return m_LandAreas     .at(index);
	case MapGenAreaInfo::atMountains: return m_MountainAreas .at(index);
	case MapGenAreaInfo::atWasteland: return m_WasteLandAreas.at(index);
	default:
		throw wexception("invalid MapGenAreaType %u", areaType);
	}
}

const MapGenBobKind * MapGenInfo::getBobKind
	(const std::string & bobKindName) const
{
	if (m_BobKinds.find(bobKindName) == m_BobKinds.end())
		throw wexception("invalid MapGenBobKind %s", bobKindName.c_str());
	// Ugly workaround because at is not defined for some systems
	// and operator[] does not fare well with constants
	return & m_BobKinds.find(bobKindName)->second;
}

MapGenInfo::MapGenInfo(const LuaTable& table, const World& world) : m_world(world) {
	m_land_weight_valid = false;
	m_sum_bob_area_weights_valid = false;

	// NOCOM(#sirver): fix
	// { //  find out about the general heights
		// Section & s = profile.get_safe_section("heights");
		// m_ocean_height        = s.get_positive("ocean",        10);
		// m_shelf_height        = s.get_positive("shelf",        10);
		// m_shallow_height      = s.get_positive("shallow",      10);
		// m_coast_height        = s.get_positive("coast",        12);
		// m_upperland_height    = s.get_positive("upperland",    16);
		// m_mountainfoot_height = s.get_positive("mountainfoot", 18);
		// m_mountain_height     = s.get_positive("mountain",     20);
		// m_snow_height         = s.get_positive("snow",         33);
		// m_summit_height       = s.get_positive("summit",       40);
	// }


	// //  read the area names
	// Section & areas_s = profile.get_safe_section("areas");
	// std::string              str;
	// std::vector<std::string> water_strs;
	// std::vector<std::string> land_strs;
	// std::vector<std::string> wasteland_strs;
	// std::vector<std::string> mountain_strs;

	// str = areas_s.get_string("water", "water_area");
	// split_string(water_strs, str);

	// str = areas_s.get_string("land", "land_area");
	// split_string(land_strs, str);

	// str = areas_s.get_string("wasteland", "wasteland_area");
	// split_string(wasteland_strs, str);

	// str = areas_s.get_string("mountains", "mountains_area");
	// split_string(mountain_strs, str);

	// for (uint32_t ix = 0; ix < water_strs.size(); ++ix) {
		// // Section & s = profile.get_safe_section(water_strs[ix].c_str());
		// // NOCOM(#sirver): which table
		// m_WaterAreas.push_back(MapGenAreaInfo(table, m_world, MapGenAreaInfo::atWater));
	// }

	// for (uint32_t ix = 0; ix < land_strs.size(); ++ix) {
		// // NOCOM(#sirver): what
		// // Section & s = profile.get_safe_section(land_strs[ix].c_str());
		// m_LandAreas.push_back(MapGenAreaInfo(table, m_world, MapGenAreaInfo::atLand));
	// }

	// for (uint32_t ix = 0; ix < wasteland_strs.size(); ++ix) {
		// // NOCOM(#sirver): what
		// // Section & s = profile.get_safe_section(wasteland_strs[ix].c_str());
		// m_WasteLandAreas.push_back(MapGenAreaInfo(table, m_world, MapGenAreaInfo::atWasteland));
	// }

	// for (uint32_t ix = 0; ix < mountain_strs.size(); ++ix) {
		// // NOCOM(#sirver): what
		// // Section & s = profile.get_safe_section(mountain_strs[ix].c_str());
		// m_MountainAreas.push_back(MapGenAreaInfo(table, m_world, MapGenAreaInfo::atMountains));
	// }

	// Section & bobs_s = profile.get_safe_section("bobs");
	// std::vector<std::string> bob_area_strs;
	// std::vector<std::string> bob_kind_strs;

	// str = bobs_s.get_string("land_resources");
	// split_string(bob_area_strs, str);

	// str = bobs_s.get_string("categories");
	// split_string(bob_kind_strs, str);

	// for (uint32_t ix = 0; ix < bob_kind_strs.size(); ++ix) {
		// // Section & s = profile.get_safe_section(bob_kind_strs[ix].c_str());
		// // NOCOM(#sirver): what
		// MapGenBobKind kind(table);;
		// m_BobKinds[bob_kind_strs[ix]] = kind;

		// for (size_t jx = 0; jx < kind.getNumImmovableBobs(); jx++)
			// if
				// (m_world.get_immovable_index(kind.getImmovableBob(jx).c_str())
				 // <
				 // 0)
				// throw wexception
					// ("unknown immovable %s", kind.getImmovableBob(jx).c_str());

		// for (size_t jx = 0; jx < kind.getNumMoveableBobs(); jx++)
			// if
				// (m_world.get_bob(kind.getMoveableBob(jx).c_str()) < 0)
				// throw wexception
					// ("unknown moveable %s", kind.getMoveableBob(jx).c_str());
	// }

	// for (uint32_t ix = 0; ix < bob_area_strs.size(); ++ix) {
		// // Section & s = profile.get_safe_section(bob_area_strs[ix].c_str());
		// // NOCOM(#sirver): which table
		// m_BobAreas.push_back(MapGenBobArea(table, *this));
	// }
}

}  // namespace Widelands
