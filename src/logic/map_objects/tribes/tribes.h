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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBES_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBES_H

#include <memory>

#include "base/macros.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "scripting/lua_table.h"

namespace Widelands {

class Tribes {
public:
	Tribes();
	~Tribes() {
	}

	/// Adds this building type to the tribe description.
	void add_constructionsite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_dismantlesite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_militarysite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_productionsite_type(const LuaTable& table, const World& world);

	/// Adds this building type to the tribe description.
	void add_trainingsite_type(const LuaTable& table, const World& world);

	/// Adds this building type to the tribe description.
	void add_warehouse_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_market_type(const LuaTable& table);

	/// Adds this immovable type to the tribe description.
	void add_immovable_type(const LuaTable& table);

	/// Adds this ship type to the tribe description.
	void add_ship_type(const LuaTable& table);

	/// Adds this ware type to the tribe description.
	void add_ware_type(const LuaTable& table);

	/// Adds this worker type to the tribe description.
	void add_carrier_type(const LuaTable& table);

	/// Adds this worker type to the tribe description.
	void add_soldier_type(const LuaTable& table);

	/// Adds this worker type to the tribe description.
	void add_ferry_type(const LuaTable& table);

	/// Adds this worker type to the tribe description.
	void add_worker_type(const LuaTable& table);

	/// Adds a specific tribe's configuration.
	void add_tribe(const LuaTable& table);

	/// Registers a custom scenario building with the tribes
	void add_custom_building(const LuaTable& table);
	/// Registers a custom scenario worker with the tribes
	void add_custom_worker(const LuaTable& table);

	size_t nrbuildings() const;
	size_t nrtribes() const;
	size_t nrwares() const;
	size_t nrworkers() const;

	bool ware_exists(const std::string& warename) const;
	bool ware_exists(const DescriptionIndex& index) const;
	bool worker_exists(const std::string& workername) const;
	bool worker_exists(const DescriptionIndex& index) const;
	bool building_exists(const std::string& buildingname) const;
	bool building_exists(const DescriptionIndex& index) const;
	bool immovable_exists(DescriptionIndex index) const;
	bool ship_exists(DescriptionIndex index) const;
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
	const ShipDescr* get_ship_descr(DescriptionIndex ship_index) const;
	const WareDescr* get_ware_descr(DescriptionIndex ware_index) const;
	const WorkerDescr* get_worker_descr(DescriptionIndex worker_index) const;
	const TribeDescr* get_tribe_descr(DescriptionIndex tribe_index) const;

	/// Load tribes' graphics
	void load_graphics();

	/// Complete the Description objects' information with data from other Description objects.
	void postload();

	uint32_t get_largest_workarea() const;

private:
	void postload_calculate_trainingsites_proportions();
	void postload_register_economy_demand_checks(BuildingDescr& building_descr,
	                                             const TribeDescr& tribe_descr);

	std::unique_ptr<DescriptionMaintainer<BuildingDescr>> buildings_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<ShipDescr>> ships_;
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;
	std::unique_ptr<DescriptionMaintainer<WorkerDescr>> workers_;
	std::unique_ptr<DescriptionMaintainer<TribeDescr>> tribes_;
	std::unique_ptr<TribesLegacyLookupTable> legacy_lookup_table_;

	uint32_t largest_workarea_;

	DISALLOW_COPY_AND_ASSIGN(Tribes);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_TRIBES_H
