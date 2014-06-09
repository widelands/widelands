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
#include "scripting/lua_table.h"

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

MapGenAreaInfo::MapGenAreaInfo(const LuaTable& table,
                                    const World& world,
                                    MapGenAreaType const areaType)
	// NOCOM(#sirver): probably unused
	: world_(world) {
	weight_ = get_positive_int(table, "weight");

	const auto& read_terrains = [this, &table, &world](
	   const std::string& key, std::vector<Terrain_Index>* list) {
		const std::vector<std::string> terrains = table.get_table(key)->array_entries<std::string>();

		for (const std::string& terrain : terrains) {
			const Terrain_Index tix = world.terrains().get_index(terrain);
			list->push_back(tix);
		}
	};

	switch (areaType) {
	case atWater:
		read_terrains("ocean_terrains", &terrains1_);
		read_terrains("shelf_terrains", &terrains2_);
		read_terrains("shallow_terrains", &terrains3_);
		break;
	case atLand:
		read_terrains("coast_terrains", &terrains1_);
		read_terrains("land_terrains", &terrains2_);
		read_terrains("upper_terrains", &terrains3_);
		break;
	case atMountains:
		read_terrains("mountainfoot_terrains", &terrains1_);
		read_terrains("mountain_terrains", &terrains2_);
		read_terrains("snow_terrains", &terrains3_);
		break;
	case atWasteland:
		read_terrains("inner_terrains", &terrains1_);
		read_terrains("outer_terrains", &terrains2_);
		break;
	default:
		throw wexception("bad areaType");
	}
}

size_t MapGenAreaInfo::getNumTerrains(MapGenTerrainType const terrType) const
{
	switch (terrType) {
		case ttWaterOcean:        return terrains1_.size();
		case ttWaterShelf:        return terrains2_.size();
		case ttWaterShallow:      return terrains3_.size();

		case ttLandCoast:         return terrains1_.size();
		case ttLandLand:          return terrains2_.size();
		case ttLandUpper:         return terrains3_.size();

		case ttWastelandInner:    return terrains1_.size();
		case ttWastelandOuter:    return terrains2_.size();

		case ttMountainsFoot:     return terrains1_.size();
		case ttMountainsMountain: return terrains2_.size();
		case ttMountainsSnow:     return terrains3_.size();

		default:                  return 0;
	}
}

Terrain_Index MapGenAreaInfo::getTerrain
	(MapGenTerrainType const terrType, uint32_t const index) const
{
	switch (terrType) {
		case ttWaterOcean:        return terrains1_[index];
		case ttWaterShelf:        return terrains2_[index];
		case ttWaterShallow:      return terrains3_[index];

		case ttLandCoast:         return terrains1_[index];
		case ttLandLand:          return terrains2_[index];
		case ttLandUpper:         return terrains3_[index];

		case ttWastelandInner:    return terrains1_[index];
		case ttWastelandOuter:    return terrains2_[index];

		case ttMountainsFoot:     return terrains1_[index];
		case ttMountainsMountain: return terrains2_[index];
		case ttMountainsSnow:     return terrains3_[index];

		default:                  return 0;
	}
}


uint32_t MapGenInfo::getSumLandWeight() const
{
	if (land_weight_valid_)
		return land_weight_;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < getNumAreas(MapGenAreaInfo::atLand); ++ix)
		sum += getArea(MapGenAreaInfo::atLand, ix).getWeight();
	land_weight_ = sum;
	land_weight_valid_ = true;

	return land_weight_;
}

const MapGenBobArea & MapGenInfo::getBobArea(size_t index) const
{
	return bob_areas_[index];
}

size_t MapGenInfo::getNumBobAreas() const
{
	return bob_areas_.size();
}

uint32_t MapGenInfo::getSumBobAreaWeight() const
{
	if (sum_bob_area_weights_valid_)
		return sum_bob_area_weights_;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < bob_areas_.size(); ++ix)
		sum += bob_areas_[ix].getWeight();
	sum_bob_area_weights_ = sum;
	sum_bob_area_weights_valid_ = true;

	return sum_bob_area_weights_;
}

size_t MapGenInfo::getNumAreas
	(MapGenAreaInfo::MapGenAreaType const areaType) const
{
	switch (areaType) {
	case MapGenAreaInfo::atWater:     return water_areas_    .size();
	case MapGenAreaInfo::atLand:      return land_areas_     .size();
	case MapGenAreaInfo::atMountains: return mountain_areas_ .size();
	case MapGenAreaInfo::atWasteland: return wasteland_areas_.size();
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
	case MapGenAreaInfo::atWater:     return water_areas_    .at(index);
	case MapGenAreaInfo::atLand:      return land_areas_     .at(index);
	case MapGenAreaInfo::atMountains: return mountain_areas_ .at(index);
	case MapGenAreaInfo::atWasteland: return wasteland_areas_.at(index);
	default:
		throw wexception("invalid MapGenAreaType %u", areaType);
	}
}

const MapGenBobKind * MapGenInfo::getBobKind
	(const std::string & bobKindName) const
{
	if (bob_kinds_.find(bobKindName) == bob_kinds_.end())
		throw wexception("invalid MapGenBobKind %s", bobKindName.c_str());
	// Ugly workaround because at is not defined for some systems
	// and operator[] does not fare well with constants
	return & bob_kinds_.find(bobKindName)->second;
}

MapGenInfo::MapGenInfo(const LuaTable& table, const World& world) : world_(world) {
	land_weight_valid_ = false;
	sum_bob_area_weights_valid_ = false;

	{ //  find out about the general heights
		std::unique_ptr<LuaTable> heights(table.get_table("heights"));
		ocean_height_ = get_uint(*heights, "ocean");
		shelf_height_ = get_uint(*heights, "shelf");
		shallow_height_ = get_uint(*heights, "shallow");
		coast_height_ = get_uint(*heights, "coast");
		upperland_height_ = get_uint(*heights, "upperland");
		mountainfoot_height_ = get_uint(*heights, "mountainfoot");
		mountain_height_ = get_uint(*heights, "mountain");
		snow_height_ = get_uint(*heights, "snow");
		summit_height_ = get_uint(*heights, "summit");
	}

	//  read the areas.
	{
		std::unique_ptr<LuaTable> areas(table.get_table("areas"));

		const auto read_area = [this, &areas](const std::string& area_name,
		                                       const MapGenAreaInfo::MapGenAreaType area_type,
		                                       std::vector<MapGenAreaInfo>* area_vector) {
			std::unique_ptr<LuaTable> area(areas->get_table(area_name));
			std::vector<std::unique_ptr<LuaTable>> entries =
			   area->array_entries<std::unique_ptr<LuaTable>>();
			log("#sirver entries.size(): %u\n", entries.size());
			for (std::unique_ptr<LuaTable>& entry : entries) {
				log("#sirver entry.get_string('name'): %s\n", entry->get_string("name").c_str());
				area_vector->push_back(MapGenAreaInfo(*entry, world_, area_type));
			}
		};

		read_area("water", MapGenAreaInfo::atWater, &water_areas_);
		read_area("land", MapGenAreaInfo::atLand, &land_areas_);
		read_area("wasteland", MapGenAreaInfo::atWasteland, &wasteland_areas_);
		read_area("mountains", MapGenAreaInfo::atMountains, &mountain_areas_);
	}
	log("#sirver ALIVE %s:%i\n", __FILE__, __LINE__);

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
		// bob_kinds_[bob_kind_strs[ix]] = kind;

		// for (size_t jx = 0; jx < kind.getNumImmovableBobs(); jx++)
			// if
				// (world_.get_immovable_index(kind.getImmovableBob(jx).c_str())
				 // <
				 // 0)
				// throw wexception
					// ("unknown immovable %s", kind.getImmovableBob(jx).c_str());

		// for (size_t jx = 0; jx < kind.getNumMoveableBobs(); jx++)
			// if
				// (world_.get_bob(kind.getMoveableBob(jx).c_str()) < 0)
				// throw wexception
					// ("unknown moveable %s", kind.getMoveableBob(jx).c_str());
	// }

	// for (uint32_t ix = 0; ix < bob_area_strs.size(); ++ix) {
		// // Section & s = profile.get_safe_section(bob_area_strs[ix].c_str());
		// // NOCOM(#sirver): which table
		// bob_areas_.push_back(MapGenBobArea(table, *this));
	// }
}

}  // namespace Widelands
