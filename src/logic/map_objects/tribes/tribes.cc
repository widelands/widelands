/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/ferry.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/warehouse.h"

namespace Widelands {
Tribes::Tribes(DescriptionManager* description_manager, LuaInterface* lua)
   : buildings_(new DescriptionMaintainer<BuildingDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     ships_(new DescriptionMaintainer<ShipDescr>()),
     wares_(new DescriptionMaintainer<WareDescr>()),
     workers_(new DescriptionMaintainer<WorkerDescr>()),
     tribes_(new DescriptionMaintainer<TribeDescr>()),
     legacy_lookup_table_(new TribesLegacyLookupTable()),
     largest_workarea_(0),
     scenario_tribes_(nullptr),
     lua_(lua),
     description_manager_(description_manager) {

	// Register tribe names. Tribes have no attributes.
	std::vector<std::string> attributes;
	for (const TribeBasicInfo& tribeinfo : Widelands::get_all_tribeinfos()) {
		description_manager_->register_description(tribeinfo.name, tribeinfo.script, attributes);
		if (!attributes.empty()) {
			throw GameDataError("Tribes can't have attributes - please remove all attributes in "
			                    "'register.lua' for tribe '%s'.",
			                    tribeinfo.name.c_str());
		}
	}

	// Walk tribes directory and register objects
	description_manager_->register_directory("tribes", g_fs, false);
}

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
bool Tribes::tribe_exists(const std::string& tribename) const {
	return tribes_->exists(tribename) != nullptr;
}
bool Tribes::tribe_exists(DescriptionIndex index) const {
	return tribes_->get_mutable(index) != nullptr;
}

DescriptionIndex Tribes::safe_building_index(const std::string& buildingname) const {
	const DescriptionIndex result =
	   building_index(legacy_lookup_table_->lookup_building(buildingname));
	if (!building_exists(result)) {
		throw GameDataError("Unknown building type \"%s\"", buildingname.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_immovable_index(const std::string& immovablename) const {
	const DescriptionIndex result =
	   immovable_index(legacy_lookup_table_->lookup_immovable(immovablename));
	if (!immovable_exists(result)) {
		throw GameDataError("Unknown immovable type \"%s\"", immovablename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_ship_index(const std::string& shipname) const {
	const DescriptionIndex result = ship_index(legacy_lookup_table_->lookup_ship(shipname));
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
	const DescriptionIndex result = ware_index(legacy_lookup_table_->lookup_ware(warename));
	if (!ware_exists(result)) {
		throw GameDataError("Unknown ware type \"%s\"", warename.c_str());
	}
	return result;
}

DescriptionIndex Tribes::safe_worker_index(const std::string& workername) const {
	const DescriptionIndex result = worker_index(legacy_lookup_table_->lookup_worker(workername));
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

// ************************ Loading *************************

void Tribes::register_scenario_tribes(FileSystem* filesystem) {
	// If the map is a scenario with custom tribe entites, load them.
	if (filesystem->file_exists("scripting/tribes")) {
		scenario_tribes_.reset(nullptr);
		description_manager_->clear_scenario_descriptions();
		if (filesystem->file_exists("scripting/tribes/init.lua")) {
			scenario_tribes_ = lua_->run_script("map:scripting/tribes/init.lua");
		}
		description_manager_->register_directory("scripting/tribes", filesystem, true);
	}
}

void Tribes::add_tribe_object_type(const LuaTable& table, World& world, MapObjectType type) {
	const std::string& type_name = table.get_string("name");
	const std::string& type_descname = table.get_string("descname").c_str();

	if (table.has_key<std::string>("msgctxt")) {
		log("WARNING: The 'msgctxt' entry is no longer needed in '%s', please remove it\n", type_name.c_str());
	}

	description_manager_->mark_loading_in_progress(type_name);

	// Add
	switch (type) {
	case MapObjectType::CARRIER:
		workers_->add(new CarrierDescr(type_descname, table, *this));
		break;
	case MapObjectType::CONSTRUCTIONSITE:
		buildings_->add(new ConstructionSiteDescr(type_descname, table, *this));
		break;
	case MapObjectType::DISMANTLESITE:
		buildings_->add(new DismantleSiteDescr(type_descname, table, *this));
		break;
	case MapObjectType::FERRY:
		workers_->add(new FerryDescr(type_descname, table, *this));
		break;
	case MapObjectType::IMMOVABLE: {
		immovables_->add(new ImmovableDescr(
		   type_descname, table, description_manager_->get_attributes(type_name), *this));
	} break;
	case MapObjectType::MARKET:
		buildings_->add(new MarketDescr(type_descname, table, *this));
		break;
	case MapObjectType::MILITARYSITE:
		buildings_->add(new MilitarySiteDescr(type_descname, table, *this));
		break;
	case MapObjectType::PRODUCTIONSITE:
		buildings_->add(new ProductionSiteDescr(type_descname, table, *this, world));
		break;
	case MapObjectType::SHIP:
		ships_->add(new ShipDescr(type_descname, table));
		break;
	case MapObjectType::SOLDIER:
		workers_->add(new SoldierDescr(type_descname, table, *this));
		break;
	case MapObjectType::TRAININGSITE:
		buildings_->add(new TrainingSiteDescr(type_descname, table, *this, world));
		break;
	case MapObjectType::WARE:
		wares_->add(new WareDescr(type_descname, table));
		break;
	case MapObjectType::WAREHOUSE:
		buildings_->add(new WarehouseDescr(type_descname, table, *this));
		break;
	case MapObjectType::WORKER:
		workers_->add(new WorkerDescr(type_descname, table, *this));
		break;
	default:
		NEVER_HERE();
	}

	// Update status
	description_manager_->mark_loading_done(type_name);
}

void Tribes::add_tribe(const LuaTable& table, const World& world) {
	const std::string name = table.get_string("name");
	// Register as in progress
	description_manager_->mark_loading_in_progress(name);

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
		throw GameDataError("The tribe '%s' is not listed in data/tribes/init.lua.", name.c_str());
	}

	// Mark as done
	description_manager_->mark_loading_done(name);
}

DescriptionIndex Tribes::load_tribe(const std::string& tribename) {
	try {
		description_manager_->load_description(tribename);
	} catch (WException& e) {
		throw GameDataError("Error while loading tribe '%s': %s", tribename.c_str(), e.what());
	}
	return safe_tribe_index(tribename);
}

DescriptionIndex Tribes::load_building(const std::string& buildingname) {
	try {
		description_manager_->load_description(buildingname);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading building type '%s': %s", buildingname.c_str(), e.what());
	}
	return safe_building_index(buildingname);
}

DescriptionIndex Tribes::load_immovable(const std::string& immovablename) {
	try {
		description_manager_->load_description(immovablename);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading immovable type '%s': %s", immovablename.c_str(), e.what());
	}
	return safe_immovable_index(immovablename);
}

DescriptionIndex Tribes::load_ship(const std::string& shipname) {
	try {
		description_manager_->load_description(shipname);
	} catch (WException& e) {
		throw GameDataError("Error while loading ship type '%s': %s", shipname.c_str(), e.what());
	}
	return safe_ship_index(shipname);
}

DescriptionIndex Tribes::load_ware(const std::string& warename) {
	try {
		description_manager_->load_description(warename);
	} catch (WException& e) {
		throw GameDataError("Error while loading ware type '%s': %s", warename.c_str(), e.what());
	}
	return safe_ware_index(warename);
}

DescriptionIndex Tribes::load_worker(const std::string& workername) {
	try {
		description_manager_->load_description(workername);
	} catch (WException& e) {
		throw GameDataError("Error while loading worker type '%s': %s", workername.c_str(), e.what());
	}
	return safe_worker_index(workername);
}

WareWorker Tribes::try_load_ware_or_worker(const std::string& objectname) const {
	Notifications::publish(
	   NoteMapObjectDescription(objectname, NoteMapObjectDescription::LoadType::kObject));
	if (ware_exists(ware_index(objectname))) {
		return WareWorker::wwWARE;
	}
	if (worker_exists(worker_index(objectname))) {
		return WareWorker::wwWORKER;
	}
	throw GameDataError("'%s' has not been registered as a ware/worker type", objectname.c_str());
}

uint32_t Tribes::get_largest_workarea() const {
	return largest_workarea_;
}

void Tribes::increase_largest_workarea(uint32_t workarea) {
	largest_workarea_ = std::max(largest_workarea_, workarea);
}
}  // namespace Widelands
