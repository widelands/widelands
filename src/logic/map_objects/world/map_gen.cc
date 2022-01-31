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

#include "logic/map_objects/world/map_gen.h"

#include <memory>

#include "base/wexception.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions.h"
#include "scripting/lua_table.h"

namespace Widelands {

MapGenBobCategory::MapGenBobCategory(const LuaTable& table)
   : immovables_(table.get_table("immovables")->array_entries<std::string>()),
     critters_(table.get_table("critters")->array_entries<std::string>()) {
}

const MapGenBobCategory*
MapGenLandResource::get_bob_category(MapGenAreaInfo::Terrain terrain_type) const {
	switch (terrain_type) {
	case MapGenAreaInfo::Terrain::kLandCoast:
		return land_coast_bob_category_;
	case MapGenAreaInfo::Terrain::kLandLand:
		return land_inner_bob_category_;
	case MapGenAreaInfo::Terrain::kLandUpper:
		return land_upper_bob_category_;
	case MapGenAreaInfo::Terrain::kWastelandInner:
		return wasteland_inner_bob_category_;
	case MapGenAreaInfo::Terrain::kWastelandOuter:
		return wasteland_outer_bob_category_;
	case MapGenAreaInfo::Terrain::kWaterOcean:
	case MapGenAreaInfo::Terrain::kWaterShelf:
	case MapGenAreaInfo::Terrain::kWaterShallow:
	case MapGenAreaInfo::Terrain::kMountainsFoot:
	case MapGenAreaInfo::Terrain::kMountainsMountain:
	case MapGenAreaInfo::Terrain::kMountainsSnow:
		return nullptr;
	}
	NEVER_HERE();
}

MapGenLandResource::MapGenLandResource(const LuaTable& table, MapGenInfo& map_gen_info) {
	weight_ = get_uint(table, "weight");

	immovable_density_ = static_cast<uint8_t>(get_uint(table, "immovable_density"));
	critter_density_ = static_cast<uint8_t>(get_uint(table, "critter_density"));

	const auto do_assign = [&table, &map_gen_info](
	                          const std::string& key, const MapGenBobCategory** our_pointer) {
		const std::string value = table.get_string(key);
		if (value.empty()) {
			*our_pointer = nullptr;
			return;
		}
		*our_pointer = map_gen_info.get_bob_category(value);
	};

	do_assign("land_coast_bobs", &land_coast_bob_category_);
	do_assign("land_inner_bobs", &land_inner_bob_category_);
	do_assign("land_upper_bobs", &land_upper_bob_category_);
	do_assign("wasteland_inner_bobs", &wasteland_inner_bob_category_);
	do_assign("wasteland_outer_bobs", &wasteland_outer_bob_category_);
}

MapGenAreaInfo::MapGenAreaInfo(const LuaTable& table,
                               const Descriptions& descriptions,
                               Area const area_type) {
	weight_ = get_positive_int(table, "weight");

	const auto read_terrains = [&table, &descriptions](
	                              const std::string& key, std::vector<DescriptionIndex>* list) {
		const std::vector<std::string> terrains = table.get_table(key)->array_entries<std::string>();

		for (const std::string& terrain : terrains) {
			const DescriptionIndex tix = descriptions.terrains().get_index(terrain);
			if (tix == INVALID_INDEX) {
				throw GameDataError("Random Map Generator: Unknown terrain '%s'", terrain.c_str());
			}
			list->push_back(tix);
		}
	};

	switch (area_type) {
	case MapGenAreaInfo::Area::kWater:
		read_terrains("ocean_terrains", &terrains1_);
		read_terrains("shelf_terrains", &terrains2_);
		read_terrains("shallow_terrains", &terrains3_);
		break;
	case MapGenAreaInfo::Area::kLand:
		read_terrains("coast_terrains", &terrains1_);
		read_terrains("land_terrains", &terrains2_);
		read_terrains("upper_terrains", &terrains3_);
		break;
	case MapGenAreaInfo::Area::kMountains:
		read_terrains("mountainfoot_terrains", &terrains1_);
		read_terrains("mountain_terrains", &terrains2_);
		read_terrains("snow_terrains", &terrains3_);
		break;
	case MapGenAreaInfo::Area::kWasteland:
		read_terrains("inner_terrains", &terrains1_);
		read_terrains("outer_terrains", &terrains2_);
		break;
	}
}

size_t MapGenAreaInfo::get_num_terrains(Terrain const terrain_type) const {
	switch (terrain_type) {
	case MapGenAreaInfo::Terrain::kWaterOcean:
		return terrains1_.size();
	case MapGenAreaInfo::Terrain::kWaterShelf:
		return terrains2_.size();
	case MapGenAreaInfo::Terrain::kWaterShallow:
		return terrains3_.size();

	case MapGenAreaInfo::Terrain::kLandCoast:
		return terrains1_.size();
	case MapGenAreaInfo::Terrain::kLandLand:
		return terrains2_.size();
	case MapGenAreaInfo::Terrain::kLandUpper:
		return terrains3_.size();

	case MapGenAreaInfo::Terrain::kWastelandInner:
		return terrains1_.size();
	case MapGenAreaInfo::Terrain::kWastelandOuter:
		return terrains2_.size();

	case MapGenAreaInfo::Terrain::kMountainsFoot:
		return terrains1_.size();
	case MapGenAreaInfo::Terrain::kMountainsMountain:
		return terrains2_.size();
	case MapGenAreaInfo::Terrain::kMountainsSnow:
		return terrains3_.size();
	}
	NEVER_HERE();
}

DescriptionIndex MapGenAreaInfo::get_terrain(Terrain const terrain_type,
                                             uint32_t const index) const {
	switch (terrain_type) {
	case MapGenAreaInfo::Terrain::kWaterOcean:
		return terrains1_[index];
	case MapGenAreaInfo::Terrain::kWaterShelf:
		return terrains2_[index];
	case MapGenAreaInfo::Terrain::kWaterShallow:
		return terrains3_[index];

	case MapGenAreaInfo::Terrain::kLandCoast:
		return terrains1_[index];
	case MapGenAreaInfo::Terrain::kLandLand:
		return terrains2_[index];
	case MapGenAreaInfo::Terrain::kLandUpper:
		return terrains3_[index];

	case MapGenAreaInfo::Terrain::kWastelandInner:
		return terrains1_[index];
	case MapGenAreaInfo::Terrain::kWastelandOuter:
		return terrains2_[index];

	case MapGenAreaInfo::Terrain::kMountainsFoot:
		return terrains1_[index];
	case MapGenAreaInfo::Terrain::kMountainsMountain:
		return terrains2_[index];
	case MapGenAreaInfo::Terrain::kMountainsSnow:
		return terrains3_[index];
	}
	NEVER_HERE();
}

uint32_t MapGenInfo::get_sum_land_weight() const {
	if (land_weight_valid_) {
		return land_weight_;
	}

	uint32_t sum = 0;
	for (uint32_t ix = 0; ix < get_num_areas(MapGenAreaInfo::Area::kLand); ++ix) {
		sum += get_area(MapGenAreaInfo::Area::kLand, ix).get_weight();
	}
	land_weight_ = sum;
	land_weight_valid_ = true;

	return land_weight_;
}

const MapGenLandResource& MapGenInfo::get_land_resource(size_t index) const {
	return land_resources_[index];
}

size_t MapGenInfo::get_num_land_resources() const {
	return land_resources_.size();
}

uint32_t MapGenInfo::get_sum_land_resource_weight() const {
	if (sum_bob_area_weights_valid_) {
		return sum_bob_area_weights_;
	}

	uint32_t sum = 0;
	for (const MapGenLandResource& land_resource : land_resources_) {
		sum += land_resource.get_weight();
	}
	sum_bob_area_weights_ = sum;
	sum_bob_area_weights_valid_ = true;

	return sum_bob_area_weights_;
}

size_t MapGenInfo::get_num_areas(MapGenAreaInfo::Area const area_type) const {
	switch (area_type) {
	case MapGenAreaInfo::Area::kWater:
		return water_areas_.size();
	case MapGenAreaInfo::Area::kLand:
		return land_areas_.size();
	case MapGenAreaInfo::Area::kMountains:
		return mountain_areas_.size();
	case MapGenAreaInfo::Area::kWasteland:
		return wasteland_areas_.size();
	}
	NEVER_HERE();
}

const MapGenAreaInfo& MapGenInfo::get_area(MapGenAreaInfo::Area const area_type,
                                           uint32_t const index) const {
	switch (area_type) {
	case MapGenAreaInfo::Area::kWater:
		return water_areas_.at(index);
	case MapGenAreaInfo::Area::kLand:
		return land_areas_.at(index);
	case MapGenAreaInfo::Area::kMountains:
		return mountain_areas_.at(index);
	case MapGenAreaInfo::Area::kWasteland:
		return wasteland_areas_.at(index);
	}
	NEVER_HERE();
}

const MapGenBobCategory* MapGenInfo::get_bob_category(const std::string& bob_category) const {
	if (bob_categories_.find(bob_category) == bob_categories_.end()) {
		throw wexception("invalid MapGenBobCategory %s", bob_category.c_str());
	}
	// Ugly workaround because at is not defined for some systems
	// and operator[] does not fare well with constants
	return &bob_categories_.find(bob_category)->second;
}

MapGenInfo::MapGenInfo(const LuaTable& table, const Descriptions& descriptions) {
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

		const auto read_area = [&descriptions, &areas](const std::string& area_name,
		                                               const MapGenAreaInfo::Area area_type,
		                                               std::vector<MapGenAreaInfo>* area_vector) {
			std::unique_ptr<LuaTable> area(areas->get_table(area_name));
			std::vector<std::unique_ptr<LuaTable>> entries =
			   area->array_entries<std::unique_ptr<LuaTable>>();
			for (std::unique_ptr<LuaTable>& entry : entries) {
				entry->get_string("name");  // name is only for debugging really. Touch it so LuaTable
				                            // will not complain.
				area_vector->push_back(MapGenAreaInfo(*entry, descriptions, area_type));
			}
		};

		read_area("water", MapGenAreaInfo::Area::kWater, &water_areas_);
		read_area("land", MapGenAreaInfo::Area::kLand, &land_areas_);
		read_area("wasteland", MapGenAreaInfo::Area::kWasteland, &wasteland_areas_);
		read_area("mountains", MapGenAreaInfo::Area::kMountains, &mountain_areas_);
	}

	// read the bobs.
	{
		std::unique_ptr<LuaTable> bobs(table.get_table("bob_categories"));

		for (std::unique_ptr<LuaTable>& entry : bobs->array_entries<std::unique_ptr<LuaTable>>()) {
			bob_categories_.insert(
			   std::make_pair(entry->get_string("name"), MapGenBobCategory(*entry)));
			MapGenBobCategory& category = bob_categories_.at(entry->get_string("name"));

			for (size_t jx = 0; jx < category.num_immovables(); jx++) {
				if (descriptions.immovable_index(category.get_immovable(jx)) ==
				    Widelands::INVALID_INDEX) {
					throw wexception("unknown immovable %s", category.get_immovable(jx).c_str());
				}
			}

			for (size_t jx = 0; jx < category.num_critters(); jx++) {
				if (descriptions.critter_index(category.get_critter(jx)) == Widelands::INVALID_INDEX) {
					throw wexception("unknown critter %s", category.get_critter(jx).c_str());
				}
			}
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

	if (get_num_areas(MapGenAreaInfo::Area::kWater) < 1) {
		throw GameDataError("missing a water area");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kWater) > 3) {
		throw GameDataError("too many water areas (>3)");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kLand) < 1) {
		throw GameDataError("missing a land area");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kLand) > 3) {
		throw GameDataError("too many land areas (>3)");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kWasteland) < 1) {
		throw GameDataError("missing a wasteland area");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kWasteland) > 2) {
		throw GameDataError("too many wasteland areas (>2)");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kMountains) < 1) {
		throw GameDataError("missing a mountain area");
	}
	if (get_num_areas(MapGenAreaInfo::Area::kMountains) > 1) {
		throw GameDataError("too many mountain areas (>1)");
	}
	if (get_area(MapGenAreaInfo::Area::kWater, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kWaterOcean) < 1) {
		throw GameDataError("missing a water/ocean terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kWater, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kWaterShelf) < 1) {
		throw GameDataError("missing a water/shelf terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kWater, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kWaterShallow) < 1) {
		throw GameDataError("is missing a water/shallow terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kLand, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kLandCoast) < 1) {
		throw GameDataError("missing a land/coast terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kLand, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kLandLand) < 1) {
		throw GameDataError("missing a land/land terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kMountains, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kMountainsFoot) < 1) {
		throw GameDataError("missing a mountain/foot terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kMountains, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kMountainsMountain) < 1) {
		throw GameDataError("missing a monutain/mountain terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kMountains, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kMountainsSnow) < 1) {
		throw GameDataError("missing a mountain/snow terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kWasteland, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kWastelandInner) < 1) {
		throw GameDataError("missing a land/coast terrain type");
	}
	if (get_area(MapGenAreaInfo::Area::kWasteland, 0)
	       .get_num_terrains(MapGenAreaInfo::Terrain::kWastelandOuter) < 1) {
		throw GameDataError("missing a land/land terrain type");
	}
}

}  // namespace Widelands
