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
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/ware_descr.h"
#include "logic/worker_descr.h"
#include "scripting/lua_table.h"

constexpr const Widelands::WareIndex kInvalidWare = -1;

namespace Widelands {

class WareDescr;
class WorkerDescr;

// NOCOM(#sirver): Make this compatible with TribeDescr.
class Tribes {
public:
	Tribes(EditorGameBase&);
	~Tribes();

	/// Adds this building type to the tribe description.
	void add_constructionsite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_dismantlesite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_militarysite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_productionsite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_trainingsite_type(const LuaTable& table);

	/// Adds this building type to the tribe description.
	void add_warehouse_type(const LuaTable& table);

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
	void add_worker_type(const LuaTable& table);

	/// Adds a specific tribe's configuration.
	void add_tribe(const LuaTable& table);

	WareIndex nrwares() const;
	WareIndex get_nrworkers() const;

	bool ware_exists(const WareIndex& index) const {
		return wares_.count(index) == 1;
	}

	BuildingIndex safe_building_index(const std::string& buildingname) const;
	int safe_immovable_index(const std::string& immovablename) const;
	int safe_ship_index(const std::string& shipname) const;
	WareIndex safe_ware_index(const std::string& warename) const;
	WareIndex safe_worker_index(const std::string& workername) const;

	BuildingIndex building_index(const std::string& buildingname) const;
	int immovable_index(const std::string& immovablename) const;
	int ship_index(const std::string& shipname) const;
	WareIndex ware_index(const std::string& warename) const;
	WareIndex worker_index(const std::string& workername) const;

	BuildingDescr const* get_building_descr(BuildingIndex building_index) const;
	ImmovableDescr const* get_immovable_descr(const std::string& immovablename) const;
	ShipDescr const* get_ship_descr(const std::string& shipname) const;
	WareDescr const* get_ware_descr(WareIndex ware_index) const;
	WorkerDescr const* get_worker_descr(WareIndex worker_index) const;

	void set_ware_type_has_demand_check(WareIndex ware_index, const std::string& tribename);
	void set_worker_type_has_demand_check(WareIndex worker_index, const std::string& tribename);
	const std::vector<WareIndex>& worker_types_without_cost() const;

private:
	EditorGameBase& egbase_;
	std::unique_ptr<DescriptionMaintainer<BuildingDescr>> buildings_;
	std::unique_ptr<DescriptionMaintainer<ImmovableDescr>> immovables_;
	std::unique_ptr<DescriptionMaintainer<ShipDescr>> ships_;
	std::unique_ptr<DescriptionMaintainer<WareDescr>> wares_;
	std::unique_ptr<DescriptionMaintainer<WorkerDescr>> workers_;
	std::unique_ptr<DescriptionMaintainer<TribeDescr>> tribes_;

	std::vector<WareIndex> worker_types_without_cost_;

	DISALLOW_COPY_AND_ASSIGN(Tribes);
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_TRIBES_TRIBES_H
