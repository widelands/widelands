/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
#include "base/log.h"
#include "base/scoped_timer.h"
#include "base/wexception.h"
#include "graphic/animation/animation_manager.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/immovable_program.h"
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
#include "scripting/lua_table.h"
#include "ui_basic/note_loading_message.h"

namespace {

// Recursively get attributes for immovable growth cycle
void walk_immovables(Widelands::DescriptionIndex index,
                     const Widelands::Descriptions& descriptions,
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
	const Widelands::ImmovableDescr* immovable_descr = descriptions.get_immovable_descr(index);
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
			   descriptions.immovable_index(imm_becomes.second);
			assert(becomes_index != Widelands::INVALID_INDEX);
			walk_immovables(becomes_index, descriptions, walked_immovables, needed_attributes,
			                deduced_immovables, deduced_bobs);
		} break;
		default:
			NEVER_HERE();
		}
	}
}

}  // namespace

namespace Widelands {

void TribeDescr::load_helptexts(Widelands::MapObjectDescr* descr, const LuaTable& table) {
	std::map<std::string, std::string> helptexts;
	if (table.has_key("helptexts")) {
		std::unique_ptr<LuaTable> helptext_table = table.get_table("helptexts");
		for (const std::string& category_key : helptext_table->keys<std::string>()) {
			LuaTable::DataType datatype = helptext_table->get_datatype(category_key);
			switch (datatype) {
			case LuaTable::DataType::kString: {
				helptexts[category_key] = helptext_table->get_string(category_key);
			} break;
			case LuaTable::DataType::kTable: {
				// Concatenate text from entries with the localized sentence joiner
				std::unique_ptr<LuaTable> category_table = helptext_table->get_table(category_key);
				std::set<int> helptext_keys = category_table->keys<int>();
				if (!helptext_keys.empty()) {
					auto it = helptext_keys.begin();
					std::string helptext = category_table->get_string(*it);
					++it;
					for (; it != helptext_keys.end(); ++it) {
						helptext = i18n::join_sentences(helptext, category_table->get_string(*it));
					}
					helptexts[category_key] = helptext;
				} else {
					log_warn("Empty helptext defined for '%s'", descr->name().c_str());
				}
			} break;
			default:
				log_warn("Wrong helptext data type for '%s', category '%s'. Expecting a table or a "
				         "string.",
				         descr->name().c_str(), category_key.c_str());
			}
		}
		if (helptexts.empty()) {
			log_warn("No helptext defined for '%s'", descr->name().c_str());
		}
	} else {
		log_warn("No helptext defined for '%s'", descr->name().c_str());
	}
	descr->set_helptexts(name(), helptexts);
}

/**
 * The contents of 'table' are documented in
 * /data/tribes/atlanteans.lua
 */
TribeDescr::TribeDescr(const Widelands::TribeBasicInfo& info,
                       Descriptions& descriptions,
                       const LuaTable& table,
                       const LuaTable* scenario_table)
   : name_(table.get_string("name")),
     descname_(info.descname),
     military_capacity_script_(table.has_key<std::string>("military_capacity_script") ?
                                  table.get_string("military_capacity_script") :
                                  ""),
     descriptions_(descriptions),
     bridge_height_(table.get_int("bridge_height")),
     builder_(Widelands::INVALID_INDEX),
     geologist_(Widelands::INVALID_INDEX),
     soldier_(Widelands::INVALID_INDEX),
     ship_(Widelands::INVALID_INDEX),
     ferry_(Widelands::INVALID_INDEX),
     port_(Widelands::INVALID_INDEX),
     scouts_house_(Widelands::INVALID_INDEX),
     basic_info_(info) {
	verb_log_info("┏━ Loading %s", name_.c_str());
	ScopedTimer timer("┗━ took %ums", true);

	if (military_capacity_script_.empty() || !g_fs->file_exists(military_capacity_script_)) {
		// TODO(GunChleoc): API compatibility - require after v 1.0
		log_warn("File '%s' for military_capacity_script for tribe '%s' does not exist",
		         military_capacity_script_.c_str(), name().c_str());
	}

	auto set_progress_message = [this](const std::string& str, int i) {
		Notifications::publish(UI::NoteLoadingMessage(
		   /** TRANSLATORS: Example: Loading Barbarians: Buildings (2/6) */
		   format(_("Loading %1%: %2% (%3%/%4%)"), descname(), str, i, 6)));
	};

	try {
		verb_log_info("┃    Ships");
		set_progress_message(_("Ships"), 1);
		load_ships(table, descriptions);

		verb_log_info("┃    Immovables");
		set_progress_message(_("Immovables"), 2);
		load_immovables(table, descriptions);

		verb_log_info("┃    Wares");
		set_progress_message(_("Wares"), 3);
		load_wares(table, descriptions);
		if (scenario_table != nullptr && scenario_table->has_key("wares_order")) {
			load_wares(*scenario_table, descriptions);
		}

		verb_log_info("┃    Workers");
		set_progress_message(_("Workers"), 4);
		load_workers(table, descriptions);
		if (scenario_table != nullptr && scenario_table->has_key("workers_order")) {
			load_workers(*scenario_table, descriptions);
		}

		verb_log_info("┃    Buildings");
		set_progress_message(_("Buildings"), 5);
		load_buildings(table, descriptions);
		if (scenario_table != nullptr && scenario_table->has_key("buildings")) {
			load_buildings(*scenario_table, descriptions);
		}

		set_progress_message(_("Finishing"), 6);

		verb_log_info("┃    Frontiers, flags and roads");
		load_frontiers_flags_roads(table);

		verb_log_info("┃    Finalizing");
		if (table.has_key<std::string>("toolbar")) {
			toolbar_image_set_.reset(new ToolbarImageset(*table.get_table("toolbar")));
		}

		if (table.has_key("fastplace")) {
			std::unique_ptr<LuaTable> fp = table.get_table("fastplace");
			for (const std::string& key : fp->keys<std::string>()) {
				const std::string& val = fp->get_string(key);
				if (has_building(building_index(val))) {
					fastplace_defaults_.emplace(key, val);
				} else {
					log_warn(
					   "fastplace: tribe %s does not use building '%s'", name().c_str(), val.c_str());
				}
			}
		}

		// TODO(Nordfriese): Require these strings after v1.1
		auto load_productionsite_string = [this, &table](std::string& target, const std::string& key,
		                                                 const std::string& default_value) {
			if (table.has_key(key)) {
				target = table.get_string(key);
			} else {
				log_warn(
				   "Tribe '%s' defines no productionsite string '%s'", name().c_str(), key.c_str());
				target = default_value;
			}
		};
		load_productionsite_string(
		   productionsite_worker_missing_, "productionsite_worker_missing",
		   /** TRANSLATORS: Productivity label on a building if there is 1 worker missing */
		   _("Worker missing"));
		load_productionsite_string(
		   productionsite_workers_missing_, "productionsite_workers_missing",
		   /** TRANSLATORS: Productivity label on a building if there is more than 1 worker missing.
		      If you need plural forms here, please let us know. */
		   _("Workers missing"));
		load_productionsite_string(
		   productionsite_worker_coming_, "productionsite_worker_coming",
		   /** TRANSLATORS: Productivity label on a building if there is 1 worker coming */
		   _("Worker is coming"));
		load_productionsite_string(
		   productionsite_workers_coming_, "productionsite_workers_coming",
		   /** TRANSLATORS: Productivity label on a building if there is more than 1 worker coming. If
		      you need plural forms here, please let us know. */
		   _("Workers are coming"));

		std::unique_ptr<LuaTable> collectors_points_table =
		   table.get_table("collectors_points_table");
		for (int key : collectors_points_table->keys<int>()) {
			std::unique_ptr<LuaTable> t = collectors_points_table->get_table(key);
			const std::string ware = t->get_string("ware");
			if (!has_ware(descriptions.safe_ware_index(ware))) {
				throw GameDataError("Collectors: Tribe does not use ware '%s'", ware.c_str());
			}
			collectors_points_table_.push_back(std::make_pair(ware, t->get_int("points")));
		}
	} catch (const GameDataError& e) {
		throw GameDataError("tribe %s: %s", name_.c_str(), e.what());
	}
}

void TribeDescr::load_frontiers_flags_roads(const LuaTable& table) {

	std::unique_ptr<LuaTable> items_table = table.get_table("roads");
	const auto load_roads = [&items_table](
	                           const std::string& road_type, std::vector<std::string>* images) {
		images->clear();
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
	// Note: Road and flag texures are loaded in "graphic/build_texture_atlas.h"
	std::vector<std::string> road_images;

	load_roads("normal", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_normal_road_texture(g_image_cache->get(texture_path));
	}

	load_roads("busy", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_busy_road_texture(g_image_cache->get(texture_path));
	}

	load_roads("waterway", &road_images);
	for (const std::string& texture_path : road_images) {
		road_textures_.add_waterway_texture(g_image_cache->get(texture_path));
	}

	const auto load_bridge_if_present = [this](const LuaTable& animations_table,
	                                           const std::string& animation_directory,
	                                           Animation::Type animation_type,
	                                           const std::string& s_dir, const std::string& s_type,
	                                           uint32_t* id) {
		const std::string directional_name("bridge_" + s_type + "_" + s_dir);
		if (animations_table.has_key(directional_name)) {
			std::unique_ptr<LuaTable> animation_table = animations_table.get_table(directional_name);
			*id =
			   g_animation_manager->load(name_ + std::string("_") + directional_name, *animation_table,
			                             directional_name, animation_directory, animation_type);
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
			   g_animation_manager->load(name_ + std::string("_frontier"), *animation_table,
			                             "frontier", animation_directory, animation_type);
		}
		if (animations_table.has_key("flag")) {
			std::unique_ptr<LuaTable> animation_table = animations_table.get_table("flag");
			flag_animation_id_ =
			   g_animation_manager->load(name_ + std::string("_flag"), *animation_table, "flag",
			                             animation_directory, animation_type);
		}
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "e", "normal", &bridges_normal_.e);
		load_bridge_if_present(animations_table, animation_directory, animation_type, "se", "normal",
		                       &bridges_normal_.se);
		load_bridge_if_present(animations_table, animation_directory, animation_type, "sw", "normal",
		                       &bridges_normal_.sw);
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "e", "busy", &bridges_busy_.e);
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "se", "busy", &bridges_busy_.se);
		load_bridge_if_present(
		   animations_table, animation_directory, animation_type, "sw", "busy", &bridges_busy_.sw);
	};

	std::string animation_directory = table.get_string("animation_directory");
	if (table.has_key("animations")) {
		load_animations(*table.get_table("animations"), animation_directory, Animation::Type::kFiles);
	}
	if (table.has_key("spritesheets")) {
		load_animations(
		   *table.get_table("spritesheets"), animation_directory, Animation::Type::kSpritesheet);
	}
}

void TribeDescr::load_ships(const LuaTable& table, Descriptions& descriptions) {
	const std::string shipname(table.get_string("ship"));
	try {
		ship_ = descriptions.load_ship(shipname);
	} catch (const WException& e) {
		throw GameDataError("Failed adding ship '%s': %s", shipname.c_str(), e.what());
	}
}

void TribeDescr::load_wares(const LuaTable& table, Descriptions& descriptions) {
	std::unique_ptr<LuaTable> items_table = table.get_table("wares_order");

	for (const int column_key : items_table->keys<int>()) {
		std::vector<DescriptionIndex> column;
		std::unique_ptr<LuaTable> column_table = items_table->get_table(column_key);
		for (const int ware_key : column_table->keys<int>()) {
			std::unique_ptr<LuaTable> ware_table = column_table->get_table(ware_key);
			std::string ware_name = ware_table->get_string("name");
			try {
				DescriptionIndex wareindex = descriptions.load_ware(ware_name);
				if (has_ware(wareindex)) {
					throw GameDataError("Duplicate definition of ware");
				}

				// Set default_target_quantity (optional) and preciousness
				WareDescr* ware_descr = descriptions.get_mutable_ware_descr(wareindex);
				if (ware_table->has_key("default_target_quantity")) {
					ware_descr->set_default_target_quantity(
					   name(), ware_table->get_int("default_target_quantity"));
				}
				ware_descr->set_preciousness(name(), ware_table->get_int("preciousness"));

				// Add helptexts
				load_helptexts(ware_descr, *ware_table);

				// Add to tribe
				wares_.insert(wareindex);
				column.push_back(wareindex);
			} catch (const WException& e) {
				throw GameDataError("Failed adding ware '%s': %s", ware_name.c_str(), e.what());
			}
		}
		if (!column.empty()) {
			wares_order_.push_back(column);
		}
	}
}

void TribeDescr::load_immovables(const LuaTable& table, Descriptions& descriptions) {
	for (const auto& immovable_table :
	     table.get_table("immovables")->array_entries<std::unique_ptr<LuaTable>>()) {
		const std::string immovablename(immovable_table->get_string("name"));
		try {
			DescriptionIndex index = descriptions.load_immovable(immovablename);
			if (immovables_.count(index) == 1) {
				throw GameDataError("Duplicate definition of immovable '%s'", immovablename.c_str());
			}
			immovables_.insert(index);
			// Add helptext
			load_helptexts(descriptions.get_mutable_immovable_descr(index), *immovable_table);

		} catch (const WException& e) {
			throw GameDataError("Failed adding immovable '%s': %s", immovablename.c_str(), e.what());
		}
	}

	std::unique_ptr<LuaTable> items_table = table.get_table("resource_indicators");
	for (const std::string& resource : items_table->keys<std::string>()) {
		ResourceIndicatorList resis;
		std::unique_ptr<LuaTable> tbl = items_table->get_table(resource);
		const std::set<int> keys = tbl->keys<int>();
		for (int upper_limit : keys) {
			resis[upper_limit] = descriptions.load_immovable(tbl->get_string(upper_limit));
		}
		if (resis.empty()) {
			throw GameDataError("Tribe has no indicators for resource %s.", resource.c_str());
		}
		resource_indicators_[resource] = resis;
	}
}

void TribeDescr::load_workers(const LuaTable& table, Descriptions& descriptions) {
	std::unique_ptr<LuaTable> items_table = table.get_table("workers_order");

	for (const int column_key : items_table->keys<int>()) {
		std::vector<DescriptionIndex> column;
		std::unique_ptr<LuaTable> column_table = items_table->get_table(column_key);
		for (const int worker_key : column_table->keys<int>()) {
			std::unique_ptr<LuaTable> worker_table = column_table->get_table(worker_key);
			std::string workername = worker_table->get_string("name");
			try {
				DescriptionIndex workerindex = descriptions.load_worker(workername);
				if (has_worker(workerindex)) {
					throw GameDataError("Duplicate definition of worker");
				}

				// Set default_target_quantity and preciousness (both optional)
				WorkerDescr* worker_descr = descriptions.get_mutable_worker_descr(workerindex);
				if (worker_table->has_key("default_target_quantity")) {
					if (!worker_table->has_key("preciousness")) {
						throw GameDataError(
						   "It has a default_target_quantity but no preciousness for tribe '%s'",
						   name().c_str());
					}
					worker_descr->set_default_target_quantity(
					   worker_table->get_int("default_target_quantity"));
				}
				if (worker_table->has_key("preciousness")) {
					worker_descr->set_preciousness(name(), worker_table->get_int("preciousness"));
				}

				// Add helptexts
				load_helptexts(worker_descr, *worker_table);

				// Add to tribe
				workers_.insert(workerindex);
				column.push_back(workerindex);

				if (worker_descr->is_buildable() && worker_descr->buildcost().empty()) {
					worker_types_without_cost_.push_back(workerindex);
				}
			} catch (const WException& e) {
				throw GameDataError("Failed adding worker '%s': %s", workername.c_str(), e.what());
			}
		}
		if (!column.empty()) {
			workers_order_.push_back(column);
		}
	}

	if (table.has_key("carriers")) {
		for (const std::string& name : table.get_table("carriers")->array_entries<std::string>()) {
			carriers_.push_back(add_special_worker(name, descriptions));
		}
	} else {
		log_warn("Tribe %s: Specifying `carrier`/`carrier2` instead of `carriers` is deprecated",
		         name().c_str());
		if (table.has_key("carrier")) {
			carriers_.push_back(add_special_worker(table.get_string("carrier"), descriptions));
		}
		if (table.has_key("carrier2")) {
			carriers_.push_back(add_special_worker(table.get_string("carrier2"), descriptions));
		}
	}

	if (table.has_key("builder")) {
		builder_ = add_special_worker(table.get_string("builder"), descriptions);
	}
	if (table.has_key("geologist")) {
		geologist_ = add_special_worker(table.get_string("geologist"), descriptions);
	}
	if (table.has_key("soldier")) {
		soldier_ = add_special_worker(table.get_string("soldier"), descriptions);
	}
	if (table.has_key("ferry")) {
		ferry_ = add_special_worker(table.get_string("ferry"), descriptions);
	}
}

void TribeDescr::load_buildings(const LuaTable& table, Descriptions& descriptions) {
	for (const auto& building_table :
	     table.get_table("buildings")->array_entries<std::unique_ptr<LuaTable>>()) {
		const std::string buildingname(building_table->get_string("name"));
		try {
			DescriptionIndex index = descriptions.load_building(buildingname);
			if (has_building(index)) {
				throw GameDataError("Duplicate definition of building '%s'", buildingname.c_str());
			}
			buildings_.insert(index);

			BuildingDescr* building_descr = descriptions.get_mutable_building_descr(index);

			// Add helptexts
			load_helptexts(building_descr, *building_table);

			// Register at enhanced building
			const DescriptionIndex& enhancement = building_descr->enhancement();
			if (enhancement != INVALID_INDEX) {
				descriptions.get_mutable_building_descr(enhancement)->set_enhanced_from(index);
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

	if (table.has_key("port")) {
		port_ = add_special_building(table.get_string("port"), descriptions);
	}
	if (table.has_key("scouts_house")) {
		scouts_house_ = add_special_building(table.get_string("scouts_house"), descriptions);
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
const std::string& TribeDescr::military_capacity_script() const {
	return military_capacity_script_;
}

size_t TribeDescr::get_nrwares() const {
	return wares_.size();
}
size_t TribeDescr::get_nrworkers() const {
	return workers_.size();
}

const std::set<DescriptionIndex>& TribeDescr::buildings() const {
	return buildings_;
}
const std::set<DescriptionIndex>& TribeDescr::wares() const {
	return wares_;
}
std::set<DescriptionIndex>& TribeDescr::mutable_wares() {
	return wares_;
}
std::set<DescriptionIndex>& TribeDescr::mutable_workers() {
	return workers_;
}
std::set<DescriptionIndex>& TribeDescr::mutable_buildings() {
	return buildings_;
}
const std::set<DescriptionIndex>& TribeDescr::workers() const {
	return workers_;
}
const std::set<DescriptionIndex>& TribeDescr::immovables() const {
	return immovables_;
}
std::set<DescriptionIndex>& TribeDescr::mutable_immovables() {
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
	return descriptions_.building_index(buildingname);
}

DescriptionIndex TribeDescr::immovable_index(const std::string& immovablename) const {
	return descriptions_.immovable_index(immovablename);
}
DescriptionIndex TribeDescr::ware_index(const std::string& warename) const {
	return descriptions_.ware_index(warename);
}
DescriptionIndex TribeDescr::worker_index(const std::string& workername) const {
	return descriptions_.worker_index(workername);
}

DescriptionIndex TribeDescr::safe_building_index(const std::string& buildingname) const {
	return descriptions_.safe_building_index(buildingname);
}

DescriptionIndex TribeDescr::safe_ware_index(const std::string& warename) const {
	return descriptions_.safe_ware_index(warename);
}
DescriptionIndex TribeDescr::safe_worker_index(const std::string& workername) const {
	return descriptions_.safe_worker_index(workername);
}

WareDescr const* TribeDescr::get_ware_descr(const DescriptionIndex& index) const {
	return descriptions_.get_ware_descr(index);
}
WorkerDescr const* TribeDescr::get_worker_descr(const DescriptionIndex& index) const {
	return descriptions_.get_worker_descr(index);
}

BuildingDescr const* TribeDescr::get_building_descr(const DescriptionIndex& index) const {
	return descriptions_.get_building_descr(index);
}
ImmovableDescr const* TribeDescr::get_immovable_descr(const DescriptionIndex& index) const {
	return descriptions_.get_immovable_descr(index);
}

DescriptionIndex TribeDescr::builder() const {
	assert(descriptions_.worker_exists(builder_));
	return builder_;
}
DescriptionIndex TribeDescr::geologist() const {
	assert(descriptions_.worker_exists(geologist_));
	return geologist_;
}
DescriptionIndex TribeDescr::soldier() const {
	assert(descriptions_.worker_exists(soldier_));
	return soldier_;
}
DescriptionIndex TribeDescr::ship() const {
	assert(descriptions_.ship_exists(ship_));
	return ship_;
}
DescriptionIndex TribeDescr::port() const {
	assert(descriptions_.building_exists(port_));
	return port_;
}
DescriptionIndex TribeDescr::scouts_house() const {
	assert(descriptions_.building_exists(scouts_house_));
	return scouts_house_;
}
DescriptionIndex TribeDescr::ferry() const {
	assert(descriptions_.worker_exists(ferry_));
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
		}
		if (lowest < amount || resi.first < lowest) {
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

ToolbarImageset* TribeDescr::toolbar_image_set() const {
	return toolbar_image_set_.get();
}

/**
 * Helper functions
 */

DescriptionIndex TribeDescr::add_special_worker(const std::string& workername,
                                                Descriptions& descriptions) {
	try {
		DescriptionIndex worker = descriptions.load_worker(workername);
		if (!has_worker(worker)) {
			throw GameDataError("This tribe doesn't have the worker '%s'", workername.c_str());
		}
		return worker;
	} catch (const WException& e) {
		throw GameDataError("Failed adding special worker '%s': %s", workername.c_str(), e.what());
	}
}

DescriptionIndex TribeDescr::add_special_building(const std::string& buildingname,
                                                  Descriptions& descriptions) {
	try {
		DescriptionIndex building = descriptions.load_building(buildingname);
		if (!has_building(building)) {
			throw GameDataError("This tribe doesn't have the building '%s'", buildingname.c_str());
		}
		return building;
	} catch (const WException& e) {
		throw GameDataError(
		   "Failed adding special building '%s': %s", buildingname.c_str(), e.what());
	}
}

void TribeDescr::finalize_loading(Descriptions& descriptions) {
	// Validate special units
	if (builder_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special worker 'builder' not defined");
	}
	if (carriers_.size() < 2) {
		throw GameDataError(
		   "only %d type(s) of carriers defined", static_cast<int>(carriers_.size()));
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
	if (scouts_house_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special building 'scouts_house' not defined");
	}
	if (ship_ == Widelands::INVALID_INDEX) {
		throw GameDataError("special unit 'ship' not defined");
	}

	calculate_trainingsites_proportions(descriptions);
	process_productionsites(descriptions);
}

// Set default trainingsites proportions for AI. Make sure that we get a sum of ca. 100
void TribeDescr::calculate_trainingsites_proportions(const Descriptions& descriptions) {
	unsigned int trainingsites_without_percent = 0;
	int used_percent = 0;
	std::vector<BuildingDescr*> traingsites_with_percent;
	for (const DescriptionIndex& index : trainingsites()) {
		BuildingDescr* descr = descriptions.get_mutable_building_descr(index);
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
			   "%s: Training sites without predefined proportions add up to < 1%%, and "
			   "will never be built: %d",
			   name().c_str(), used_percent);
		}
		for (const DescriptionIndex& index : trainingsites()) {
			BuildingDescr* descr = descriptions.get_mutable_building_descr(index);
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

// Calculate building properties that have circular dependencies
void TribeDescr::process_productionsites(Descriptions& descriptions) {
	// Get a list of productionsites - we will need to iterate them more than once
	// The temporary use of pointers here is fine, because it doesn't affect the game state.
	std::set<ProductionSiteDescr*> productionsites;

	// Iterate buildings and update circular dependencies
	for (const DescriptionIndex index : buildings()) {
		BuildingDescr* building = descriptions_.get_mutable_building_descr(index);
		assert(building != nullptr);

		if (building->type() != MapObjectType::CONSTRUCTIONSITE &&
		    building->type() != MapObjectType::DISMANTLESITE) {
			building->set_owning_tribe(name());
		}

		// Calculate largest possible workarea radius
		for (const auto& pair : building->workarea_info()) {
			descriptions.increase_largest_workarea(pair.first);
		}

		if (building->get_built_over_immovable() != INVALID_INDEX) {
			buildings_built_over_immovables_.insert(building);
		}

		ProductionSiteDescr* productionsite = dynamic_cast<ProductionSiteDescr*>(building);
		if (productionsite != nullptr) {
			// List productionsite for use below
			productionsites.insert(productionsite);

			// Add consumers and producers to wares.
			for (const auto& ware_amount : productionsite->input_wares()) {
				assert(has_ware(ware_amount.first));
				descriptions.get_mutable_ware_descr(ware_amount.first)->add_consumer(index);
			}
			for (const DescriptionIndex& wareindex : productionsite->output_ware_types()) {
				assert(has_ware(wareindex));
				descriptions.get_mutable_ware_descr(wareindex)->add_producer(index);
			}
			for (const auto& job : productionsite->working_positions()) {
				assert(has_worker(job.first));
				descriptions.get_mutable_worker_descr(job.first)->add_employer(index);
			}
			// Resource info
			for (const auto& r : productionsite->collected_resources()) {
				used_resources_.insert(r.first);
			}
			for (const std::string& r : productionsite->created_resources()) {
				used_resources_.insert(r);
			}
		}
	}

	// Now that we have gathered all resources we can use, verify the resource indicators
	for (DescriptionIndex resource_index = 0; resource_index < descriptions.nr_resources();
	     resource_index++) {
		const ResourceDescription* res = descriptions.get_resource_descr(resource_index);
		if (res->detectable() && uses_resource(res->name())) {
			// This function will throw an exception if this tribe doesn't
			// have a high enough resource indicator for this resource
			get_resource_indicator(res, res->max_amount());
		}
	}
	// For the "none" indicator
	get_resource_indicator(nullptr, 0);

	const DescriptionMaintainer<ImmovableDescr>& all_immovables = descriptions.immovables();

	descriptions.postload_immovable_relations();

	// Find all attributes that we need to collect from map
	std::set<MapObjectDescr::AttributeIndex> needed_attributes;
	for (ProductionSiteDescr* prod : productionsites) {
		for (const auto& attribinfo : prod->needed_attributes()) {
			const MapObjectType mapobjecttype = attribinfo.first;
			const MapObjectDescr::AttributeIndex attribute_id = attribinfo.second;
			needed_attributes.insert(attribute_id);

			// Add collected entities
			switch (mapobjecttype) {
			case MapObjectType::IMMOVABLE: {
				for (DescriptionIndex i = 0; i < all_immovables.size(); ++i) {
					const ImmovableDescr& immovable_descr = all_immovables.get(i);
					if (immovable_descr.has_attribute(attribute_id)) {
						prod->add_needed_immovable(immovable_descr.name());
						if (prod->collected_attributes().count(attribinfo)) {
							prod->add_collected_immovable(immovable_descr.name());
						}
						const_cast<ImmovableDescr&>(immovable_descr)
						   .add_collected_by(descriptions, prod->name());
					}
				}
			} break;
			case MapObjectType::BOB: {
				// We only support critters here, because no other bobs are collected so far
				for (DescriptionIndex i = 0; i < descriptions.nr_critters(); ++i) {
					const CritterDescr* critter = descriptions.get_critter_descr(i);
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
			const CritterDescr* critter = descriptions.get_critter_descr(bobname);
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
		std::set<DescriptionIndex> walked_immovables;

		for (const auto& attribinfo : prod->created_attributes()) {
			const MapObjectType mapobjecttype = attribinfo.first;
			const MapObjectDescr::AttributeIndex attribute_id = attribinfo.second;
			if (mapobjecttype != MapObjectType::IMMOVABLE) {
				continue;
			}

			for (DescriptionIndex i = 0; i < all_immovables.size(); ++i) {
				const ImmovableDescr& immovable_descr = *get_immovable_descr(i);
				if (immovable_descr.has_attribute(attribute_id)) {
					walk_immovables(i, descriptions, &walked_immovables, needed_attributes,
					                &deduced_immovables, &deduced_bobs);
					if (needed_attributes.count(attribute_id) == 1) {
						prod->add_created_immovable(immovable_descr.name());
						add_creator(immovable_descr.name(), prod);
					}
				}
			}
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
		for (const auto& resource : prod->collected_resources()) {
			add_collector(resource.first, prod);
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

		for (const std::string& item : prod->needed_immovables()) {
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
		for (const auto& item : prod->collected_resources()) {
			// Sites that collect resources and sites of other types that create resources for them
			// should overlap each other
			if (creators.count(item.first)) {
				for (ProductionSiteDescr* creator : creators.at(item.first)) {
					if (creator != prod) {
						prod->add_supported_by_productionsite(creator->name());
						creator->add_supports_productionsite(prod->name());
					}
				}
			}
			// Sites that collect resources should not overlap sites that collect the same resource
			if (collectors.count(item.first)) {
				for (const ProductionSiteDescr* collector : collectors.at(item.first)) {
					prod->add_competing_productionsite(collector->name());
				}
			}
		}
	}
}
}  // namespace Widelands
