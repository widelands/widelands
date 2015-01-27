/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#ifndef WL_LOGIC_TRIBE_H
#define WL_LOGIC_TRIBE_H

#include <map>
#include <memory>
#include <vector>

#include "base/macros.h"
#include "graphic/animation.h"
#include "logic/building.h"
#include "logic/description_maintainer.h"
#include "logic/immovable.h"
#include "logic/ship.h"
#include "logic/tribe_basic_info.h"
#include "logic/ware_descr.h"
#include "logic/worker.h"

namespace Widelands {

class EditorGameBase;
class ResourceDescription;
class WareDescr;
class Warehouse;
class WorkerDescr;
class World;
struct BuildingDescr;
struct Event;

/*
Tribes
------

Every player chooses a tribe. A tribe has distinct properties such as the
buildings it can build and the associated graphics.
Two players can choose the same tribe.
*/
struct TribeDescr {
	TribeDescr(const LuaTable& t, const EditorGameBase&);

	//  Static function to check for tribes.
	static bool exists_tribe
		(const std::string & name, TribeBasicInfo * info = nullptr);
	static std::vector<std::string> get_all_tribenames();
	static std::vector<TribeBasicInfo> get_all_tribe_infos();


	const std::string & name() const {return name_;}

	// NOCOM(GunChleoc): Look at the usage, ranged-bases for loops now?
	WareIndex get_nrworkers() const {return workers_.size();}

	WorkerDescr const * get_worker_descr(const WareIndex& index) const {
		return workers_.at(index);
	}
	WareIndex worker_index(const std::string & workername) const {
		WareIndex index = egbase_.tribes().worker_index(workername);
		return workers_.at(index);
	}

	WareIndex carrier2() const {
		assert(!carrier_.empty());
		return safe_worker_index(carrier_);
	}

	// NOCOM(GunChleoc): Look at the usage, ranged-bases for loops now?
	WareIndex get_nrwares() const {return wares_.size();}

	const std::map<WareIndex, WareDescr> wares() { return wares_;}

	WareIndex safe_ware_index(const std::string & warename) const;
	WareIndex ware_index(const std::string & warename) const;
	WareDescr const * get_ware_descr(const WareIndex& index) const {
		return wares_.at(index);
	}
	bool has_ware(const WareIndex& index) const {
		return wares_.count(index) == 1;
	}
	void set_ware_type_has_demand_check(const WareIndex& index, const std::string& tribename) const {
		wares_.at(index)->set_has_demand_check(tribename);
	}
	void set_worker_type_has_demand_check(const WareIndex& index) const {
		workers_.at(index)->set_has_demand_check();
	}
	WareIndex safe_worker_index(const std::string & workername) const;

	// NOCOM(GunChleoc): Look at the usage, ranged-bases for loops now?
	BuildingIndex get_nrbuildings() const {
		return buildings_.size();
	}
	BuildingIndex safe_building_index(char const * name) const;
	BuildingDescr const * get_building_descr(const BuildingIndex& index) const {
		return buildings_.at(index);
	}
	BuildingIndex building_index(const std::string & buildingname) const {
		return egbase_.tribes().building_index(buildingname);
	}

	int get_immovable_index(const std::string & immovablename) const {
		return egbase_.tribes().immovable_index(immovablename);
	}

	ImmovableDescr const * get_immovable_descr(int index) const {
		return immovables_.at(index);
	}
	ImmovableDescr const * get_immovable_descr(const std::string& immovablename) const {
		return egbase_.tribes().immovable_index(immovablename);
	}

	ShipDescr const * get_ship_descr(int const index) const {
		return ships_.at(index);
	}
	ShipDescr const * get_ship_descr(const std::string & shipname) const {
		return egbase_.tribes().ship_index(shipname);
	}

	const std::vector<WareIndex> & worker_types_without_cost() const {
		return m_worker_types_without_cost;
	}

	uint32_t frontier_animation() const {
		return m_frontier_animation_id;
	}

	uint32_t flag_animation() const {
		return m_flag_animation_id;
	}

	uint32_t get_bob_vision_range() const {return m_bob_vision_range;}

	uint32_t get_resource_indicator
		(const ResourceDescription * const res, const uint32_t amount) const;

	void postload(EditorGameBase&);
	void load_graphics();

	struct Initialization {
		std::string script;
		std::string descname;
	};

	// Returns the initalization at 'index' (which must not be out of bounds).
	const Initialization& initialization(const uint8_t index) const {
		return m_initializations.at(index);
	}

	using WaresOrder = std::vector<std::vector<Widelands::WareIndex>>;
	using WaresOrderCoords = std::vector<std::pair<uint32_t, uint32_t>>;
	const WaresOrder & wares_order() const {return m_wares_order;}
	const WaresOrderCoords & wares_order_coords() const {
		return m_wares_order_coords;
	}

	const WaresOrder & workers_order() const {return m_workers_order;}
	const WaresOrderCoords & workers_order_coords() const {
		return m_workers_order_coords;
	}

	void resize_ware_orders(size_t maxLength);

private:
	void add_building(const std::string& buildingname);

	const std::string name_;
	const EditorGameBase& egbase_;

	uint32_t m_frontier_animation_id;
	uint32_t m_flag_animation_id;
	uint32_t m_bob_vision_range;

	std::map<BuildingIndex, BuildingDescr> buildings_;
	std::map<int, ImmovableDescr> immovables_;  // The player immovables
	std::map<int, ShipDescr> ships_;
	std::map<WareIndex, WorkerDescr> workers_;
	std::map<WareIndex, WareDescr> wares_;
	std::string                       m_carrier; // NOCOM(GunChleoc): Use this to define the basic carrier. We need a logic change here.
	std::string                       carrier_;
	std::string                       m_soldier; // NOCOM(GunChleoc): We can probably remove these from the init.
	// Order and positioning of wares in the warehouse display
	WaresOrder                        m_wares_order;
	WaresOrderCoords                  m_wares_order_coords;
	WaresOrder                        m_workers_order;
	WaresOrderCoords                  m_workers_order_coords;

	std::vector<WareIndex> m_worker_types_without_cost;

	std::vector<Initialization> m_initializations;

	DISALLOW_COPY_AND_ASSIGN(TribeDescr);
};

}

#endif  // end of include guard: WL_LOGIC_TRIBE_H
