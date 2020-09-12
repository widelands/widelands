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

#include "logic/map_objects/descriptions.h"

#include <memory>

#include "base/log.h"
#include "base/wexception.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/building.h"
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
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"

namespace Widelands {
Descriptions::Descriptions(LuaInterface* lua)
   : critters_(new DescriptionMaintainer<CritterDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     terrains_(new DescriptionMaintainer<TerrainDescription>()),
     resources_(new DescriptionMaintainer<ResourceDescription>()),
     buildings_(new DescriptionMaintainer<BuildingDescr>()),
     ships_(new DescriptionMaintainer<ShipDescr>()),
     wares_(new DescriptionMaintainer<WareDescr>()),
     workers_(new DescriptionMaintainer<WorkerDescr>()),
     tribes_(new DescriptionMaintainer<TribeDescr>()),
     legacy_lookup_table_(new TribesLegacyLookupTable()),
     largest_workarea_(0),
     scenario_tribes_(nullptr),
     tribes_have_been_registered_(false),
     lua_(lua),
     description_manager_(new DescriptionManager(lua)) {

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

	// Walk world directory and register objects
	description_manager_->register_directory("world", g_fs, false);

	// We register tribes on demand in load_tribe for performance reasons
}

const DescriptionMaintainer<CritterDescr>& Descriptions::critters() const {
	return *critters_;
}
const DescriptionMaintainer<ImmovableDescr>& Descriptions::immovables() const {
	return *immovables_;
}
const DescriptionMaintainer<TerrainDescription>& Descriptions::terrains() const {
	return *terrains_;
}

size_t Descriptions::nr_buildings() const {
	return buildings_->size();
}
size_t Descriptions::nr_critters() const {
	return critters_->size();
}
size_t Descriptions::nr_immovables() const {
	return immovables_->size();
}
size_t Descriptions::nr_resources() const {
	return resources_->size();
}
size_t Descriptions::nr_terrains() const {
	return terrains_->size();
}
size_t Descriptions::nr_tribes() const {
	return tribes_->size();
}
size_t Descriptions::nr_wares() const {
	return wares_->size();
}
size_t Descriptions::nr_workers() const {
	return workers_->size();
}

bool Descriptions::ware_exists(const std::string& warename) const {
	return wares_->exists(warename) != nullptr;
}
bool Descriptions::ware_exists(DescriptionIndex index) const {
	return wares_->get_mutable(index) != nullptr;
}
bool Descriptions::worker_exists(const std::string& workername) const {
	return workers_->exists(workername) != nullptr;
}
bool Descriptions::worker_exists(DescriptionIndex index) const {
	return workers_->get_mutable(index) != nullptr;
}
bool Descriptions::building_exists(const std::string& buildingname) const {
	return buildings_->exists(buildingname) != nullptr;
}
bool Descriptions::building_exists(DescriptionIndex index) const {
	return buildings_->get_mutable(index) != nullptr;
}
bool Descriptions::immovable_exists(DescriptionIndex index) const {
	return immovables_->get_mutable(index) != nullptr;
}
bool Descriptions::ship_exists(DescriptionIndex index) const {
	return ships_->get_mutable(index) != nullptr;
}
bool Descriptions::tribe_exists(const std::string& tribename) const {
	return tribes_->exists(tribename) != nullptr;
}
bool Descriptions::tribe_exists(DescriptionIndex index) const {
	return tribes_->get_mutable(index) != nullptr;
}

DescriptionIndex Descriptions::safe_building_index(const std::string& buildingname) const {
	const DescriptionIndex result =
	   building_index(legacy_lookup_table_->lookup_building(buildingname));
	if (!building_exists(result)) {
		throw GameDataError("Unknown building '%s'", buildingname.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_critter_index(const std::string& critername) const {
	DescriptionIndex const result = critter_index(critername);

	if (result == INVALID_INDEX) {
		throw GameDataError("Unknown critter '%s'", critername.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_immovable_index(const std::string& immovablename) const {
	const DescriptionIndex result =
	   immovable_index(legacy_lookup_table_->lookup_immovable(immovablename));
	if (!immovable_exists(result)) {
		throw GameDataError("Unknown immovable '%s'", immovablename.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_resource_index(const std::string& resourcename) const {
	DescriptionIndex const result = resource_index(resourcename);

	if (result == INVALID_INDEX) {
		throw GameDataError("Unknown resource '%s'", resourcename.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_ship_index(const std::string& shipname) const {
	const DescriptionIndex result = ship_index(legacy_lookup_table_->lookup_ship(shipname));
	if (!ship_exists(result)) {
		throw GameDataError("Unknown ship '%s'", shipname.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_terrain_index(const std::string& terrainname) const {
	DescriptionIndex const result = terrain_index(terrainname);

	if (result == INVALID_INDEX) {
		throw GameDataError("Unknown terrain '%s'", terrainname.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_tribe_index(const std::string& tribename) const {
	const DescriptionIndex result = tribe_index(tribename);
	if (!tribe_exists(result)) {
		throw GameDataError("Unknown tribe '%s'", tribename.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_ware_index(const std::string& warename) const {
	const DescriptionIndex result = ware_index(legacy_lookup_table_->lookup_ware(warename));
	if (!ware_exists(result)) {
		throw GameDataError("Unknown ware '%s'", warename.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_worker_index(const std::string& workername) const {
	const DescriptionIndex result = worker_index(legacy_lookup_table_->lookup_worker(workername));
	if (!worker_exists(result)) {
		throw GameDataError("Unknown worker '%s'", workername.c_str());
	}
	return result;
}

DescriptionIndex Descriptions::building_index(const std::string& buildingname) const {
	return buildings_->get_index(buildingname);
}
DescriptionIndex Descriptions::critter_index(const std::string& crittername) const {
	return critters_->get_index(crittername);
}
DescriptionIndex Descriptions::immovable_index(const std::string& immovablename) const {
	return immovables_->get_index(immovablename);
}
DescriptionIndex Descriptions::resource_index(const std::string& resourcename) const {
	return resourcename != "none" ? resources_->get_index(resourcename) : Widelands::kNoResource;
}
DescriptionIndex Descriptions::ship_index(const std::string& shipname) const {
	return ships_->get_index(shipname);
}
DescriptionIndex Descriptions::terrain_index(const std::string& terrainname) const {
	return terrains_->get_index(terrainname);
}
DescriptionIndex Descriptions::tribe_index(const std::string& tribename) const {
	return tribes_->get_index(tribename);
}

DescriptionIndex Descriptions::ware_index(const std::string& warename) const {
	return wares_->get_index(warename);
}

DescriptionIndex Descriptions::worker_index(const std::string& workername) const {
	return workers_->get_index(workername);
}

const BuildingDescr* Descriptions::get_building_descr(DescriptionIndex index) const {
	return buildings_->get_mutable(index);
}

BuildingDescr* Descriptions::get_mutable_building_descr(DescriptionIndex index) const {
	return buildings_->get_mutable(index);
}

const CritterDescr* Descriptions::get_critter_descr(DescriptionIndex index) const {
	return critters_->get_mutable(index);
}
const CritterDescr* Descriptions::get_critter_descr(const std::string& name) const {
	return critters_->exists(name.c_str());
}

const ImmovableDescr* Descriptions::get_immovable_descr(DescriptionIndex index) const {
	return immovables_->get_mutable(index);
}
ImmovableDescr* Descriptions::get_mutable_immovable_descr(DescriptionIndex index) const {
	return immovables_->get_mutable(index);
}

const ResourceDescription* Descriptions::get_resource_descr(DescriptionIndex const index) const {
	assert(index < resources_->size() || index == Widelands::kNoResource);
	return resources_->get_mutable(index);
}

const ShipDescr* Descriptions::get_ship_descr(DescriptionIndex index) const {
	return ships_->get_mutable(index);
}

const TerrainDescription* Descriptions::get_terrain_descr(DescriptionIndex index) const {
	return terrains_->get_mutable(index);
}
const TerrainDescription* Descriptions::get_terrain_descr(const std::string& name) const {
	DescriptionIndex const i = terrains_->get_index(name);
	return i != INVALID_INDEX ? terrains_->get_mutable(i) : nullptr;
}

const WareDescr* Descriptions::get_ware_descr(DescriptionIndex index) const {
	return wares_->get_mutable(index);
}
WareDescr* Descriptions::get_mutable_ware_descr(DescriptionIndex index) const {
	return wares_->get_mutable(index);
}

const WorkerDescr* Descriptions::get_worker_descr(DescriptionIndex index) const {
	return workers_->get_mutable(index);
}
WorkerDescr* Descriptions::get_mutable_worker_descr(DescriptionIndex index) const {
	return workers_->get_mutable(index);
}

const TribeDescr* Descriptions::get_tribe_descr(DescriptionIndex index) const {
	return tribes_->get_mutable(index);
}

// ************************ Loading *************************

void Descriptions::register_scenario_tribes(FileSystem* filesystem) {
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

void Descriptions::add_object_description(const LuaTable& table, MapObjectType type) {
	const std::string& type_name = table.get_string("name");
	const std::string& type_descname = table.get_string("descname").c_str();

	// TODO(GunChleoc): Compatibility, remove after v1.0
	if (table.has_key<std::string>("msgctxt")) {
		log_warn(
		   "The 'msgctxt' entry is no longer needed in '%s', please remove it", type_name.c_str());
	}

	description_manager_->mark_loading_in_progress(type_name);

	// Add
	switch (type) {
	case MapObjectType::CRITTER:
		critters_->add(
		   new CritterDescr(type_descname, table, description_manager_->get_attributes(type_name)));
		break;
	case MapObjectType::RESOURCE:
		resources_->add(new ResourceDescription(table));
		break;
	case MapObjectType::TERRAIN:
		terrains_->add(new TerrainDescription(table, *this));
		break;
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
	case MapObjectType::IMMOVABLE:
		immovables_->add(new ImmovableDescr(
		   type_descname, table, description_manager_->get_attributes(type_name), *this));
		break;
	case MapObjectType::MARKET:
		buildings_->add(new MarketDescr(type_descname, table, *this));
		break;
	case MapObjectType::MILITARYSITE:
		buildings_->add(new MilitarySiteDescr(type_descname, table, *this));
		break;
	case MapObjectType::PRODUCTIONSITE:
		buildings_->add(new ProductionSiteDescr(type_descname, table, *this));
		break;
	case MapObjectType::SHIP:
		ships_->add(new ShipDescr(type_descname, table));
		break;
	case MapObjectType::SOLDIER:
		workers_->add(new SoldierDescr(type_descname, table, *this));
		break;
	case MapObjectType::TRAININGSITE:
		buildings_->add(new TrainingSiteDescr(type_descname, table, *this));
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

void Descriptions::add_tribe(const LuaTable& table) {
	const std::string name = table.get_string("name");
	// Register as in progress
	description_manager_->mark_loading_in_progress(name);

	if (Widelands::tribe_exists(name)) {
		if (scenario_tribes_ != nullptr && scenario_tribes_->has_key(name)) {
			// If we're loading a scenario with custom tribe entites, load them here.
			tribes_->add(new TribeDescr(
			   Widelands::get_tribeinfo(name), *this, table, scenario_tribes_->get_table(name).get()));
		} else {
			// Normal tribes loading without scenario entities
			tribes_->add(new TribeDescr(Widelands::get_tribeinfo(name), *this, table));
		}
	} else {
		throw GameDataError(
		   "The tribe '%s' is not present in data/tribes/initialization", name.c_str());
	}

	// Mark as done
	description_manager_->mark_loading_done(name);
}

DescriptionIndex Descriptions::load_tribe(const std::string& tribename) {
	try {
		// Register tribes on demand for better performance during mapselect, for the editor and for
		// the website tools
		if (!tribes_have_been_registered_) {
			description_manager_->register_directory("tribes", g_fs, false);
			tribes_have_been_registered_ = true;
		}
		description_manager_->load_description(tribename);
	} catch (WException& e) {
		throw GameDataError("Error while loading tribe '%s': %s", tribename.c_str(), e.what());
	}
	return safe_tribe_index(tribename);
}

DescriptionIndex Descriptions::load_building(const std::string& buildingname) {
	try {
		description_manager_->load_description(buildingname);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading building type '%s': %s", buildingname.c_str(), e.what());
	}
	return safe_building_index(buildingname);
}

DescriptionIndex Descriptions::load_critter(const std::string& crittername) {
	try {
		description_manager_->load_description(crittername);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading critter type '%s': %s", crittername.c_str(), e.what());
	}
	return safe_critter_index(crittername);
}

DescriptionIndex Descriptions::load_immovable(const std::string& immovablename) {
	try {
		description_manager_->load_description(immovablename);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading immovable type '%s': %s", immovablename.c_str(), e.what());
	}
	return safe_immovable_index(immovablename);
}

DescriptionIndex Descriptions::load_resource(const std::string& resourcename) {
	try {
		description_manager_->load_description(resourcename);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading resource type '%s': %s", resourcename.c_str(), e.what());
	}
	return safe_resource_index(resourcename);
}

DescriptionIndex Descriptions::load_ship(const std::string& shipname) {
	try {
		description_manager_->load_description(shipname);
	} catch (WException& e) {
		throw GameDataError("Error while loading ship type '%s': %s", shipname.c_str(), e.what());
	}
	return safe_ship_index(shipname);
}

DescriptionIndex Descriptions::load_terrain(const std::string& terrainname) {
	try {
		description_manager_->load_description(terrainname);
	} catch (WException& e) {
		throw GameDataError(
		   "Error while loading terrain type '%s': %s", terrainname.c_str(), e.what());
	}
	return safe_terrain_index(terrainname);
}

DescriptionIndex Descriptions::load_ware(const std::string& warename) {
	try {
		description_manager_->load_description(warename);
	} catch (WException& e) {
		throw GameDataError("Error while loading ware type '%s': %s", warename.c_str(), e.what());
	}
	return safe_ware_index(warename);
}

DescriptionIndex Descriptions::load_worker(const std::string& workername) {
	try {
		description_manager_->load_description(workername);
	} catch (WException& e) {
		throw GameDataError("Error while loading worker type '%s': %s", workername.c_str(), e.what());
	}
	return safe_worker_index(workername);
}

WareWorker Descriptions::try_load_ware_or_worker(const std::string& objectname) const {
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

uint32_t Descriptions::get_largest_workarea() const {
	return largest_workarea_;
}

void Descriptions::increase_largest_workarea(uint32_t workarea) {
	largest_workarea_ = std::max(largest_workarea_, workarea);
}

}  // namespace Widelands
