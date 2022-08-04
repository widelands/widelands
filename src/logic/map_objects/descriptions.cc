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

#include "logic/map_objects/descriptions.h"

#include <memory>

#include "base/log.h"
#include "base/wexception.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/addons.h"
#include "logic/filesystem_constants.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/descriptions_compatibility_table.h"
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
#include "sound/sound_handler.h"

namespace Widelands {

uint32_t Descriptions::instances_ = 0;

Descriptions::Descriptions(LuaInterface* lua, const AddOns::AddOnsList& addons)
   : all_tribes_(get_all_tribeinfos(&addons)),
     addons_(addons),
     critters_(new DescriptionMaintainer<CritterDescr>()),
     immovables_(new DescriptionMaintainer<ImmovableDescr>()),
     terrains_(new DescriptionMaintainer<TerrainDescription>()),
     resources_(new DescriptionMaintainer<ResourceDescription>()),
     buildings_(new DescriptionMaintainer<BuildingDescr>()),
     ships_(new DescriptionMaintainer<ShipDescr>()),
     wares_(new DescriptionMaintainer<WareDescr>()),
     workers_(new DescriptionMaintainer<WorkerDescr>()),
     tribes_(new DescriptionMaintainer<TribeDescr>()),
     compatibility_table_(new PostOneWorldLegacyLookupTable()),
     largest_workarea_(0),
     scenario_tribes_(nullptr),
     tribes_have_been_registered_(false),
     subscriber_(Notifications::subscribe<DescriptionManager::NoteMapObjectDescriptionTypeCheck>(
        [this](DescriptionManager::NoteMapObjectDescriptionTypeCheck note) { check(note); })),
     lua_(lua),
     description_manager_(new DescriptionManager(lua)) {
	instances_++;

	// Immediately preload and register all add-on units. Better to do this
	// very early than to risk crashes because it was done too late…

	assert(lua_);
	for (const auto& info : addons) {
		if (info->category == AddOns::AddOnCategory::kWorld ||
		    info->category == AddOns::AddOnCategory::kTribes) {
			const std::string script(kAddOnDir + FileSystem::file_separator() + info->internal_name +
			                         FileSystem::file_separator() + "preload.lua");
			if (g_fs->file_exists(script)) {
				log_info("Running preload script for add-on %s", info->internal_name.c_str());
				lua_->run_script(script);
			}
		}
	}

	for (const auto& info : addons) {
		if (info->category == AddOns::AddOnCategory::kWorld) {
			description_manager_->register_directory(
			   kAddOnDir + FileSystem::file_separator() + info->internal_name, g_fs,
			   DescriptionManager::RegistryCallerInfo(
			      DescriptionManager::RegistryCallerType::kWorldAddon, info->internal_name));
		} else if (info->category == AddOns::AddOnCategory::kTribes) {
			description_manager_->register_directory(
			   kAddOnDir + FileSystem::file_separator() + info->internal_name, g_fs,
			   DescriptionManager::RegistryCallerInfo(
			      DescriptionManager::RegistryCallerType::kTribeAddon, info->internal_name));
		}
	}

	// Register tribe names. Tribes have no attributes.
	std::vector<std::string> attributes;
	for (const TribeBasicInfo& tribeinfo : all_tribes_) {
		description_manager_->register_description(
		   tribeinfo.name, tribeinfo.script, attributes,
		   DescriptionManager::RegistryCallerInfo(
		      DescriptionManager::RegistryCallerType::kDefault, std::string()));
		if (!attributes.empty()) {
			throw GameDataError("Tribes can't have attributes - please remove all attributes in "
			                    "'register.lua' for tribe '%s'.",
			                    tribeinfo.name.c_str());
		}
	}

	// Walk world directory and register objects
	description_manager_->register_directory(
	   "world", g_fs,
	   DescriptionManager::RegistryCallerInfo(
	      DescriptionManager::RegistryCallerType::kDefault, std::string()));

	// We register tribes on demand in load_tribe for performance reasons
}

Descriptions::~Descriptions() {
	// We might have multiple Descriptions instances
	// so the sounds should only go with the last one
	// to prevent "Sound effect does not exist" errors
	assert(instances_ > 0);
	instances_--;
	if (g_sh != nullptr && 0 == instances_) {
		g_sh->remove_fx_set(SoundType::kAmbient);
	}
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
const DescriptionMaintainer<WorkerDescr>& Descriptions::workers() const {
	return *workers_;
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
	DescriptionIndex result = building_index(buildingname);
	if (result == Widelands::INVALID_INDEX) {
		result = building_index(compatibility_table_->lookup_building(buildingname));
		if (result == Widelands::INVALID_INDEX) {
			throw GameDataError("Unknown building '%s'", buildingname.c_str());
		}
	}
	return result;
}
DescriptionIndex Descriptions::safe_critter_index(const std::string& crittername) const {
	DescriptionIndex const result = critter_index(crittername);

	if (result == INVALID_INDEX) {
		throw GameDataError("Unknown critter '%s'", crittername.c_str());
	}
	return result;
}
DescriptionIndex Descriptions::safe_immovable_index(const std::string& immovablename) const {
	DescriptionIndex result = immovable_index(immovablename);
	if (result == Widelands::INVALID_INDEX) {
		result = immovable_index(compatibility_table_->lookup_immovable(immovablename));
		if (result == Widelands::INVALID_INDEX) {
			throw GameDataError("Unknown immovable '%s'", immovablename.c_str());
		}
	}
	return result;
}
DescriptionIndex Descriptions::safe_resource_index(const std::string& resourcename) const {
	DescriptionIndex result = resource_index(resourcename);
	if (result == INVALID_INDEX) {
		result = resource_index(compatibility_table_->lookup_resource(resourcename));
		if (result == INVALID_INDEX) {
			throw GameDataError("Unknown resource '%s'", resourcename.c_str());
		}
	}
	return result;
}
DescriptionIndex Descriptions::safe_ship_index(const std::string& shipname) const {
	DescriptionIndex result = ship_index(shipname);
	if (result == INVALID_INDEX) {
		result = ship_index(compatibility_table_->lookup_ship(shipname));
		if (result == INVALID_INDEX) {
			throw GameDataError("Unknown ship '%s'", shipname.c_str());
		}
	}
	return result;
}
DescriptionIndex Descriptions::safe_terrain_index(const std::string& terrainname) const {
	DescriptionIndex result = terrain_index(terrainname);
	if (result == INVALID_INDEX) {
		result = terrain_index(compatibility_table_->lookup_terrain(terrainname));
		if (result == INVALID_INDEX) {
			throw GameDataError("Unknown terrain '%s'", terrainname.c_str());
		}
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
	DescriptionIndex result = ware_index(warename);
	if (result == Widelands::INVALID_INDEX) {
		result = ware_index(compatibility_table_->lookup_ware(warename));
		if (result == Widelands::INVALID_INDEX) {
			throw GameDataError("Unknown ware '%s'", warename.c_str());
		}
	}
	return result;
}
DescriptionIndex Descriptions::safe_worker_index(const std::string& workername) const {
	DescriptionIndex result = worker_index(workername);
	if (result == Widelands::INVALID_INDEX) {
		result = worker_index(compatibility_table_->lookup_worker(workername));
		if (result == Widelands::INVALID_INDEX) {
			throw GameDataError("Unknown worker '%s'", workername.c_str());
		}
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
	// TODO(GunChleoc): Having "none" in the backend here is bad design.
	// I think we have this for supporting LuaField::get_resource only, but we need to verify this.
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
	return critters_->exists(name);
}

const ImmovableDescr* Descriptions::get_immovable_descr(DescriptionIndex index) const {
	return immovables_->get_mutable(index);
}
ImmovableDescr* Descriptions::get_mutable_immovable_descr(DescriptionIndex index) const {
	return immovables_->get_mutable(index);
}

ResourceDescription* Descriptions::get_mutable_resource_descr(DescriptionIndex const index) const {
	assert(index < resources_->size() || index == Widelands::kNoResource);
	return resources_->get_mutable(index);
}
const ResourceDescription* Descriptions::get_resource_descr(DescriptionIndex const index) const {
	return get_mutable_resource_descr(index);
}

const ShipDescr* Descriptions::get_ship_descr(DescriptionIndex index) const {
	return ships_->get_mutable(index);
}

const TerrainDescription* Descriptions::get_terrain_descr(DescriptionIndex index) const {
	return terrains_->get_mutable(index);
}
TerrainDescription* Descriptions::get_mutable_terrain_descr(DescriptionIndex index) const {
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
TribeDescr* Descriptions::get_mutable_tribe_descr(DescriptionIndex index) const {
	return tribes_->get_mutable(index);
}

// ************************ Loading *************************

void Descriptions::register_scenario_tribes(FileSystem* filesystem) {
	// If the map is a scenario with custom tribe entites, load them.
	if (filesystem->file_exists("scripting/tribes")) {
		description_manager_->clear_scenario_descriptions();
		if (scenario_tribes_ == nullptr && filesystem->file_exists("scripting/tribes/init.lua")) {
			scenario_tribes_ = lua_->run_script("map:scripting/tribes/init.lua");
		}
		description_manager_->register_directory(
		   "scripting/tribes", filesystem,
		   DescriptionManager::RegistryCallerInfo(
		      DescriptionManager::RegistryCallerType::kScenario, std::string()));
	}
}

void Descriptions::add_object_description(const LuaTable& table, MapObjectType type) {
	const std::string& type_name = table.get_string("name");
	const std::string& type_descname = table.get_string("descname");

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
		add_terrain_description(type_name, table);
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

void Descriptions::add_terrain_description(const std::string& type_name, const LuaTable& table) {
	const DescriptionManager::RegistryCallerInfo& c =
	   description_manager_->get_registry_caller_info(type_name);
	assert(c.second.empty() ^ (c.first == DescriptionManager::RegistryCallerType::kWorldAddon));
	uint32_t index = 0;
	if (!c.second.empty()) {
		index = addons_.size();
		for (uint32_t i = 0; i < addons_.size(); ++i) {
			if (addons_.at(i)->internal_name == c.second) {
				index = i;
				break;
			}
		}
		if (index >= addons_.size()) {
			throw wexception("Terrain %s defined by add-on %s which is not enabled", type_name.c_str(),
			                 c.second.c_str());
		}
		++index;  // To avoid conflicts between add-on number 0 and official units.
	}
	terrains_->add(new TerrainDescription(table, *this, index));
}

void Descriptions::add_tribe(const LuaTable& table) {
	const std::string name = table.get_string("name");
	// Register as in progress
	description_manager_->mark_loading_in_progress(name);

	if (Widelands::tribe_exists(name, all_tribes_)) {
		if (scenario_tribes_ != nullptr && scenario_tribes_->has_key(name)) {
			// If we're loading a scenario with custom tribe entites, load them here.
			tribes_->add(new TribeDescr(Widelands::get_tribeinfo(name, all_tribes_), *this, table,
			                            scenario_tribes_->get_table(name).get()));
		} else {
			// Normal tribes loading without scenario entities
			tribes_->add(new TribeDescr(Widelands::get_tribeinfo(name, all_tribes_), *this, table));
		}
	} else {
		throw GameDataError(
		   "The tribe '%s' is not present in data/tribes/initialization", name.c_str());
	}

	// Mark as done
	description_manager_->mark_loading_done(name);
}

void Descriptions::ensure_tribes_are_registered() {
	if (tribes_have_been_registered_) {
		return;
	}
	description_manager_->register_directory(
	   "tribes", g_fs,
	   DescriptionManager::RegistryCallerInfo(
	      DescriptionManager::RegistryCallerType::kDefault, std::string()));
	tribes_have_been_registered_ = true;
}

DescriptionIndex Descriptions::load_tribe(const std::string& tribename) {
	try {
		// Register tribes on demand for better performance during mapselect, for the editor and for
		// the website tools
		ensure_tribes_are_registered();
		description_manager_->load_description(tribename);
	} catch (WException& e) {
		throw GameDataError("Error while loading tribe '%s': %s", tribename.c_str(), e.what());
	}
	return safe_tribe_index(tribename);
}

DescriptionIndex Descriptions::load_building(const std::string& buildingname) {
	const std::string& looked_up_name = compatibility_table_->lookup_building(buildingname);
	description_manager_->load_description(looked_up_name);
	return safe_building_index(looked_up_name);
}

DescriptionIndex Descriptions::load_critter(const std::string& crittername) {
	const std::string& looked_up_name = compatibility_table_->lookup_critter(crittername);
	description_manager_->load_description(looked_up_name);
	return safe_critter_index(looked_up_name);
}

DescriptionIndex Descriptions::load_immovable(const std::string& immovablename) {
	const std::string& looked_up_name = compatibility_table_->lookup_immovable(immovablename);
	description_manager_->load_description(looked_up_name);
	return safe_immovable_index(looked_up_name);
}

DescriptionIndex Descriptions::load_resource(const std::string& resourcename) {
	const std::string& looked_up_name = compatibility_table_->lookup_resource(resourcename);
	description_manager_->load_description(looked_up_name);
	return safe_resource_index(looked_up_name);
}

DescriptionIndex Descriptions::load_ship(const std::string& shipname) {
	const std::string& looked_up_name = compatibility_table_->lookup_ship(shipname);
	description_manager_->load_description(looked_up_name);
	return safe_ship_index(looked_up_name);
}

DescriptionIndex Descriptions::load_terrain(const std::string& terrainname) {
	const std::string& looked_up_name = compatibility_table_->lookup_terrain(terrainname);
	description_manager_->load_description(looked_up_name);
	return safe_terrain_index(looked_up_name);
}

DescriptionIndex Descriptions::load_ware(const std::string& warename) {
	const std::string& looked_up_name = compatibility_table_->lookup_ware(warename);
	description_manager_->load_description(looked_up_name);
	return safe_ware_index(looked_up_name);
}

DescriptionIndex Descriptions::load_worker(const std::string& workername) {
	const std::string& looked_up_name = compatibility_table_->lookup_worker(workername);
	description_manager_->load_description(looked_up_name);
	return safe_worker_index(looked_up_name);
}

std::pair<WareWorker, DescriptionIndex>
Descriptions::load_ware_or_worker(const std::string& objectname) const {
	std::string looked_up_name = compatibility_table_->lookup_ware(objectname);
	Notifications::publish(
	   NoteMapObjectDescription(looked_up_name, NoteMapObjectDescription::LoadType::kObject, true));
	const DescriptionIndex wai = ware_index(looked_up_name);
	if (wai != Widelands::INVALID_INDEX) {
		return std::make_pair(WareWorker::wwWARE, wai);
	}
	looked_up_name = compatibility_table_->lookup_worker(objectname);
	Notifications::publish(
	   NoteMapObjectDescription(looked_up_name, NoteMapObjectDescription::LoadType::kObject));
	const DescriptionIndex woi = worker_index(looked_up_name);
	if (woi != Widelands::INVALID_INDEX) {
		return std::make_pair(WareWorker::wwWORKER, woi);
	}
	throw GameDataError("'%s' has not been registered as a ware/worker type", objectname.c_str());
}
std::pair<bool, DescriptionIndex>
Descriptions::load_building_or_immovable(const std::string& objectname) const {
	std::string looked_up_name = compatibility_table_->lookup_building(objectname);
	Notifications::publish(
	   NoteMapObjectDescription(looked_up_name, NoteMapObjectDescription::LoadType::kObject, true));
	const DescriptionIndex bi = building_index(looked_up_name);
	if (bi != Widelands::INVALID_INDEX) {
		return std::make_pair(true, bi);
	}
	looked_up_name = compatibility_table_->lookup_immovable(objectname);
	Notifications::publish(
	   NoteMapObjectDescription(looked_up_name, NoteMapObjectDescription::LoadType::kObject));
	const DescriptionIndex ii = immovable_index(looked_up_name);
	if (ii != Widelands::INVALID_INDEX) {
		return std::make_pair(false, ii);
	}
	throw GameDataError(
	   "'%s' has not been registered as a building or immovable type", objectname.c_str());
}

uint32_t Descriptions::get_largest_workarea() const {
	return largest_workarea_;
}

void Descriptions::increase_largest_workarea(uint32_t workarea) {
	largest_workarea_ = std::max(largest_workarea_, workarea);
}

void Descriptions::set_old_world_name(const std::string& name) {
	if (name.empty()) {
		compatibility_table_ =
		   std::unique_ptr<DescriptionsCompatibilityTable>(new PostOneWorldLegacyLookupTable());
	} else {
		compatibility_table_ =
		   std::unique_ptr<DescriptionsCompatibilityTable>(new OneWorldLegacyLookupTable(name));
	}
}

void Descriptions::finalize_loading() {
	postload_immovable_relations();
	scenario_tribes_.reset();
}

void Descriptions::add_immovable_relation(const std::string& a, const std::string& b) {
	immovable_relations_.emplace_back(a, b);
}
void Descriptions::postload_immovable_relations() {
	for (const auto& pair : immovable_relations_) {
		get_mutable_immovable_descr(load_immovable(pair.second))->add_became_from(*this, pair.first);
	}
	immovable_relations_.clear();
}

#define CHECK_FACTORY(addon, unit_type)                                                            \
	if (unit_type##_index(note.description_name) != INVALID_INDEX) {                                \
		throw GameDataError(#addon " add-ons must not define " #unit_type "s (offending unit: %s)",  \
		                    note.description_name.c_str());                                          \
	}

void Descriptions::check(const DescriptionManager::NoteMapObjectDescriptionTypeCheck& note) const {
	switch (note.caller.first) {
	case DescriptionManager::RegistryCallerType::kTribeAddon:
		if (note.caller.second.empty()) {
			throw wexception("Unidentified registry caller for tribe add-on (unit: %s)",
			                 note.description_name.c_str());
		}
		CHECK_FACTORY(Tribe, critter)
		CHECK_FACTORY(Tribe, terrain)
		CHECK_FACTORY(Tribe, resource)
		break;
	case DescriptionManager::RegistryCallerType::kWorldAddon:
		if (note.caller.second.empty()) {
			throw wexception("Unidentified registry caller for world add-on (unit: %s)",
			                 note.description_name.c_str());
		}
		CHECK_FACTORY(World, tribe)
		CHECK_FACTORY(World, ware)
		CHECK_FACTORY(World, worker)
		CHECK_FACTORY(World, building)
		CHECK_FACTORY(World, ship)
		break;
	default:
		if (!note.caller.second.empty()) {
			throw wexception("Unexpected registry caller identifier '%s' for offical/scenario unit %s",
			                 note.caller.second.c_str(), note.description_name.c_str());
		}
		// Scenarios and official scripts may load anything
		break;
	}
}

#undef CHECK_FACTORY

}  // namespace Widelands
