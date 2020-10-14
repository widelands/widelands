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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBES_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBES_H

#include <list>
#include <memory>

#include "base/macros.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/description_manager.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/map_object_type.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Tribes {
public:
	Tribes(DescriptionManager* description_manager, LuaInterface* lua);
	~Tribes() = default;

	size_t nrbuildings() const;
	size_t nrtribes() const;
	size_t nrwares() const;
	size_t nrworkers() const;

	bool ware_exists(const std::string& warename) const;
	bool ware_exists(DescriptionIndex index) const;
	bool worker_exists(const std::string& workername) const;
	bool worker_exists(DescriptionIndex index) const;
	bool building_exists(const std::string& buildingname) const;
	bool building_exists(DescriptionIndex index) const;
	bool immovable_exists(DescriptionIndex index) const;
	bool ship_exists(DescriptionIndex index) const;
	bool tribe_exists(const std::string& tribename) const;
	bool tribe_exists(DescriptionIndex index) const;

	DescriptionIndex safe_building_index(const std::string& buildingname) const;
	DescriptionIndex safe_immovable_index(const std::string& immovablename) const;
	DescriptionIndex safe_ship_index(const std::string& shipname) const;
	DescriptionIndex safe_tribe_index(const std::string& tribename) const;
	DescriptionIndex safe_ware_index(const std::string& warename) const;
	DescriptionIndex safe_worker_index(const std::string& workername) const;

	DescriptionIndex building_index(const std::string& buildingname) const;
	DescriptionIndex immovable_index(const std::string& immovablename) const;
	DescriptionIndex ship_index(const std::string& shipname) const;
	DescriptionIndex tribe_index(const std::string& tribename) const;
	DescriptionIndex ware_index(const std::string& warename) const;
	DescriptionIndex worker_index(const std::string& workername) const;

	const BuildingDescr* get_building_descr(DescriptionIndex building_index) const;
	BuildingDescr* get_mutable_building_descr(DescriptionIndex building_index) const;
	const ImmovableDescr* get_immovable_descr(DescriptionIndex immovable_index) const;
	ImmovableDescr* get_mutable_immovable_descr(DescriptionIndex immovable_index) const;
	const ShipDescr* get_ship_descr(DescriptionIndex ship_index) const;
	const WareDescr* get_ware_descr(DescriptionIndex ware_index) const;
	WareDescr* get_mutable_ware_descr(DescriptionIndex ware_index) const;
	const WorkerDescr* get_worker_descr(DescriptionIndex worker_index) const;
	WorkerDescr* get_mutable_worker_descr(DescriptionIndex worker_index) const;
	const TribeDescr* get_tribe_descr(DescriptionIndex tribe_index) const;

	// ************************ Loading *************************

	/// Define a scenario tribe directory, search it for 'register.lua' files and register their
	/// 'init.lua' scripts
	void register_scenario_tribes(FileSystem* filesystem);

	/// Add a tribe object type to the tribes.
	void add_tribe_object_type(const LuaTable& table, World& world, MapObjectType type);

	/// Adds a specific tribe's configuration.
	void add_tribe(const LuaTable& table, const World& world);

	/// Load a tribe that has been registered previously with 'register_description'
	DescriptionIndex load_tribe(const std::string& tribename);
	/// Load a building that has been registered previously with 'register_description'
	DescriptionIndex load_building(const std::string& buildingname);
	/// Load an immovable that has been registered previously with 'register_description'
	DescriptionIndex load_immovable(const std::string& immovablename);
	/// Load a ship that has been registered previously with 'register_description'
	DescriptionIndex load_ship(const std::string& shipname);
	/// Load a ware that has been registered previously with 'register_description'
	DescriptionIndex load_ware(const std::string& warename);
	/// Load a worker that has been registered previously with 'register_description'
	DescriptionIndex load_worker(const std::string& workername);
	/// Try to load a ware/worker that has been registered previously with 'register_description'
	/// when we don't know whether it's a ware or worker
	/// Throws GameDataError if object hasn't been registered
	WareWorker try_load_ware_or_worker(const std::string& objectname) const;

	uint32_t get_largest_workarea() const;
	void increase_largest_workarea(uint32_t workarea);

private:
	std::unique_ptr<DescriptionMaintainer<BuildingDescr>> buildings_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<ShipDescr>> ships_;
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;
	std::unique_ptr<DescriptionMaintainer<WorkerDescr>> workers_;
	std::unique_ptr<DescriptionMaintainer<TribeDescr>> tribes_;
	std::unique_ptr<TribesLegacyLookupTable> legacy_lookup_table_;

	uint32_t largest_workarea_;

	/// Custom scenario tribes
	std::unique_ptr<LuaTable> scenario_tribes_;

	LuaInterface* lua_;                        // Not owned
	DescriptionManager* description_manager_;  // Not owned
	DISALLOW_COPY_AND_ASSIGN(Tribes);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBES_H
