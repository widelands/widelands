/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map_objects/tribes/tribe_descr.h"

#include <algorithm>
#include <iostream>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace Widelands {

/**
 * The contents of 'table' are documented in
 * /data/tribes/atlanteans.lua
 */
TribeDescr::TribeDescr(const LuaTable& table,
                       const Widelands::TribeBasicInfo& info,
                       const Tribes& init_tribes)
   : name_(table.get_string("name")), descname_(info.descname), tribes_(init_tribes) {

	try {
		initializations_ = info.initializations;

		std::unique_ptr<LuaTable> items_table = table.get_table("roads");
		const auto load_roads = [&items_table](
		                           const std::string& road_type, std::vector<std::string>* images) {
			std::vector<std::string> roads =
			   items_table->get_table(road_type)->array_entries<std::string>();
			for (const std::string& filename : roads) {
				if (g_fs->file_exists(filename)) {
					images->push_back(filename);
				} else {
					throw GameDataError("File '%s' for %s road texture doesn't exist", filename.c_str(),
					                    road_type.c_str());
				}
			}
			if (images->empty()) {
				throw GameDataError("Tribe has no %s roads.", road_type.c_str());
			}
		};
		load_roads("normal", &normal_road_paths_);
		load_roads("busy", &busy_road_paths_);

		// Frontier and flag animations can be a mix of file and spritesheet animations
		const auto load_animations = [this](const LuaTable& animations_table,
		                                    Animation::Type animation_type) {
			if (animations_table.has_key("frontier")) {
				std::unique_ptr<LuaTable> animation_table = animations_table.get_table("frontier");
				frontier_animation_id_ =
				   g_gr->animations().load(name_ + std::string("_frontier"), *animation_table,
				                           animation_table->get_string("basename"), animation_type);
			}
			if (animations_table.has_key("flag")) {
				std::unique_ptr<LuaTable> animation_table = animations_table.get_table("flag");
				flag_animation_id_ =
				   g_gr->animations().load(name_ + std::string("_flag"), *animation_table,
				                           animation_table->get_string("basename"), animation_type);
			}
		};

		if (table.has_key("animations")) {
			load_animations(*table.get_table("animations"), Animation::Type::kFiles);
		}
		if (table.has_key("spritesheets")) {
			load_animations(*table.get_table("spritesheets"), Animation::Type::kSpritesheet);
		}

		items_table = table.get_table("wares_order");
		for (const int key : items_table->keys<int>()) {
			std::vector<DescriptionIndex> column;
			std::vector<std::string> warenames =
			   items_table->get_table(key)->array_entries<std::string>();
			for (size_t rowindex = 0; rowindex < warenames.size(); ++rowindex) {
				try {
					DescriptionIndex wareindex = tribes_.safe_ware_index(warenames[rowindex]);
					if (has_ware(wareindex)) {
						throw GameDataError(
						   "Duplicate definition of ware '%s'", warenames[rowindex].c_str());
					}
					wares_.insert(wareindex);
					column.push_back(wareindex);
				} catch (const WException& e) {
					throw GameDataError(
					   "Failed adding ware '%s: %s", warenames[rowindex].c_str(), e.what());
				}
			}
			if (!column.empty()) {
				wares_order_.push_back(column);
			}
		}

		items_table = table.get_table("workers_order");
		for (const int key : items_table->keys<int>()) {
			std::vector<DescriptionIndex> column;
			for (const std::string& workername :
			     items_table->get_table(key)->array_entries<std::string>()) {
				add_worker(workername, column);
			}
			if (!column.empty()) {
				workers_order_.push_back(column);
			}
		}

		for (const std::string& immovablename :
		     table.get_table("immovables")->array_entries<std::string>()) {
			try {
				DescriptionIndex index = tribes_.safe_immovable_index(immovablename);
				if (immovables_.count(index) == 1) {
					throw GameDataError("Duplicate definition of immovable '%s'", immovablename.c_str());
				}
				immovables_.insert(index);
			} catch (const WException& e) {
				throw GameDataError(
				   "Failed adding immovable '%s': %s", immovablename.c_str(), e.what());
			}
		}

		items_table = table.get_table("resource_indicators");
		for (std::string resource : items_table->keys<std::string>()) {
			ResourceIndicatorList resis;
			std::unique_ptr<LuaTable> tbl = items_table->get_table(resource);
			const std::set<int> keys = tbl->keys<int>();
			for (int upper_limit : keys) {
				resis[upper_limit] = tribes_.safe_immovable_index(tbl->get_string(upper_limit));
			}
			if (resis.empty()) {
				throw GameDataError("Tribe has no indicators for resource %s.", resource.c_str());
			}
			resource_indicators_[resource] = resis;
		}

		ship_names_ = table.get_table("ship_names")->array_entries<std::string>();

		for (const std::string& buildingname :
		     table.get_table("buildings")->array_entries<std::string>()) {
			add_building(buildingname);
		}

		// Special types
		builder_ = add_special_worker(table.get_string("builder"));
		carrier_ = add_special_worker(table.get_string("carrier"));
		carrier2_ = add_special_worker(table.get_string("carrier2"));
		geologist_ = add_special_worker(table.get_string("geologist"));
		soldier_ = add_special_worker(table.get_string("soldier"));

		const std::string shipname = table.get_string("ship");
		try {
			ship_ = tribes_.safe_ship_index(shipname);
		} catch (const WException& e) {
			throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
		}

		port_ = add_special_building(table.get_string("port"));

		ironore_ = add_special_ware(table.get_string("ironore"));
		rawlog_ = add_special_ware(table.get_string("rawlog"));
		refinedlog_ = add_special_ware(table.get_string("refinedlog"));
		granite_ = add_special_ware(table.get_string("granite"));

		if (table.has_key<std::string>("toolbar")) {
			toolbar_image_set_.reset(new ToolbarImageset(*table.get_table("toolbar")));
		}
	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}
}

/**
 * Access functions
 */

const std::string& TribeDescr::name() const {
	return name_;
}
const std::string& TribeDescr::descname() const {
	return descname_;
}

size_t TribeDescr::get_nrwares() const {
	return wares_.size();
}
size_t TribeDescr::get_nrworkers() const {
	return workers_.size();
}

const std::vector<DescriptionIndex>& TribeDescr::buildings() const {
	return buildings_;
}
const std::set<DescriptionIndex>& TribeDescr::wares() const {
	return wares_;
}
const std::set<DescriptionIndex>& TribeDescr::workers() const {
	return workers_;
}
const std::set<DescriptionIndex>& TribeDescr::immovables() const {
	return immovables_;
}
const ResourceIndicatorSet& TribeDescr::resource_indicators() const {
	return resource_indicators_;
}

bool TribeDescr::has_building(const DescriptionIndex& index) const {
	return std::find(buildings_.begin(), buildings_.end(), index) != buildings_.end();
}
bool TribeDescr::has_ware(const DescriptionIndex& index) const {
	return wares_.count(index) == 1;
}
bool TribeDescr::has_worker(const DescriptionIndex& index) const {
	return workers_.count(index) == 1;
}
bool TribeDescr::has_immovable(const DescriptionIndex& index) const {
	return immovables_.count(index) == 1;
}
bool TribeDescr::is_construction_material(const DescriptionIndex& index) const {
	return construction_materials_.count(index) == 1;
}

DescriptionIndex TribeDescr::building_index(const std::string& buildingname) const {
	return tribes_.building_index(buildingname);
}

DescriptionIndex TribeDescr::immovable_index(const std::string& immovablename) const {
	return tribes_.immovable_index(immovablename);
}
DescriptionIndex TribeDescr::ware_index(const std::string& warename) const {
	return tribes_.ware_index(warename);
}
DescriptionIndex TribeDescr::worker_index(const std::string& workername) const {
	return tribes_.worker_index(workername);
}

DescriptionIndex TribeDescr::safe_building_index(const std::string& buildingname) const {
	return tribes_.safe_building_index(buildingname);
}

DescriptionIndex TribeDescr::safe_ware_index(const std::string& warename) const {
	return tribes_.safe_ware_index(warename);
}
DescriptionIndex TribeDescr::safe_worker_index(const std::string& workername) const {
	return tribes_.safe_worker_index(workername);
}

WareDescr const* TribeDescr::get_ware_descr(const DescriptionIndex& index) const {
	return tribes_.get_ware_descr(index);
}
WorkerDescr const* TribeDescr::get_worker_descr(const DescriptionIndex& index) const {
	return tribes_.get_worker_descr(index);
}

BuildingDescr const* TribeDescr::get_building_descr(const DescriptionIndex& index) const {
	return tribes_.get_building_descr(index);
}
ImmovableDescr const* TribeDescr::get_immovable_descr(const DescriptionIndex& index) const {
	return tribes_.get_immovable_descr(index);
}

DescriptionIndex TribeDescr::builder() const {
	assert(tribes_.worker_exists(builder_));
	return builder_;
}
DescriptionIndex TribeDescr::carrier() const {
	assert(tribes_.worker_exists(carrier_));
	return carrier_;
}
DescriptionIndex TribeDescr::carrier2() const {
	assert(tribes_.worker_exists(carrier2_));
	return carrier2_;
}
DescriptionIndex TribeDescr::geologist() const {
	assert(tribes_.worker_exists(geologist_));
	return geologist_;
}
DescriptionIndex TribeDescr::soldier() const {
	assert(tribes_.worker_exists(soldier_));
	return soldier_;
}
DescriptionIndex TribeDescr::ship() const {
	assert(tribes_.ship_exists(ship_));
	return ship_;
}
DescriptionIndex TribeDescr::port() const {
	assert(tribes_.building_exists(port_));
	return port_;
}
DescriptionIndex TribeDescr::ironore() const {
	assert(tribes_.ware_exists(ironore_));
	return ironore_;
}
DescriptionIndex TribeDescr::rawlog() const {
	assert(tribes_.ware_exists(rawlog_));
	return rawlog_;
}
DescriptionIndex TribeDescr::refinedlog() const {
	assert(tribes_.ware_exists(refinedlog_));
	return refinedlog_;
}
DescriptionIndex TribeDescr::granite() const {
	assert(tribes_.ware_exists(granite_));
	return granite_;
}

const std::vector<DescriptionIndex>& TribeDescr::trainingsites() const {
	return trainingsites_;
}
const std::vector<DescriptionIndex>& TribeDescr::worker_types_without_cost() const {
	return worker_types_without_cost_;
}

uint32_t TribeDescr::frontier_animation() const {
	return frontier_animation_id_;
}

uint32_t TribeDescr::flag_animation() const {
	return flag_animation_id_;
}

const std::vector<std::string>& TribeDescr::normal_road_paths() const {
	return normal_road_paths_;
}

const std::vector<std::string>& TribeDescr::busy_road_paths() const {
	return busy_road_paths_;
}

void TribeDescr::add_normal_road_texture(const Image* texture) {
	road_textures_.add_normal_road_texture(texture);
}

void TribeDescr::add_busy_road_texture(const Image* texture) {
	road_textures_.add_busy_road_texture(texture);
}

const RoadTextures& TribeDescr::road_textures() const {
	return road_textures_;
}

/*
==============
Find the best matching indicator for the given amount.
==============
*/
DescriptionIndex TribeDescr::get_resource_indicator(ResourceDescription const* const res,
                                                    const ResourceAmount amount) const {
	if (!res || !amount) {
		auto list = resource_indicators_.find("");
		if (list == resource_indicators_.end() || list->second.empty()) {
			throw GameDataError("Tribe '%s' has no indicator for no resources!", name_.c_str());
		}
		return list->second.begin()->second;
	}

	auto list = resource_indicators_.find(res->name());
	if (list == resource_indicators_.end() || list->second.empty()) {
		throw GameDataError(
		   "Tribe '%s' has no indicators for resource '%s'!", name_.c_str(), res->name().c_str());
	}

	uint32_t lowest = 0;
	for (const auto& resi : list->second) {
		if (resi.first < amount) {
			continue;
		} else if (lowest < amount || resi.first < lowest) {
			lowest = resi.first;
		}
	}

	if (lowest < amount) {
		throw GameDataError("Tribe '%s' has no indicators for amount %i of resource '%s' (highest "
		                    "possible amount is %i)!",
		                    name_.c_str(), amount, res->name().c_str(), lowest);
	}

	return list->second.find(lowest)->second;
}

void TribeDescr::add_building(const std::string& buildingname) {
	try {
		DescriptionIndex index = tribes_.safe_building_index(buildingname);
		if (has_building(index)) {
			throw GameDataError("Duplicate definition of building '%s'", buildingname.c_str());
		}
		buildings_.push_back(index);

		// Register trainigsites
		if (get_building_descr(index)->type() == MapObjectType::TRAININGSITE) {
			trainingsites_.push_back(index);
		}

		// Register construction materials
		for (const auto& build_cost : get_building_descr(index)->buildcost()) {
			if (!is_construction_material(build_cost.first)) {
				construction_materials_.insert(build_cost.first);
			}
		}
		for (const auto& enhancement_cost : get_building_descr(index)->enhancement_cost()) {
			if (!is_construction_material(enhancement_cost.first)) {
				construction_materials_.insert(enhancement_cost.first);
			}
		}
	} catch (const WException& e) {
		throw GameDataError("Failed adding building '%s': %s", buildingname.c_str(), e.what());
	}
}

void TribeDescr::add_worker(const std::string& workername,
                            std::vector<DescriptionIndex>& workers_order_column) {
	try {
		DescriptionIndex workerindex = tribes_.safe_worker_index(workername);
		if (has_worker(workerindex)) {
			throw GameDataError("Duplicate definition of worker '%s'", workername.c_str());
		}
		workers_.insert(workerindex);
		workers_order_column.push_back(workerindex);

		const WorkerDescr& worker_descr = *tribes_.get_worker_descr(workerindex);
		if (worker_descr.is_buildable() && worker_descr.buildcost().empty()) {
			worker_types_without_cost_.push_back(workerindex);
		}
	} catch (const WException& e) {
		throw GameDataError("Failed adding worker '%s: %s", workername.c_str(), e.what());
	}
}

void TribeDescr::add_worker(const std::string& workername) {
	add_worker(workername, workers_order_.back());
}

ToolbarImageset* TribeDescr::toolbar_image_set() const {
	return toolbar_image_set_.get();
}

/**
 * Helper functions
 */

DescriptionIndex TribeDescr::add_special_worker(const std::string& workername) {
	try {
		DescriptionIndex worker = tribes_.safe_worker_index(workername);
		if (!has_worker(worker)) {
			throw GameDataError("This tribe doesn't have the worker '%s'", workername.c_str());
		}
		return worker;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special worker '%s': %s", workername.c_str(), e.what());
	}
}

DescriptionIndex TribeDescr::add_special_building(const std::string& buildingname) {
	try {
		DescriptionIndex building = tribes_.safe_building_index(buildingname);
		if (!has_building(building)) {
			throw GameDataError("This tribe doesn't have the building '%s'", buildingname.c_str());
		}
		return building;
	} catch (const WException& e) {
		throw GameDataError(
		   "Failed adding special building '%s': %s", buildingname.c_str(), e.what());
	}
}
DescriptionIndex TribeDescr::add_special_ware(const std::string& warename) {
	try {
		DescriptionIndex ware = tribes_.safe_ware_index(warename);
		if (!has_ware(ware)) {
			throw GameDataError("This tribe doesn't have the ware '%s'", warename.c_str());
		}
		return ware;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special ware '%s': %s", warename.c_str(), e.what());
	}
}
}  // namespace Widelands
