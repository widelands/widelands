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

#include "logic/world/map_gen.h"

#include <memory>
#include <string>
#include <vector>

#include "logic/game_data_error.h"
#include "logic/world/world.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"

namespace Widelands {

MapGenBobCategory::MapGenBobCategory(const LuaTable& table) {
	immovables_ = table.get_table("immovables")->array_entries<std::string>();
	critters_ = table.get_table("critters")->array_entries<std::string>();
}

const MapGenBobCategory*
MapGenLandResource::getBobCategory(MapGenAreaInfo::MapGenTerrainType terrType) const {
	switch (terrType) {
	case MapGenAreaInfo::ttLandCoast:
		return land_coast_bob_category_;
	case MapGenAreaInfo::ttLandLand:
		return land_inner_bob_category_;
	case MapGenAreaInfo::ttLandUpper:
		return land_upper_bob_category_;
	case MapGenAreaInfo::ttWastelandInner:
		return wasteland_inner_bob_category_;
	case MapGenAreaInfo::ttWastelandOuter:
		return wasteland_outer_bob_category_;
	default:
		return nullptr;
	};
}

MapGenLandResource::MapGenLandResource(const LuaTable& table, MapGenInfo& mapGenInfo) {
	weight_ = get_uint(table, "weight");

	immovable_density_ = static_cast<uint8_t>(get_uint(table, "immovable_density"));
	critter_density_ = static_cast<uint8_t>(get_uint(table, "critter_density"));

	const auto do_assign = [&table, &mapGenInfo](
	   const std::string& key, const MapGenBobCategory** our_pointer) {
		const std::string value = table.get_string(key);
		if (value.empty()) {
			*our_pointer = nullptr;
			return;
		}
		*our_pointer = mapGenInfo.getBobCategory(value);
	};

	do_assign("land_coast_bobs", &land_coast_bob_category_);
	do_assign("land_inner_bobs", &land_inner_bob_category_);
	do_assign("land_upper_bobs", &land_upper_bob_category_);
	do_assign("wasteland_inner_bobs", &wasteland_inner_bob_category_);
	do_assign("wasteland_outer_bobs", &wasteland_outer_bob_category_);
}

MapGenAreaInfo::MapGenAreaInfo(const LuaTable& table,
                               const World& world,
                               MapGenAreaType const areaType) {
	weight_ = get_positive_int(table, "weight");

	const auto read_terrains = [this, &table, &world](
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

size_t MapGenAreaInfo::getNumTerrains(MapGenTerrainType const terrType) const {
	switch (terrType) {
	case ttWaterOcean:
		return terrains1_.size();
	case ttWaterShelf:
		return terrains2_.size();
	case ttWaterShallow:
		return terrains3_.size();

	case ttLandCoast:
		return terrains1_.size();
	case ttLandLand:
		return terrains2_.size();
	case ttLandUpper:
		return terrains3_.size();

	case ttWastelandInner:
		return terrains1_.size();
	case ttWastelandOuter:
		return terrains2_.size();

	case ttMountainsFoot:
		return terrains1_.size();
	case ttMountainsMountain:
		return terrains2_.size();
	case ttMountainsSnow:
		return terrains3_.size();

	default:
		return 0;
	}
}

Terrain_Index MapGenAreaInfo::getTerrain(MapGenTerrainType const terrType,
                                         uint32_t const index) const {
	switch (terrType) {
	case ttWaterOcean:
		return terrains1_[index];
	case ttWaterShelf:
		return terrains2_[index];
	case ttWaterShallow:
		return terrains3_[index];

	case ttLandCoast:
		return terrains1_[index];
	case ttLandLand:
		return terrains2_[index];
	case ttLandUpper:
		return terrains3_[index];

	case ttWastelandInner:
		return terrains1_[index];
	case ttWastelandOuter:
		return terrains2_[index];

	case ttMountainsFoot:
		return terrains1_[index];
	case ttMountainsMountain:
		return terrains2_[index];
	case ttMountainsSnow:
		return terrains3_[index];

	default:
		return 0;
	}
}

uint32_t MapGenInfo::getSumLandWeight() const {
	if (land_weight_valid_)
		return land_weight_;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < getNumAreas(MapGenAreaInfo::atLand); ++ix)
		sum += getArea(MapGenAreaInfo::atLand, ix).getWeight();
	land_weight_ = sum;
	land_weight_valid_ = true;

	return land_weight_;
}

const MapGenLandResource& MapGenInfo::getLandResource(size_t index) const {
	return land_resources_[index];
}

size_t MapGenInfo::getNumLandResources() const {
	return land_resources_.size();
}

uint32_t MapGenInfo::getSumLandResourceWeight() const {
	if (sum_bob_area_weights_valid_)
		return sum_bob_area_weights_;

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < land_resources_.size(); ++ix)
		sum += land_resources_[ix].getWeight();
	sum_bob_area_weights_ = sum;
	sum_bob_area_weights_valid_ = true;

	return sum_bob_area_weights_;
}

size_t MapGenInfo::getNumAreas(MapGenAreaInfo::MapGenAreaType const areaType) const {
	switch (areaType) {
	case MapGenAreaInfo::atWater:
		return water_areas_.size();
	case MapGenAreaInfo::atLand:
		return land_areas_.size();
	case MapGenAreaInfo::atMountains:
		return mountain_areas_.size();
	case MapGenAreaInfo::atWasteland:
		return wasteland_areas_.size();
	default:
		throw wexception("invalid MapGenAreaType %u", areaType);
	}
}

const MapGenAreaInfo& MapGenInfo::getArea(MapGenAreaInfo::MapGenAreaType const areaType,
                                          uint32_t const index) const {
	switch (areaType) {
	case MapGenAreaInfo::atWater:
		return water_areas_.at(index);
	case MapGenAreaInfo::atLand:
		return land_areas_.at(index);
	case MapGenAreaInfo::atMountains:
		return mountain_areas_.at(index);
	case MapGenAreaInfo::atWasteland:
		return wasteland_areas_.at(index);
	default:
		throw wexception("invalid MapGenAreaType %u", areaType);
	}
}

const MapGenBobCategory* MapGenInfo::getBobCategory(const std::string& bobCategory) const {
	if (bob_categories_.find(bobCategory) == bob_categories_.end())
		throw wexception("invalid MapGenBobCategory %s", bobCategory.c_str());
	// Ugly workaround because at is not defined for some systems
	// and operator[] does not fare well with constants
	return &bob_categories_.find(bobCategory)->second;
}

MapGenInfo::MapGenInfo(const LuaTable& table, const World& world) {
	land_weight_valid_ = false;
	sum_bob_area_weights_valid_ = false;

	{  //  find out about the general heights
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

		const auto read_area = [&world, &areas](const std::string& area_name,
		                                        const MapGenAreaInfo::MapGenAreaType area_type,
		                                        std::vector<MapGenAreaInfo>* area_vector) {
			std::unique_ptr<LuaTable> area(areas->get_table(area_name));
			std::vector<std::unique_ptr<LuaTable>> entries =
			   area->array_entries<std::unique_ptr<LuaTable>>();
			for (std::unique_ptr<LuaTable>& entry : entries) {
				entry->get_string("name");  // name is only for debugging really. Touch it so LuaTable
				                            // will not complain.
				area_vector->push_back(MapGenAreaInfo(*entry, world, area_type));
			}
		};

		read_area("water", MapGenAreaInfo::atWater, &water_areas_);
		read_area("land", MapGenAreaInfo::atLand, &land_areas_);
		read_area("wasteland", MapGenAreaInfo::atWasteland, &wasteland_areas_);
		read_area("mountains", MapGenAreaInfo::atMountains, &mountain_areas_);
	}

	// read the bobs.
	{
		std::unique_ptr<LuaTable> bobs(table.get_table("bob_categories"));

		for (std::unique_ptr<LuaTable>& entry : bobs->array_entries<std::unique_ptr<LuaTable>>()) {
			bob_categories_.insert(
			   std::make_pair(entry->get_string("name"), MapGenBobCategory(*entry)));
			MapGenBobCategory& category = bob_categories_.at(entry->get_string("name"));

			for (size_t jx = 0; jx < category.num_immovables(); jx++)
				if (world.get_immovable_index(category.get_immovable(jx).c_str()) < 0)
					throw wexception("unknown immovable %s", category.get_immovable(jx).c_str());

			for (size_t jx = 0; jx < category.num_critters(); jx++)
				if (world.get_bob(category.get_critter(jx).c_str()) < 0)
					throw wexception("unknown moveable %s", category.get_critter(jx).c_str());
		}
	}

	// read the land resources.
	{
		std::unique_ptr<LuaTable> land_resources(table.get_table("land_resources"));

		for (std::unique_ptr<LuaTable>& entry :
		     land_resources->array_entries<std::unique_ptr<LuaTable>>()) {
			entry->get_string(
			   "name");  // name is only for debugging really. Touch it so LuaTable will not complain.
			land_resources_.push_back(MapGenLandResource(*entry, *this));
		}
	}

	if (getNumAreas(MapGenAreaInfo::atWater) < 1)
		throw GameDataError("missing a water area");

	if (getNumAreas(MapGenAreaInfo::atWater) > 3)
		throw GameDataError("too many water areas (>3)");

	if (getNumAreas(MapGenAreaInfo::atLand) < 1)
		throw GameDataError("missing a land area");

	if (getNumAreas(MapGenAreaInfo::atLand) > 3)
		throw GameDataError("too many land areas (>3)");

	if (getNumAreas(MapGenAreaInfo::atWasteland) < 1)
		throw GameDataError("missing a wasteland area");

	if (getNumAreas(MapGenAreaInfo::atWasteland) > 2)
		throw GameDataError("too many wasteland areas (>2)");

	if (getNumAreas(MapGenAreaInfo::atMountains) < 1)
		throw GameDataError("missing a mountain area");

	if (getNumAreas(MapGenAreaInfo::atMountains) > 1)
		throw GameDataError("too many mountain areas (>1)");

	if (getArea(MapGenAreaInfo::atWater, 0).getNumTerrains(MapGenAreaInfo::ttWaterOcean) < 1)
		throw GameDataError("missing a water/ocean terrain type");

	if (getArea(MapGenAreaInfo::atWater, 0).getNumTerrains(MapGenAreaInfo::ttWaterShelf) < 1)
		throw GameDataError("missing a water/shelf terrain type");

	if (getArea(MapGenAreaInfo::atWater, 0).getNumTerrains(MapGenAreaInfo::ttWaterShallow) < 1)
		throw GameDataError("is missing a water/shallow terrain type");

	if (getArea(MapGenAreaInfo::atLand, 0).getNumTerrains(MapGenAreaInfo::ttLandCoast) < 1)
		throw GameDataError("missing a land/coast terrain type");

	if (getArea(MapGenAreaInfo::atLand, 0).getNumTerrains(MapGenAreaInfo::ttLandLand) < 1)
		throw GameDataError("missing a land/land terrain type");

	if (getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains(MapGenAreaInfo::ttMountainsFoot) < 1)
		throw GameDataError("missing a mountain/foot terrain type");

	if (getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains(MapGenAreaInfo::ttMountainsMountain) <
	    1)
		throw GameDataError("missing a monutain/mountain terrain type");

	if (getArea(MapGenAreaInfo::atMountains, 0).getNumTerrains(MapGenAreaInfo::ttMountainsSnow) < 1)
		throw GameDataError("missing a mountain/snow terrain type");

	if (getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains(MapGenAreaInfo::ttWastelandInner) < 1)
		throw GameDataError("missing a land/coast terrain type");

	if (getArea(MapGenAreaInfo::atWasteland, 0).getNumTerrains(MapGenAreaInfo::ttWastelandOuter) < 1)
		throw GameDataError("missing a land/land terrain type");
}

}  // namespace Widelands
