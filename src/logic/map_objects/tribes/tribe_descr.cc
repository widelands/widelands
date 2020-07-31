/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include <memory>

#include "base/i18n.h"
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
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/world.h"
#include "scripting/lua_table.h"

namespace {

// Recursively get attributes for world immovable growth cycle
void walk_world_immovables(
   Widelands::DescriptionIndex index,
   const Widelands::World& world,
   std::set<Widelands::DescriptionIndex>* walked_immovables,
   const std::set<Widelands::MapObjectDescr::AttributeIndex>& needed_attributes,
   std::set<std::string>* deduced_immovables,
   std::set<std::string>* deduced_bobs) {
	// Protect against endless recursion
	if (walked_immovables->count(index) == 1) {
		return;
	}
	walked_immovables->insert(index);

	// Insert this immovable's attributes
	const Widelands::ImmovableDescr* immovable_descr = world.get_immovable_descr(index);
	for (const Widelands::MapObjectDescr::AttributeIndex id : immovable_descr->attributes()) {
		if (needed_attributes.count(id) == 1) {
			deduced_immovables->insert(immovable_descr->name());
		}
	}

	// Check immovables that this immovable can turn into
	for (const auto& imm_becomes : immovable_descr->becomes()) {
		switch (imm_becomes.first) {
		case Widelands::MapObjectType::BOB:
			deduced_bobs->insert(imm_becomes.second);
			// Bobs don't transform further
			return;
		case Widelands::MapObjectType::IMMOVABLE: {
			const Widelands::DescriptionIndex becomes_index =
			   world.get_immovable_index(imm_becomes.second);
			assert(becomes_index != Widelands::INVALID_INDEX);
			walk_world_immovables(becomes_index, world, walked_immovables, needed_attributes,
			                      deduced_immovables, deduced_bobs);
		} break;
		default:
			NEVER_HERE();
		}
	}
}

// Recursively get attributes for tribe immovable growth cycle
void walk_tribe_immovables(
   Widelands::DescriptionIndex index,
   const Widelands::TribeDescr& tribe,
   std::set<Widelands::DescriptionIndex>* walked_immovables,
   const std::set<Widelands::MapObjectDescr::AttributeIndex>& needed_attributes,
   std::set<std::string>* deduced_immovables,
   std::set<std::string>* deduced_bobs) {
	// Protect against endless recursion
	if (walked_immovables->count(index) == 1) {
		return;
	}
	walked_immovables->insert(index);

	// Insert this immovable's attributes
	const Widelands::ImmovableDescr* immovable_descr = tribe.get_immovable_descr(index);
	for (const Widelands::MapObjectDescr::AttributeIndex id : immovable_descr->attributes()) {
		if (needed_attributes.count(id) == 1) {
			deduced_immovables->insert(immovable_descr->name());
		}
	}

	// Check immovables that this immovable can turn into
	for (const auto& imm_becomes : immovable_descr->becomes()) {
		switch (imm_becomes.first) {
		case Widelands::MapObjectType::BOB:
			deduced_bobs->insert(imm_becomes.second);
			// Bobs don't transform further
			return;
		case Widelands::MapObjectType::IMMOVABLE: {
			const Widelands::DescriptionIndex becomes_index =
			   tribe.immovable_index(imm_becomes.second);
			assert(becomes_index != Widelands::INVALID_INDEX);
			walk_tribe_immovables(becomes_index, tribe, walked_immovables, needed_attributes,
			                      deduced_immovables, deduced_bobs);
		} break;
		default:
			NEVER_HERE();
		}
	}
}
}  // namespace

namespace Widelands {

/**
 * The contents of 'table' are documented in
 * /data/tribes/atlanteans.lua
 */
TribeDescr::TribeDescr(const LuaTable& table,
                       const Widelands::TribeBasicInfo& info,
                       const World& world,
                       const Tribes& init_tribes)
   : name_(table.get_string("name")),
     descname_(info.descname),
     tribes_(init_tribes),
     bridge_height_(table.get_int("bridge_height")) {

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
		load_roads("waterway", &waterway_paths_);

		const auto load_bridge_if_present = [this](const LuaTable& animations_table,
		                                           const std::string& animation_directory,
		                                           Animation::Type animation_type, std::string s_dir,
		                                           std::string s_type, uint32_t* id) {
			const std::string directional_name("bridge_" + s_type + "_" + s_dir);
			if (animations_table.has_key(directional_name)) {
				std::unique_ptr<LuaTable> animation_table =
				   animations_table.get_table(directional_name);
				*id = g_gr->animations().load(name_ + std::string("_") + directional_name,
				                              *animation_table, directional_name, animation_directory,
				                              animation_type);
			}
		};
		// Frontier and flag animations can be a mix of file and spritesheet animations
		const auto load_animations = [this, load_bridge_if_present](
		                                const LuaTable& animations_table,
		                                const std::string& animation_directory,
		                                Animation::Type animation_type) {
			if (animations_table.has_key("frontier")) {
				std::unique_ptr<LuaTable> animation_table = animations_table.get_table("frontier");
				frontier_animation_id_ =
				   g_gr->animations().load(name_ + std::string("_frontier"), *animation_table,
				                           "frontier", animation_directory, animation_type);
			}
			if (animations_table.has_key("flag")) {
				std::unique_ptr<LuaTable> animation_table = animations_table.get_table("flag");
				flag_animation_id_ =
				   g_gr->animations().load(name_ + std::string("_flag"), *animation_table, "flag",
				                           animation_directory, animation_type);
			}
			load_bridge_if_present(animations_table, animation_directory, animation_type, "e",
			                       "normal", &bridges_normal_.e);
			load_bridge_if_present(animations_table, animation_directory, animation_type, "se",
			                       "normal", &bridges_normal_.se);
			load_bridge_if_present(animations_table, animation_directory, animation_type, "sw",
			                       "normal", &bridges_normal_.sw);
			load_bridge_if_present(
			   animations_table, animation_directory, animation_type, "e", "busy", &bridges_busy_.e);
			load_bridge_if_present(
			   animations_table, animation_directory, animation_type, "se", "busy", &bridges_busy_.se);
			load_bridge_if_present(
			   animations_table, animation_directory, animation_type, "sw", "busy", &bridges_busy_.sw);
		};

		std::string animation_directory = table.get_string("animation_directory");
		if (table.has_key("animations")) {
			load_animations(
			   *table.get_table("animations"), animation_directory, Animation::Type::kFiles);
		}
		if (table.has_key("spritesheets")) {
			load_animations(
			   *table.get_table("spritesheets"), animation_directory, Animation::Type::kSpritesheet);
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
		for (const std::string& resource : items_table->keys<std::string>()) {
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
		ferry_ = add_special_worker(table.get_string("ferry"));

		const std::string shipname = table.get_string("ship");
		try {
			ship_ = tribes_.safe_ship_index(shipname);
		} catch (const WException& e) {
			throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
		}

		port_ = add_special_building(table.get_string("port"));

		if (table.has_key<std::string>("toolbar")) {
			toolbar_image_set_.reset(new ToolbarImageset(*table.get_table("toolbar")));
		}
	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}

	process_productionsites(world);
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

const std::vector<std::string>& TribeDescr::normal_road_paths() const {
	return normal_road_paths_;
}

const std::vector<std::string>& TribeDescr::busy_road_paths() const {
	return busy_road_paths_;
}

const std::vector<std::string>& TribeDescr::waterway_paths() const {
	return waterway_paths_;
}

void TribeDescr::add_normal_road_texture(const Image* texture) {
	road_textures_.add_normal_road_texture(texture);
}

void TribeDescr::add_busy_road_texture(const Image* texture) {
	road_textures_.add_busy_road_texture(texture);
}

void TribeDescr::add_waterway_texture(const Image* texture) {
	road_textures_.add_waterway_texture(texture);
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

void TribeDescr::process_productionsites(const World& world) {
	// Get a list of productionsites - we will need to iterate them more than once
	std::set<ProductionSiteDescr*> productionsites;
	for (const DescriptionIndex index : buildings()) {
		BuildingDescr* building = tribes_.get_mutable_building_descr(index);
		ProductionSiteDescr* productionsite = dynamic_cast<ProductionSiteDescr*>(building);
		if (productionsite != nullptr) {
			productionsites.insert(productionsite);
		}
	}

	const DescriptionMaintainer<ImmovableDescr>& world_immovables = world.immovables();

	// Find all attributes that we need to collect from map
	std::set<MapObjectDescr::AttributeIndex> needed_attributes;
	for (ProductionSiteDescr* prod : productionsites) {
		for (const auto& attribinfo : prod->collected_attributes()) {
			const MapObjectType mapobjecttype = attribinfo.first;
			const MapObjectDescr::AttributeIndex attribute_id = attribinfo.second;
			needed_attributes.insert(attribute_id);

			// Add collected entities
			switch (mapobjecttype) {
			case MapObjectType::IMMOVABLE: {
				for (DescriptionIndex i = 0; i < world_immovables.size(); ++i) {
					const ImmovableDescr& immovable_descr = world_immovables.get(i);
					if (immovable_descr.has_attribute(attribute_id)) {
						prod->add_collected_immovable(immovable_descr.name());
					}
				}
				for (const DescriptionIndex i : immovables()) {
					const ImmovableDescr& immovable_descr = *get_immovable_descr(i);
					if (immovable_descr.has_attribute(attribute_id)) {
						prod->add_collected_immovable(immovable_descr.name());
					}
				}
			} break;
			case MapObjectType::BOB: {
				// We only support critters here, because no other bobs are collected so far
				for (DescriptionIndex i = 0; i < world.get_nr_critters(); ++i) {
					const CritterDescr* critter = world.get_critter_descr(i);
					if (critter->has_attribute(attribute_id)) {
						prod->add_collected_bob(critter->name());
					}
				}
			} break;
			default:
				NEVER_HERE();
			}
		}
	}

	// Register who creates which entities
	std::map<std::string, std::set<ProductionSiteDescr*>> creators;
	auto add_creator = [&creators](const std::string& item, ProductionSiteDescr* productionsite) {
		if (creators.count(item) != 1) {
			creators[item] = {productionsite};
		} else {
			creators[item].insert(productionsite);
		}
	};

	// Register who collects which entities
	std::map<std::string, std::set<ProductionSiteDescr*>> collectors;
	auto add_collector = [&collectors](
	                        const std::string& item, ProductionSiteDescr* productionsite) {
		if (collectors.count(item) != 1) {
			collectors[item] = {productionsite};
		} else {
			collectors[item].insert(productionsite);
		}
	};

	for (ProductionSiteDescr* prod : productionsites) {
		// Add bobs that are created directly
		for (const std::string& bobname : prod->created_bobs()) {
			const CritterDescr* critter = world.get_critter_descr(bobname);
			if (critter == nullptr) {
				if (worker_index(bobname) == Widelands::INVALID_INDEX) {
					throw GameDataError(
					   "Productionsite '%s' has unknown bob '%s' in production or worker program",
					   prod->name().c_str(), bobname.c_str());
				}
			}
			add_creator(bobname, prod);
		}

		// Get attributes and bobs from transformations
		std::set<std::string> deduced_bobs;
		std::set<std::string> deduced_immovables;
		// Remember where we walked in case of circular dependencies
		std::set<DescriptionIndex> walked_world_immovables;
		std::set<DescriptionIndex> walked_tribe_immovables;

		for (const auto& attribinfo : prod->created_attributes()) {
			const MapObjectType mapobjecttype = attribinfo.first;
			const MapObjectDescr::AttributeIndex attribute_id = attribinfo.second;
			if (mapobjecttype != MapObjectType::IMMOVABLE) {
				continue;
			}
			for (DescriptionIndex i = 0; i < world_immovables.size(); ++i) {
				const ImmovableDescr& immovable_descr = world_immovables.get(i);
				if (immovable_descr.has_attribute(attribute_id)) {
					walk_world_immovables(i, world, &walked_world_immovables, needed_attributes,
					                      &deduced_immovables, &deduced_bobs);
					if (needed_attributes.count(attribute_id) == 1) {
						prod->add_created_immovable(immovable_descr.name());
						add_creator(immovable_descr.name(), prod);
					}
				}
			}
			for (const DescriptionIndex i : immovables()) {
				const ImmovableDescr& immovable_descr = *get_immovable_descr(i);
				if (immovable_descr.has_attribute(attribute_id)) {
					walk_tribe_immovables(i, *this, &walked_tribe_immovables, needed_attributes,
					                      &deduced_immovables, &deduced_bobs);
					if (needed_attributes.count(attribute_id) == 1) {
						prod->add_created_immovable(immovable_descr.name());
						add_creator(immovable_descr.name(), prod);
					}
				}
			}

			// We're done with this site's attributes, let's get some memory back
			prod->clear_attributes();
		}

		// Add deduced bobs & immovables
		for (const std::string& bob_name : deduced_bobs) {
			prod->add_created_bob(bob_name);
			add_creator(bob_name, prod);
		}
		for (const std::string& immovable_name : deduced_immovables) {
			prod->add_created_immovable(immovable_name);
			add_creator(immovable_name, prod);
		}

		// Register remaining creators and collectors
		for (const std::string& resource : prod->created_resources()) {
			add_creator(resource, prod);
		}
		for (const std::string& resource : prod->collected_resources()) {
			add_collector(resource, prod);
		}
		for (const std::string& bob : prod->collected_bobs()) {
			add_collector(bob, prod);
		}
		for (const std::string& immovable : prod->collected_immovables()) {
			add_collector(immovable, prod);
		}
	}

	// Calculate workarea overlaps + AI info
	for (ProductionSiteDescr* prod : productionsites) {
		// Sites that create any immovables should not overlap each other
		if (!prod->created_immovables().empty()) {
			for (const ProductionSiteDescr* other_prod : productionsites) {
				if (!other_prod->created_immovables().empty()) {
					prod->add_competing_productionsite(other_prod->name());
				}
			}
		}
		// Sites that create any resources should not overlap each other
		if (!prod->created_resources().empty()) {
			for (const ProductionSiteDescr* other_prod : productionsites) {
				if (!other_prod->created_resources().empty()) {
					prod->add_competing_productionsite(other_prod->name());
				}
			}
		}

		// Sites that create a bob should not overlap sites that create the same bob
		for (const std::string& item : prod->created_bobs()) {
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					prod->add_competing_productionsite(creator->name());
					creator->add_competing_productionsite(prod->name());
				}
			}
		}

		for (const std::string& item : prod->collected_immovables()) {
			// Sites that collect immovables and sites of other types that create immovables for them
			// should overlap each other
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect immovables should not overlap sites that collect the same immovable
			if (collectors.count(item)) {
				for (const ProductionSiteDescr* collector : collectors.at(item)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
		for (const std::string& item : prod->collected_bobs()) {
			// Sites that collect bobs and sites of other types that create bobs for them should
			// overlap each other
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect bobs should not overlap sites that collect the same bob
			if (collectors.count(item)) {
				for (const ProductionSiteDescr* collector : collectors.at(item)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
		for (const std::string& item : prod->collected_resources()) {
			// Sites that collect resources and sites of other types that create resources for them
			// should overlap each other
			if (creators.count(item)) {
				for (ProductionSiteDescr* creator : creators.at(item)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect resources should not overlap sites that collect the same resource
			if (collectors.count(item)) {
				for (const ProductionSiteDescr* collector : collectors.at(item)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
	}
}
}  // namespace Widelands
