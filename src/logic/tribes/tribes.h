/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef WL_LOGIC_TRIBES_TRIBES_H
#define WL_LOGIC_TRIBES_TRIBES_H

#include <memory>

#include "base/macros.h"
#include "graphic/texture.h"
#include "logic/carrier.h"
#include "logic/constructionsite.h"
#include "logic/description_maintainer.h"
#include "logic/dismantlesite.h"
#include "logic/immovable.h"
#include "logic/militarysite.h"
#include "logic/productionsite.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/tribes/tribe_descr.h"
#include "logic/tribes/tribe_basic_info.h"
#include "logic/warehouse.h"
#include "logic/ware_descr.h"
#include "logic/worker_descr.h"
#include "scripting/lua_table.h"

namespace Widelands {

class WareDescr;
class WorkerDescr;

class Tribes {
public:
	Tribes();
	~Tribes() {}

	/// Returns a vector with the basic info for all tribes.
	static std::vector<TribeBasicInfo> get_all_tribeinfos();

	/// Returns the basic preload info for a tribe.
	static TribeBasicInfo tribeinfo(const std::string& tribename);

	/// Returns whether a tribe with this tribenamethis tribe is listed in tribes/preload.lua.
	static bool tribe_exists(const std::string & tribename);

	/// Returns a string vector with the names of all tribes.
	std::vector<std::string> get_all_tribenames();

	/// Adds this building type to the tribe description.
	void add_constructionsite_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this building type to the tribe description.
	void add_dismantlesite_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this building type to the tribe description.
	void add_militarysite_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this building type to the tribe description.
	void add_productionsite_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this building type to the tribe description.
	void add_trainingsite_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this building type to the tribe description.
	void add_warehouse_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this immovable type to the tribe description.
	void add_immovable_type(const LuaTable& table);

	/// Adds this ship type to the tribe description.
	void add_ship_type(const LuaTable& table);

	/// Adds this ware type to the tribe description.
	void add_ware_type(const LuaTable& table);

	/// Adds this worker type to the tribe description.
	void add_carrier_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this worker type to the tribe description.
	void add_soldier_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds this worker type to the tribe description.
	void add_worker_type(const LuaTable& table, const EditorGameBase& egbase);

	/// Adds a specific tribe's configuration.
	void add_tribe(const LuaTable& table, const EditorGameBase& egbase);

	size_t nrbuildings() const;
	size_t nrtribes() const;
	size_t nrwares() const;
	size_t nrworkers() const;

	bool ware_exists(const WareIndex& index) const;
	bool worker_exists(const WareIndex& index) const;
	bool building_exists(const std::string& buildingname) const;
	bool building_exists(const BuildingIndex& index) const;
	bool immovable_exists(WareIndex index) const;
	bool ship_exists(int index) const;
	bool tribe_exists(int index) const;

	BuildingIndex safe_building_index(const std::string& buildingname) const;
	WareIndex safe_immovable_index(const std::string& immovablename) const;
	WareIndex safe_ship_index(const std::string& shipname) const;
	WareIndex safe_tribe_index(const std::string& tribename) const;
	WareIndex safe_ware_index(const std::string& warename) const;
	WareIndex safe_worker_index(const std::string& workername) const;

	BuildingIndex building_index(const std::string& buildingname) const;
	WareIndex immovable_index(const std::string& immovablename) const;
	WareIndex ship_index(const std::string& shipname) const;
	WareIndex tribe_index(const std::string& tribename) const;
	WareIndex ware_index(const std::string& warename) const;
	WareIndex worker_index(const std::string& workername) const;

	const BuildingDescr* get_building_descr(BuildingIndex building_index) const;
	const ImmovableDescr* get_immovable_descr(WareIndex immovable_index) const;
	const ShipDescr* get_ship_descr(WareIndex ship_index) const;
	const WareDescr* get_ware_descr(WareIndex ware_index) const;
	const WorkerDescr* get_worker_descr(WareIndex worker_index) const;
	const TribeDescr* get_tribe_descr(WareIndex tribe_index) const;

	void set_ware_type_has_demand_check(const WareIndex& ware_index, const std::string& tribename) const;
	void set_worker_type_has_demand_check(const WareIndex& worker_index) const;

	/// Load tribes' graphics
	void load_graphics();

	/// Complete the Description objects' information with data from other Description objects.
	void postload();

private:
	std::unique_ptr<DescriptionMaintainer<BuildingDescr>> buildings_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<ShipDescr>> ships_;
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;
	std::unique_ptr<DescriptionMaintainer<WorkerDescr>> workers_;
	std::unique_ptr<DescriptionMaintainer<TribeDescr>> tribes_;

	std::unique_ptr<Texture> road_texture_; // Used in loading the road texture graphics

	DISALLOW_COPY_AND_ASSIGN(Tribes);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRIBES_TRIBES_H
