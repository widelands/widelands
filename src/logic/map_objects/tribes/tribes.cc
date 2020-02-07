/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#include "logic/map_objects/tribes/tribes.h"

#include <memory>

#include "base/wexception.h"
#include "graphic/graphic.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"

namespace Widelands {
Tribes::Tribes(LuaInterface* lua)
   : buildings_(new DescriptionMaintainer<BuildingDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     ships_(new DescriptionMaintainer<ShipDescr>()),
     wares_(new DescriptionMaintainer<WareDescr>()),
     workers_(new DescriptionMaintainer<WorkerDescr>()),
     tribes_(new DescriptionMaintainer<TribeDescr>()),
     legacy_lookup_table_(new TribesLegacyLookupTable()),
     largest_workarea_(0),
     scenario_tribes_(nullptr),
     lua_(lua) {

	// NOCOM create a test scenario that will load all tribes

	// Register tribe names
	for (const TribeBasicInfo& tribeinfo : Widelands::get_all_tribeinfos()) {
		register_object(tribeinfo.name, tribeinfo.script);
	}

	// Walk tribes directory and register objects
	register_directory("tribes", g_fs, false);
}

/* NOCOM replace
void Tribes::add_ferry_type(const LuaTable& table) {
i18n::Textdomain td("tribes");
workers_->add(new FerryDescr(
 pgettext_expr(table.get_string("msgctxt").c_str(), table.get_string("descname").c_str()),
 table, *this));
}
*/

Tribes::~Tribes() {
}

/* NOCOM replacement
void Tribes::add_custom_worker(const LuaTable& table) {
   const std::string tribename = table.get_string("tribename");
   if (Widelands::tribe_exists(tribename)) {
      TribeDescr* descr = tribes_->get_mutable(tribe_index(tribename));
      descr->add_worker(table.get_string("workername"));
   } else {
      throw GameDataError("The tribe '%s'' has no preload file.", tribename.c_str());
   }
} */

size_t Tribes::nrbuildings() const {
	return buildings_->size();
}

size_t Tribes::nrtribes() const {
	return tribes_->size();
}

size_t Tribes::nrwares() const {
	return wares_->size();
}

size_t Tribes::nrworkers() const {
	return workers_->size();
}

bool Tribes::ware_exists(const std::string& warename) const {
	return wares_->exists(warename) != nullptr;
}
bool Tribes::ware_exists(DescriptionIndex index) const {
	return wares_->get_mutable(index) != nullptr;
}
bool Tribes::worker_exists(const std::string& workername) const {
	return workers_->exists(workername) != nullptr;
}
bool Tribes::worker_exists(DescriptionIndex index) const {
	return workers_->get_mutable(index) != nullptr;
}
bool Tribes::building_exists(const std::string& buildingname) const {
	return buildings_->exists(buildingname) != nullptr;
}
bool Tribes::building_exists(DescriptionIndex index) const {
	return buildings_->get_mutable(index) != nullptr;
}
bool Tribes::immovable_exists(DescriptionIndex index) const {
	return immovables_->get_mutable(index) != nullptr;
}
bool Tribes::ship_exists(DescriptionIndex index) const {
	return ships_->get_mutable(index) != nullptr;
}
bool Tribes::tribe_exists(DescriptionIndex index) const {
	return tribes_->get_mutable(index) != nullptr;
}

DescriptionIndex Tribes::safe_building_index(const std::string& buildingname) const {
	const DescriptionIndex result =
	   building_index(legacy_lookup_table_.get()->lookup_building(buildingname));
	if (!building_exists(result)) {
		throw GameDataError("Unknown building type \"%s\"", buildingname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_immovable_index(const std::string& immovablename) const {
	const DescriptionIndex result =
	   immovable_index(legacy_lookup_table_.get()->lookup_immovable(immovablename));
	if (!immovable_exists(result)) {
		throw GameDataError("Unknown immovable type \"%s\"", immovablename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ship_index(const std::string& shipname) const {
	const DescriptionIndex result = ship_index(legacy_lookup_table_.get()->lookup_ship(shipname));
	if (!ship_exists(result)) {
		throw GameDataError("Unknown ship type \"%s\"", shipname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_tribe_index(const std::string& tribename) const {
	const DescriptionIndex result = tribe_index(tribename);
	if (!tribe_exists(result)) {
		throw GameDataError("Unknown tribe \"%s\"", tribename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ware_index(const std::string& warename) const {
	const DescriptionIndex result = ware_index(legacy_lookup_table_.get()->lookup_ware(warename));
	if (!ware_exists(result)) {
		throw GameDataError("Unknown ware type \"%s\"", warename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_worker_index(const std::string& workername) const {
	const DescriptionIndex result =
	   worker_index(legacy_lookup_table_.get()->lookup_worker(workername));
	if (!worker_exists(result)) {
		throw GameDataError("Unknown worker type \"%s\"", workername.c_str());
	}
	return result;
}

DescriptionIndex Tribes::building_index(const std::string& buildingname) const {
	return buildings_->get_index(buildingname);
}

DescriptionIndex Tribes::immovable_index(const std::string& immovablename) const {
	return immovables_->get_index(immovablename);
}

DescriptionIndex Tribes::ship_index(const std::string& shipname) const {
	return ships_->get_index(shipname);
}

DescriptionIndex Tribes::tribe_index(const std::string& tribename) const {
	return tribes_->get_index(tribename);
}

DescriptionIndex Tribes::ware_index(const std::string& warename) const {
	return wares_->get_index(warename);
}

DescriptionIndex Tribes::worker_index(const std::string& workername) const {
	return workers_->get_index(workername);
}

const BuildingDescr* Tribes::get_building_descr(DescriptionIndex buildingindex) const {
	return buildings_->get_mutable(buildingindex);
}

BuildingDescr* Tribes::get_mutable_building_descr(DescriptionIndex buildingindex) const {
	return buildings_->get_mutable(buildingindex);
}

const ImmovableDescr* Tribes::get_immovable_descr(DescriptionIndex immovableindex) const {
	return immovables_->get_mutable(immovableindex);
}

const ShipDescr* Tribes::get_ship_descr(DescriptionIndex shipindex) const {
	return ships_->get_mutable(shipindex);
}

const WareDescr* Tribes::get_ware_descr(DescriptionIndex wareindex) const {
	return wares_->get_mutable(wareindex);
}
WareDescr* Tribes::get_mutable_ware_descr(DescriptionIndex wareindex) const {
	return wares_->get_mutable(wareindex);
}

const WorkerDescr* Tribes::get_worker_descr(DescriptionIndex workerindex) const {
	return workers_->get_mutable(workerindex);
}
WorkerDescr* Tribes::get_mutable_worker_descr(DescriptionIndex workerindex) const {
	return workers_->get_mutable(workerindex);
}

const TribeDescr* Tribes::get_tribe_descr(DescriptionIndex tribeindex) const {
	return tribes_->get_mutable(tribeindex);
}

void Tribes::set_ware_type_has_demand_check(DescriptionIndex wareindex,
                                            const std::string& tribename) const {
	wares_->get_mutable(wareindex)->set_has_demand_check(tribename);
}

void Tribes::set_worker_type_has_demand_check(DescriptionIndex workerindex) const {
	workers_->get_mutable(workerindex)->set_has_demand_check();
}

// ************************ Loading *************************

void Tribes::register_scenario_tribes(FileSystem* filesystem) {
	// If the map is a scenario with custom tribe entites, load them.
	if (filesystem->file_exists("scripting/tribes")) {
		scenario_tribes_.reset(nullptr);
		if (filesystem->file_exists("scripting/tribes/init.lua")) {
			scenario_tribes_ = lua_->run_script("map:scripting/tribes/init.lua");
		}
		register_directory("scripting/tribes", filesystem, true);
	}
}

// Walk tribes directory and register objects
void Tribes::register_directory(const std::string& dirname,
                                FileSystem* filesystem,
                                bool is_scenario_tribe) {
	FilenameSet files = filesystem->list_directory(dirname);
	for (const std::string& file : files) {
		if (filesystem->is_directory(file)) {
			register_directory(file, filesystem, is_scenario_tribe);
		} else {
			if (strcmp(filesystem->fs_filename(file.c_str()), "register.lua") == 0) {
				if (is_scenario_tribe) {
					std::unique_ptr<LuaTable> names_table = lua_->run_script("map:" + file);
					for (const std::string& object_name : names_table->array_entries<std::string>()) {
						register_scenario_object(
						   filesystem, object_name, filesystem->fs_dirname(file) + "init.lua");
					}
				} else {
					std::unique_ptr<LuaTable> names_table = lua_->run_script(file);
					for (const std::string& object_name : names_table->array_entries<std::string>()) {
						register_object(object_name, filesystem->fs_dirname(file) + "init.lua");
					}
				}
			}
		}
	}
}

void Tribes::register_object(const std::string& name, const std::string& script_path) {
	if (registered_tribe_objects_.count(name) == 1) {
		throw GameDataError("Tribes::register_object: Attempt to register object\n"
		                    "   name: '%s'\n"
		                    "   script: '%s'\n"
		                    "but the object already exists",
		                    name.c_str(), script_path.c_str());
	}
	if (!g_fs->file_exists(script_path)) {
		throw GameDataError("Tribes::register_object: Attempt to register object\n"
		                    "   name: '%s'\n"
		                    "   script: '%s'\n"
		                    "but the script file does not exist",
		                    name.c_str(), script_path.c_str());
	}
	registered_tribe_objects_.insert(std::make_pair(name, script_path));
}

void Tribes::register_scenario_object(FileSystem* filesystem,
                                      const std::string& name,
                                      const std::string& script_path) {
	if (registered_scenario_objects_.count(name) == 1) {
		throw GameDataError("Tribes::register_object: Attempt to register scenario object\n"
		                    "   name: '%s'\n"
		                    "   script: '%s'\n"
		                    "but the object already exists",
		                    name.c_str(), script_path.c_str());
	}
	if (!filesystem->file_exists(script_path)) {
		throw GameDataError("Tribes::register_object: Attempt to register scenario object\n"
		                    "   name: '%s'\n"
		                    "   script: '%s'\n"
		                    "but the script file does not exist",
		                    name.c_str(), script_path.c_str());
	}
	registered_scenario_objects_.insert(std::make_pair(name, "map:" + script_path));
}

void Tribes::add_tribe_object_type(const LuaTable& table,
                                   EditorGameBase& egbase,
                                   MapObjectType type) {
	i18n::Textdomain td("tribes");
	const std::string& object_name = table.get_string("name");
	const std::string& msgctxt = table.get_string("msgctxt");
	const std::string& object_descname =
	   pgettext_expr(msgctxt.c_str(), table.get_string("descname").c_str());

	// Register as in progress
	tribe_objects_being_loaded_.insert(object_name);

	// Add
	switch (type) {
	case MapObjectType::CARRIER:
		workers_->add(new CarrierDescr(object_descname, table, *this));
		break;
	case MapObjectType::CONSTRUCTIONSITE:
		buildings_->add(new ConstructionSiteDescr(object_descname, table, *this));
		break;
	case MapObjectType::DISMANTLESITE:
		buildings_->add(new DismantleSiteDescr(object_descname, table, *this));
		break;
		// NOCOM ferry
	case MapObjectType::IMMOVABLE:
		immovables_->add(new ImmovableDescr(object_descname, table, *this));
		break;
	case MapObjectType::MARKET:
		buildings_->add(new MarketDescr(object_descname, table, *this));
		break;
	case MapObjectType::MILITARYSITE:
		buildings_->add(new MilitarySiteDescr(object_descname, table, *this));
		break;
	case MapObjectType::PRODUCTIONSITE:
		buildings_->add(
		   new ProductionSiteDescr(object_descname, msgctxt, table, *this, egbase.world()));
		break;
	case MapObjectType::SHIP:
		ships_->add(new ShipDescr(object_descname, table));
		break;
	case MapObjectType::SOLDIER:
		workers_->add(new SoldierDescr(object_descname, table, *this));
		break;
	case MapObjectType::TRAININGSITE:
		buildings_->add(
		   new TrainingSiteDescr(object_descname, msgctxt, table, *this, egbase.world()));
		break;
	case MapObjectType::WARE:
		wares_->add(new WareDescr(object_descname, table));
		break;
	case MapObjectType::WAREHOUSE:
		buildings_->add(new WarehouseDescr(object_descname, table, *this));
		break;
	case MapObjectType::WORKER:
		workers_->add(new WorkerDescr(object_descname, table, *this));
		break;
	default:
		NEVER_HERE();
	}

	// Update status
	loaded_tribe_objects_.insert(object_name);

	// Mark as done
	tribe_objects_being_loaded_.erase(tribe_objects_being_loaded_.find(object_name));
}
// NOCOM ERROR: Unused key "return_on_dismantle" in LuaTable. Please report as a bug.
void Tribes::add_tribe(const LuaTable& table, const World& world) {
	const std::string name = table.get_string("name");
	// Register as in progress
	tribe_objects_being_loaded_.insert(name);

	if (Widelands::tribe_exists(name)) {
		if (scenario_tribes_ != nullptr && scenario_tribes_->has_key(name)) {
			// If we're loading a scenario with custom tribe entites, load them here.
			tribes_->add(new TribeDescr(Widelands::get_tribeinfo(name), *this, world, table,
			                            scenario_tribes_->get_table(name).get()));
		} else {
			// Normal tribes loading without scenario entities
			tribes_->add(new TribeDescr(Widelands::get_tribeinfo(name), *this, world, table));
		}
	} else {
		throw GameDataError(
		   "The tribe '%s'' is not listed in data/tribes/preload.lua.", name.c_str());
	}

	// Update status
	loaded_tribe_objects_.insert(name);

	// Mark as done
	tribe_objects_being_loaded_.erase(tribe_objects_being_loaded_.find(name));
}

/* NOCOM
void Tribes::load_graphics() {
   for (size_t tribeindex = 0; tribeindex < nrtribes(); ++tribeindex) {
      TribeDescr* tribe = tribes_->get_mutable(tribeindex);
      for (const std::string& texture_path : tribe->normal_road_paths()) {
         tribe->add_normal_road_texture(g_gr->images().get(texture_path));
      }
      for (const std::string& texture_path : tribe->busy_road_paths()) {
         tribe->add_busy_road_texture(g_gr->images().get(texture_path));
      }
      for (const std::string& texture_path : tribe->waterway_paths()) {
         tribe->add_waterway_texture(g_gr->images().get(texture_path));
      }
   }
} */

void Tribes::load_object(const std::string& object_name) {
	// Nothing to do if it's already loaded
	if (loaded_tribe_objects_.count(object_name) == 1) {
		return;
	}

	// Protect against circular dependencies
	if (tribe_objects_being_loaded_.count(object_name) == 1) {
		throw GameDataError(
		   "Tribes::load_object: Object '%s' is already being loaded", object_name.c_str());
	}

	std::string object_script = "";

	// Load it - scenario objects take precedence
	if (registered_scenario_objects_.count(object_name) == 1) {
		object_script = registered_scenario_objects_.at(object_name);
	} else if (registered_tribe_objects_.count(object_name) == 1) {
		object_script = registered_tribe_objects_.at(object_name);
	} else {
		throw GameDataError(
		   "Tribes::load_object: Object '%s' was not registered", object_name.c_str());
	}

	// Protect against circular dependencies when 1 script file has multiple objects in it
	if (tribe_objects_being_loaded_.count(object_script) == 1) {
		log("Tribes::load_object: Object script '%s' is already being loaded", object_name.c_str());
		return;
	}
	tribe_objects_being_loaded_.insert(object_script);
	lua_->run_script(object_script);
	tribe_objects_being_loaded_.erase(tribe_objects_being_loaded_.find(object_script));
}

DescriptionIndex Tribes::load_tribe(const std::string& tribename) {
	try {
		load_object(tribename);
	} catch (WException& e) {
		throw GameDataError("Error while loading tribe '%s': %s", tribename.c_str(), e.what());
	}
	return safe_tribe_index(tribename);
}

DescriptionIndex Tribes::load_building(const std::string& buildingname) {
	try {
		load_object(buildingname);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading building type '%s': %s", buildingname.c_str(), e.what());
	}
	return safe_building_index(buildingname);
}

DescriptionIndex Tribes::load_immovable(const std::string& immovablename) {
	try {
		load_object(immovablename);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading immovable type '%s': %s", immovablename.c_str(), e.what());
	}
	return safe_immovable_index(immovablename);
}

DescriptionIndex Tribes::load_ship(const std::string& shipname) {
	try {
		load_object(shipname);
	} catch (WException& e) {
		throw GameDataError("Error while loading ship type '%s': %s", shipname.c_str(), e.what());
	}
	return safe_ship_index(shipname);
}

DescriptionIndex Tribes::load_ware(const std::string& warename) {
	try {
		load_object(warename);
	} catch (WException& e) {
		throw GameDataError("Error while loading ware type '%s': %s", warename.c_str(), e.what());
	}
	return safe_ware_index(warename);
}

DescriptionIndex Tribes::load_worker(const std::string& workername) {
	try {
		load_object(workername);
	} catch (WException& e) {
		throw GameDataError("Error while loading worker type '%s': %s", workername.c_str(), e.what());
	}
	return safe_worker_index(workername);
}

uint32_t Tribes::get_largest_workarea() const {
	return largest_workarea_;
}
}  // namespace Widelands
