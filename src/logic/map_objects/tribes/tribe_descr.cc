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
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
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
#include "scripting/lua_table.h"

namespace Widelands {

/**
 * The contents of 'table' are documented in
 * /data/tribes/atlanteans.lua
 */
TribeDescr::TribeDescr(const Widelands::TribeBasicInfo& info,
                       Tribes& tribes,
                       const World& world,
                       const LuaTable& table,
                       const LuaTable* scenario_table)
   : name_(table.get_string("name")),
     descname_(info.descname),
     tribes_(tribes),
     bridge_height_(table.get_int("bridge_height")),
     builder_(Widelands::INVALID_INDEX),
     carrier_(Widelands::INVALID_INDEX),
     carrier2_(Widelands::INVALID_INDEX),
     geologist_(Widelands::INVALID_INDEX),
     soldier_(Widelands::INVALID_INDEX),
     ship_(Widelands::INVALID_INDEX),
     ferry_(Widelands::INVALID_INDEX),
     port_(Widelands::INVALID_INDEX),
     ironore_(Widelands::INVALID_INDEX),
     rawlog_(Widelands::INVALID_INDEX),
     refinedlog_(Widelands::INVALID_INDEX),
     granite_(Widelands::INVALID_INDEX) {
	log("┏━ Loading %s:\n", name_.c_str());
	ScopedTimer timer("┗━ took: %ums");

	initializations_ = info.initializations;

	try {
		log("┃    Frontiers, flags and roads: ");
		load_frontiers_flags_roads(table);
		log("%ums\n", timer.ms_since_last_query());

		log("┃    Ships: ");
		load_ships(table, tribes);
		log("%ums\n", timer.ms_since_last_query());

		log("┃    Wares: ");
		load_wares(table, tribes);
		if (scenario_table != nullptr && scenario_table->has_key("wares_order")) {
			load_wares(*scenario_table, tribes);
		}
		log("%ums\n", timer.ms_since_last_query());

		log("┃    Immovables: ");
		load_immovables(table, tribes, world);
		log("%ums\n", timer.ms_since_last_query());

		log("┃    Workers: ");
		load_workers(table, tribes);
		if (scenario_table != nullptr && scenario_table->has_key("workers_order")) {
			load_workers(*scenario_table, tribes);
		}
		log("%ums\n", timer.ms_since_last_query());

		log("┃    Buildings: ");
		load_buildings(table, tribes);
		if (scenario_table != nullptr && scenario_table->has_key("buildings")) {
			load_buildings(*scenario_table, tribes);
		}
		log("%ums\n", timer.ms_since_last_query());

		log("┃    Finalizing: ");
		if (table.has_key<std::string>("toolbar")) {
			toolbar_image_set_.reset(new ToolbarImageset(*table.get_table("toolbar")));
		}
		finalize_loading(tribes);
		log("%ums\n", timer.ms_since_last_query());
	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}
}

void TribeDescr::load_frontiers_flags_roads(const LuaTable& table) {

	std::unique_ptr<LuaTable> items_table = table.get_table("roads");
	const auto load_roads = [&items_table](
	   const std::string& road_type, std::vector<std::string>* images) {
		std::vector<std::string> roads =
		   items_table->get_table(road_type)->array_entries<std::string>();
		for (const std::string& filename : roads) {
			if (g_fs->file_exists(filename)) {
				images->push_back(filename);
			} else {
				throw GameDataError(
				   "File '%s' for %s road texture doesn't exist", filename.c_str(), road_type.c_str());
			}
		}
		if (images->empty()) {
			throw GameDataError("Tribe has no %s roads.", road_type.c_str());
		}
	};

	// Add textures for roads/waterways.
	std::vector<std::string> road_images;

	load_roads("normal", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_normal_road_texture(g_gr->images().get(texture_path));
	}

	load_roads("busy", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_busy_road_texture(g_gr->images().get(texture_path));
	}

	load_roads("waterway", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_waterway_texture(g_gr->images().get(texture_path));
	}

	const auto load_bridge_if_present = [this](const LuaTable& animations_table,
	                                           Animation::Type animation_type, std::string s_dir,
	                                           std::string s_type, uint32_t* id) {
		if (animations_table.has_key("bridge_" + s_type + "_" + s_dir)) {
			std::unique_ptr<LuaTable> animation_table =
			   animations_table.get_table("bridge_" + s_type + "_" + s_dir);
			*id = g_gr->animations().load(name_ + std::string("_bridge_" + s_type + "_" + s_dir),
			                              *animation_table, animation_table->get_string("basename"),
			                              animation_type);
		}
	};
	// Frontier and flag animations can be a mix of file and spritesheet animations
	const auto load_animations = [this, load_bridge_if_present](
	   const LuaTable& animations_table, Animation::Type animation_type) {
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
		load_bridge_if_present(animations_table, animation_type, "e", "normal", &bridges_normal_.e);
		load_bridge_if_present(animations_table, animation_type, "se", "normal", &bridges_normal_.se);
		load_bridge_if_present(animations_table, animation_type, "sw", "normal", &bridges_normal_.sw);
		load_bridge_if_present(animations_table, animation_type, "e", "busy", &bridges_busy_.e);
		load_bridge_if_present(animations_table, animation_type, "se", "busy", &bridges_busy_.se);
		load_bridge_if_present(animations_table, animation_type, "sw", "busy", &bridges_busy_.sw);
	};

	if (table.has_key("animations")) {
		load_animations(*table.get_table("animations"), Animation::Type::kFiles);
	}
	if (table.has_key("spritesheets")) {
		load_animations(*table.get_table("spritesheets"), Animation::Type::kSpritesheet);
	}
}

void TribeDescr::load_ships(const LuaTable& table, Tribes& tribes) {
	const std::string shipname(table.get_string("ship"));
	try {
		ship_ = tribes.load_ship(shipname);
	} catch (const WException& e) {
		throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
	}
	ship_names_ = table.get_table("ship_names")->array_entries<std::string>();
}

void TribeDescr::load_wares(const LuaTable& table, Tribes& tribes) {
	std::unique_ptr<LuaTable> items_table = table.get_table("wares_order");

	for (const int key : items_table->keys<int>()) {
		std::vector<DescriptionIndex> column;
		std::vector<std::string> warenames =
		   items_table->get_table(key)->array_entries<std::string>();
		for (size_t rowindex = 0; rowindex < warenames.size(); ++rowindex) {
			try {
				DescriptionIndex wareindex = tribes.load_ware(warenames[rowindex]);
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

	if (table.has_key("ironore")) {
		ironore_ = add_special_ware(table.get_string("ironore"), tribes);
	}
	if (table.has_key("rawlog")) {
		rawlog_ = add_special_ware(table.get_string("rawlog"), tribes);
	}
	if (table.has_key("refinedlog")) {
		refinedlog_ = add_special_ware(table.get_string("refinedlog"), tribes);
	}
	if (table.has_key("granite")) {
		granite_ = add_special_ware(table.get_string("granite"), tribes);
	}

	// Verify that the preciousness has been set for all of the tribe's wares
	for (const DescriptionIndex wi : wares()) {
		if (get_ware_descr(wi)->ai_hints().preciousness(name()) == kInvalidWare) {
			throw GameDataError("The ware '%s' needs to define a preciousness for tribe '%s'",
			                    get_ware_descr(wi)->name().c_str(), name().c_str());
		}
	}
}

void TribeDescr::load_immovables(const LuaTable& table, Tribes& tribes, const World& world) {
	for (const std::string& immovablename :
	     table.get_table("immovables")->array_entries<std::string>()) {
		try {
			DescriptionIndex index = tribes.load_immovable(immovablename);
			if (immovables_.count(index) == 1) {
				throw GameDataError("Duplicate definition of immovable '%s'", immovablename.c_str());
			}
			immovables_.insert(index);
		} catch (const WException& e) {
			throw GameDataError("Failed adding immovable '%s': %s", immovablename.c_str(), e.what());
		}
	}

	std::unique_ptr<LuaTable> items_table = table.get_table("resource_indicators");
	for (std::string resource : items_table->keys<std::string>()) {
		ResourceIndicatorList resis;
		std::unique_ptr<LuaTable> tbl = items_table->get_table(resource);
		const std::set<int> keys = tbl->keys<int>();
		for (int upper_limit : keys) {
			resis[upper_limit] = tribes.load_immovable(tbl->get_string(upper_limit));
		}
		if (resis.empty()) {
			throw GameDataError("Tribe has no indicators for resource %s.", resource.c_str());
		}
		resource_indicators_[resource] = resis;
	};

	// Verify the resource indicators
	for (DescriptionIndex resource_index = 0; resource_index < world.get_nr_resources();
	     resource_index++) {
		const ResourceDescription* res = world.get_resource(resource_index);
		if (res->detectable()) {
			// This function will throw an exception if this tribe doesn't have a high enough resource
			// indicator for this resource
			get_resource_indicator(res, res->max_amount());
		}
	}
	// For the "none" indicator
	get_resource_indicator(nullptr, 0);
}

void TribeDescr::load_workers(const LuaTable& table, Tribes& tribes) {
	std::unique_ptr<LuaTable> items_table = table.get_table("workers_order");

	for (const int key : items_table->keys<int>()) {
		std::vector<DescriptionIndex> column;
		for (const std::string& workername :
		     items_table->get_table(key)->array_entries<std::string>()) {
			add_worker(workername, column, tribes);
		}
		if (!column.empty()) {
			workers_order_.push_back(column);
		}
	}

	if (table.has_key("builder")) {
		builder_ = add_special_worker(table.get_string("builder"), tribes);
	}
	if (table.has_key("carrier")) {
		carrier_ = add_special_worker(table.get_string("carrier"), tribes);
	}
	if (table.has_key("carrier2")) {
		carrier2_ = add_special_worker(table.get_string("carrier2"), tribes);
	}
	if (table.has_key("geologist")) {
		geologist_ = add_special_worker(table.get_string("geologist"), tribes);
	}
	if (table.has_key("soldier")) {
		soldier_ = add_special_worker(table.get_string("soldier"), tribes);
	}
	if (table.has_key("ferry")) {
		ferry_ = add_special_worker(table.get_string("ferry"), tribes);
	}
}

void TribeDescr::load_buildings(const LuaTable& table, Tribes& tribes) {
	for (const std::string& buildingname :
	     table.get_table("buildings")->array_entries<std::string>()) {
		add_building(buildingname, tribes);
	}

	if (table.has_key("port")) {
		port_ = add_special_building(table.get_string("port"), tribes);
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
DescriptionIndex TribeDescr::ferry() const {
	assert(tribes_.worker_exists(ferry_));
	return ferry_;
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

uint32_t TribeDescr::bridge_animation(uint8_t dir, bool busy) const {
	switch (dir) {
	case WALK_E:
		return (busy ? bridges_busy_ : bridges_normal_).e;
	case WALK_SE:
		return (busy ? bridges_busy_ : bridges_normal_).se;
	case WALK_SW:
		return (busy ? bridges_busy_ : bridges_normal_).sw;
	default:
		NEVER_HERE();
	}
}

uint32_t TribeDescr::bridge_height() const {
	return bridge_height_;
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

void TribeDescr::add_building(const std::string& buildingname, Tribes& tribes) {
	try {
		DescriptionIndex index = tribes.load_building(buildingname);
		if (has_building(index)) {
			throw GameDataError("Duplicate definition of building '%s'", buildingname.c_str());
		}
		buildings_.push_back(index);

		const BuildingDescr* building_descr = get_building_descr(index);

		// Register at enhanced building
		const DescriptionIndex& enhancement = building_descr->enhancement();
		if (enhancement != INVALID_INDEX) {
			tribes.get_mutable_building_descr(enhancement)->set_enhanced_from(index);
		}

		// Register trainigsites
		if (building_descr->type() == MapObjectType::TRAININGSITE) {
			trainingsites_.push_back(index);
		}

		// Register construction materials
		for (const auto& build_cost : building_descr->buildcost()) {
			if (!is_construction_material(build_cost.first)) {
				construction_materials_.insert(build_cost.first);
			}
		}
		for (const auto& enhancement_cost : building_descr->enhancement_cost()) {
			if (!is_construction_material(enhancement_cost.first)) {
				construction_materials_.insert(enhancement_cost.first);
			}
		}
	} catch (const WException& e) {
		throw GameDataError("Failed adding building '%s': %s", buildingname.c_str(), e.what());
	}
}

void TribeDescr::add_worker(const std::string& workername,
                            std::vector<DescriptionIndex>& workers_order_column,
                            Tribes& tribes) {
	try {
		DescriptionIndex workerindex = tribes.load_worker(workername);
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

void TribeDescr::add_worker(const std::string& workername, Tribes& tribes) {
	add_worker(workername, workers_order_.back(), tribes);
}

ToolbarImageset* TribeDescr::toolbar_image_set() const {
	return toolbar_image_set_.get();
}

/**
 * Helper functions
 */

DescriptionIndex TribeDescr::add_special_worker(const std::string& workername, Tribes& tribes) {
	try {
		DescriptionIndex worker = tribes.load_worker(workername);
		if (!has_worker(worker)) {
			throw GameDataError("This tribe doesn't have the worker '%s'", workername.c_str());
		}
		return worker;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special worker '%s': %s", workername.c_str(), e.what());
	}
}

DescriptionIndex TribeDescr::add_special_building(const std::string& buildingname, Tribes& tribes) {
	try {
		DescriptionIndex building = tribes.load_building(buildingname);
		if (!has_building(building)) {
			throw GameDataError("This tribe doesn't have the building '%s'", buildingname.c_str());
		}
		return building;
	} catch (const WException& e) {
		throw GameDataError(
		   "Failed adding special building '%s': %s", buildingname.c_str(), e.what());
	}
}
DescriptionIndex TribeDescr::add_special_ware(const std::string& warename, Tribes& tribes) {
	try {
		DescriptionIndex ware = tribes.load_ware(warename);
		if (!has_ware(ware)) {
			throw GameDataError("This tribe doesn't have the ware '%s'", warename.c_str());
		}
		return ware;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special ware '%s': %s", warename.c_str(), e.what());
	}
}

void TribeDescr::finalize_loading(Tribes& tribes) {
	// Validate special units
	if (builder_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'builder' not defined");
	}
	if (carrier_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'carrier' not defined");
	}
	if (carrier2_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'carrier2' not defined");
	}
	if (geologist_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'geologist' not defined");
	}
	if (soldier_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'soldier' not defined");
	}
	if (ferry_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'ferry' not defined");
	}
	if (port_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special building 'port' not defined");
	}
	if (ironore_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special ware 'ironore' not defined");
	}
	if (rawlog_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special ware 'rawlog' not defined");
	}
	if (refinedlog_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special ware 'refinedlog' not defined");
	}
	if (granite_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special ware 'granite' not defined");
	}
	if (ship_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special unit 'ship' not defined");
	}

	// Calculate building properties that have circular dependencies
	for (DescriptionIndex i : buildings_) {
		BuildingDescr* building_descr = tribes.get_mutable_building_descr(i);
		assert(building_descr != nullptr);

		// Add consumers and producers to wares.
		if (upcast(ProductionSiteDescr, de, building_descr)) {
			for (const auto& ware_amount : de->input_wares()) {
				assert(has_ware(ware_amount.first));
				tribes.get_mutable_ware_descr(ware_amount.first)->add_consumer(i);
			}
			for (const DescriptionIndex& wareindex : de->output_ware_types()) {
				assert(has_ware(wareindex));
				tribes.get_mutable_ware_descr(wareindex)->add_producer(i);
			}
			for (const auto& job : de->working_positions()) {
				assert(has_worker(job.first));
				tribes.get_mutable_worker_descr(job.first)->add_employer(i);
			}

			// Check that all workarea overlap hints are valid
			for (const auto& pair : de->get_highlight_overlapping_workarea_for()) {
				const DescriptionIndex di = safe_building_index(pair.first);
				if (upcast(const ProductionSiteDescr, p, get_building_descr(di))) {
					if (!p->workarea_info().empty()) {
						continue;
					}
					throw GameDataError("Productionsite %s will inform about conflicting building %s "
					                    "which doesn’t have a workarea",
					                    de->name().c_str(), pair.first.c_str());
				}
				throw GameDataError("Productionsite %s will inform about conflicting building %s which "
				                    "is not a productionsite",
				                    de->name().c_str(), pair.first.c_str());
			}
		}
	}

	calculate_trainingsites_proportions(tribes);
}

// Set default trainingsites proportions for AI. Make sure that we get a sum of ca. 100
void TribeDescr::calculate_trainingsites_proportions(Tribes& tribes) {
	unsigned int trainingsites_without_percent = 0;
	int used_percent = 0;
	std::vector<BuildingDescr*> traingsites_with_percent;
	for (const DescriptionIndex& index : trainingsites()) {
		BuildingDescr* descr = tribes.get_mutable_building_descr(index);
		if (descr->hints().trainingsites_max_percent() == 0) {
			++trainingsites_without_percent;
		} else {
			used_percent += descr->hints().trainingsites_max_percent();
			traingsites_with_percent.push_back(descr);
		}
	}

	// Adjust used_percent if we don't have at least 5% for each remaining trainingsite
	const float limit = 100 - trainingsites_without_percent * 5;
	if (used_percent > limit) {
		const int deductme = (used_percent - limit) / traingsites_with_percent.size();
		used_percent = 0;
		for (BuildingDescr* descr : traingsites_with_percent) {
			descr->set_hints_trainingsites_max_percent(descr->hints().trainingsites_max_percent() -
			                                           deductme);
			used_percent += descr->hints().trainingsites_max_percent();
		}
	}

	// Now adjust for trainingsites that didn't have their max_percent set
	if (trainingsites_without_percent > 0) {
		int percent_to_use = std::ceil((100 - used_percent) / trainingsites_without_percent);
		// We sometimes get below 100% in spite of the ceil call above.
		// A total sum a bit above 100% is fine though, so we increment until it's big enough.
		while ((used_percent + percent_to_use * trainingsites_without_percent) < 100) {
			++percent_to_use;
		}
		if (percent_to_use < 1) {
			throw GameDataError(
			   "%s: Training sites without predefined proportions add up to < 1%% and "
			   "will never be built: %d",
			   name().c_str(), used_percent);
		}
		for (const DescriptionIndex& index : trainingsites()) {
			BuildingDescr* descr = tribes.get_mutable_building_descr(index);
			if (descr->hints().trainingsites_max_percent() == 0) {
				descr->set_hints_trainingsites_max_percent(percent_to_use);
				used_percent += percent_to_use;
			}
		}
	}
	if (used_percent < 100) {
		throw GameDataError("%s: Final training sites proportions add up to < 100%%: %d",
		                    name().c_str(), used_percent);
	}
}

}  // namespace Widelands
