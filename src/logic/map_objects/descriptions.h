/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_H
#define WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_H

#include <memory>

#include "base/macros.h"
#include "io/filesystem/filesystem.h"
#include "logic/addons.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/description_manager.h"
#include "logic/map_objects/map_object_type.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "scripting/lua_table.h"

class DescriptionsCompatibilityTable;

namespace Widelands {

class BuildingDescr;
class CritterDescr;
class ImmovableDescr;
class ResourceDescription;
class ShipDescr;
class TerrainDescription;
class TribeDescr;
class WareDescr;
class WorkerDescr;

class Descriptions {
public:
	explicit Descriptions(LuaInterface* lua, const AddOns::AddOnsList&);
	~Descriptions();

	[[nodiscard]] const DescriptionMaintainer<CritterDescr>& critters() const;
	[[nodiscard]] const DescriptionMaintainer<TerrainDescription>& terrains() const;
	[[nodiscard]] const DescriptionMaintainer<ImmovableDescr>& immovables() const;
	[[nodiscard]] const DescriptionMaintainer<WorkerDescr>& workers() const;

	[[nodiscard]] size_t nr_buildings() const;
	[[nodiscard]] size_t nr_critters() const;
	[[nodiscard]] size_t nr_immovables() const;
	[[nodiscard]] size_t nr_terrains() const;
	[[nodiscard]] size_t nr_tribes() const;
	[[nodiscard]] size_t nr_resources() const;
	[[nodiscard]] size_t nr_wares() const;
	[[nodiscard]] size_t nr_workers() const;

	[[nodiscard]] bool building_exists(const std::string& buildingname) const;
	[[nodiscard]] bool building_exists(DescriptionIndex index) const;
	[[nodiscard]] bool immovable_exists(const std::string& immoname) const;
	[[nodiscard]] bool immovable_exists(DescriptionIndex index) const;
	[[nodiscard]] bool ship_exists(DescriptionIndex index) const;
	[[nodiscard]] bool tribe_exists(const std::string& tribename) const;
	[[nodiscard]] bool tribe_exists(DescriptionIndex index) const;
	[[nodiscard]] bool ware_exists(const std::string& warename) const;
	[[nodiscard]] bool ware_exists(DescriptionIndex index) const;
	[[nodiscard]] bool worker_exists(const std::string& workername) const;
	[[nodiscard]] bool worker_exists(DescriptionIndex index) const;
	[[nodiscard]] bool terrain_exists(const std::string& terrainname) const;
	[[nodiscard]] bool terrain_exists(DescriptionIndex index) const;

	/// Returns the index for 'buildingname' and throws an exception if the building can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_building_index(const std::string& buildingname) const;
	/// Returns the index for 'crittername' and throws an exception if the critter can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_critter_index(const std::string& crittername) const;
	/// Returns the index for 'immovablename' and throws an exception if the immovable can't be
	/// found. This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_immovable_index(const std::string& immovablename) const;
	/// Returns the index for 'warename' and throws an exception if the ware can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_resource_index(const std::string& resourcename) const;
	/// Returns the index for 'shipname' and throws an exception if the ship can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_ship_index(const std::string& shipname) const;
	/// Returns the index for 'terrainname' and throws an exception if the terrain can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_terrain_index(const std::string& terrainname) const;
	/// Returns the index for 'tribename' and throws an exception if the tribe can't be found.
	[[nodiscard]] DescriptionIndex safe_tribe_index(const std::string& tribename) const;
	/// Returns the index for 'warename' and throws an exception if the ware can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_ware_index(const std::string& warename) const;
	/// Returns the index for 'workername' and throws an exception if the worker can't be found.
	/// This function is safe for map/savegame compatibility.
	[[nodiscard]] DescriptionIndex safe_worker_index(const std::string& workername) const;

	[[nodiscard]] DescriptionIndex building_index(const std::string& buildingname) const;
	[[nodiscard]] DescriptionIndex critter_index(const std::string& crittername) const;
	[[nodiscard]] DescriptionIndex immovable_index(const std::string& immovablename) const;
	[[nodiscard]] DescriptionIndex resource_index(const std::string& resourcename) const;
	[[nodiscard]] DescriptionIndex ship_index(const std::string& shipname) const;
	[[nodiscard]] DescriptionIndex terrain_index(const std::string& terrainname) const;
	[[nodiscard]] DescriptionIndex tribe_index(const std::string& tribename) const;
	[[nodiscard]] DescriptionIndex ware_index(const std::string& warename) const;
	[[nodiscard]] DescriptionIndex worker_index(const std::string& workername) const;

	[[nodiscard]] const BuildingDescr* get_building_descr(DescriptionIndex index) const;
	[[nodiscard]] BuildingDescr* get_mutable_building_descr(DescriptionIndex index) const;
	[[nodiscard]] const CritterDescr* get_critter_descr(DescriptionIndex index) const;
	[[nodiscard]] const CritterDescr* get_critter_descr(const std::string& name) const;
	[[nodiscard]] CritterDescr* get_mutable_critter_descr(DescriptionIndex index) const;
	[[nodiscard]] const ImmovableDescr* get_immovable_descr(DescriptionIndex index) const;
	[[nodiscard]] ImmovableDescr* get_mutable_immovable_descr(DescriptionIndex index) const;
	[[nodiscard]] const ResourceDescription* get_resource_descr(DescriptionIndex index) const;
	[[nodiscard]] ResourceDescription* get_mutable_resource_descr(DescriptionIndex index) const;
	[[nodiscard]] const ShipDescr* get_ship_descr(DescriptionIndex index) const;
	[[nodiscard]] ShipDescr* get_mutable_ship_descr(DescriptionIndex index) const;
	[[nodiscard]] const TerrainDescription* get_terrain_descr(DescriptionIndex index) const;
	[[nodiscard]] const TerrainDescription* get_terrain_descr(const std::string& name) const;
	[[nodiscard]] TerrainDescription* get_mutable_terrain_descr(DescriptionIndex index) const;
	[[nodiscard]] const WareDescr* get_ware_descr(DescriptionIndex index) const;
	[[nodiscard]] WareDescr* get_mutable_ware_descr(DescriptionIndex index) const;
	[[nodiscard]] const WorkerDescr* get_worker_descr(DescriptionIndex index) const;
	[[nodiscard]] WorkerDescr* get_mutable_worker_descr(DescriptionIndex index) const;
	[[nodiscard]] const TribeDescr* get_tribe_descr(DescriptionIndex index) const;
	[[nodiscard]] TribeDescr* get_mutable_tribe_descr(DescriptionIndex index) const;

	// ************************ Loading *************************

	/// Define a scenario tribe directory, search it for 'register.lua' files and register their
	/// 'init.lua' scripts
	void register_scenario_tribes(FileSystem* filesystem);

	/// Add a tribe object type to the tribes.
	void add_object_description(const LuaTable& table, MapObjectType type);

	/// Adds a specific tribe's configuration.
	void add_tribe(const LuaTable& table);

	/// Load a building that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_building(const std::string& buildingname);
	/// Load a critter that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_critter(const std::string& crittername);
	/// Load an immovable that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_immovable(const std::string& immovablename);
	/// Load a resource that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_resource(const std::string& resourcename);
	/// Load a ship that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_ship(const std::string& shipname);
	/// Load a terrain that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_terrain(const std::string& terrainname);
	/// Load a tribe that has been registered previously with 'register_description'
	DescriptionIndex load_tribe(const std::string& tribename);
	/// Load a ware that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_ware(const std::string& warename);
	/// Load a worker that has been registered previously with 'register_description'.
	/// This function is safe for map/savegame compatibility.
	DescriptionIndex load_worker(const std::string& workername);
	/// Try to load a ware/worker that has been registered previously with 'register_description'
	/// when we don't know whether it's a ware or worker.
	/// Throws GameDataError if object hasn't been registered.
	/// This function is safe for map/savegame compatibility.
	std::pair<WareWorker, DescriptionIndex>  // NOLINT not nodiscard
	load_ware_or_worker(const std::string& objectname) const;
	/// Try to load a building or immovable that has been registered previously with
	/// 'register_description' when we don't know whether it's a building or immovable. Throws
	/// GameDataError if object hasn't been registered. If first == 'true', we have a building.
	/// Otherwise, it's an immovable. This function is safe for map/savegame compatibility.
	[[nodiscard]] std::pair<bool, DescriptionIndex>
	load_building_or_immovable(const std::string& objectname) const;

	/** Register the tribes if they have not been registered yet. */
	void ensure_tribes_are_registered();

	[[nodiscard]] uint32_t get_largest_workarea() const;
	void increase_largest_workarea(uint32_t workarea);

	/// For loading old maps
	void set_old_world_name(const std::string& name);

	[[nodiscard]] const AllTribes& all_tribes() const {
		return all_tribes_;
	}

	/** The order in which all units have been loaded. */
	[[nodiscard]] const std::vector<std::string>& load_order() const {
		return description_manager_->load_order();
	}

	void add_immovable_relation(const std::string&, const std::string&);
	void postload_immovable_relations();
	void finalize_loading();

private:
	AllTribes all_tribes_;
	const AddOns::AddOnsList& addons_;

	std::unique_ptr<DescriptionMaintainer<CritterDescr>> critters_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<TerrainDescription>> terrains_;
	std::unique_ptr<DescriptionMaintainer<ResourceDescription>> resources_;
	std::unique_ptr<DescriptionMaintainer<BuildingDescr>> buildings_;
	std::unique_ptr<DescriptionMaintainer<ShipDescr>> ships_;
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;
	std::unique_ptr<DescriptionMaintainer<WorkerDescr>> workers_;
	std::unique_ptr<DescriptionMaintainer<TribeDescr>> tribes_;
	std::unique_ptr<DescriptionsCompatibilityTable> compatibility_table_;

	/** Helper function for add_object_description(). */
	void add_terrain_description(const std::string&, const LuaTable&);

	uint32_t largest_workarea_{0U};

	/// Custom scenario tribes
	std::unique_ptr<LuaTable> scenario_tribes_;
	bool tribes_have_been_registered_{false};

	std::unique_ptr<Notifications::Subscriber<DescriptionManager::NoteMapObjectDescriptionTypeCheck>>
	   subscriber_;
	void check(const DescriptionManager::NoteMapObjectDescriptionTypeCheck&) const;

	std::vector<std::pair<std::string /* immo */, std::string /* becomes */>> immovable_relations_;

	LuaInterface* lua_;  // Not owned
	std::unique_ptr<DescriptionManager> description_manager_;
	DISALLOW_COPY_AND_ASSIGN(Descriptions);

	static uint32_t instances_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_DESCRIPTIONS_H
