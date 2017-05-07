/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_WAREHOUSE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_WAREHOUSE_H

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/request.h"
#include "economy/wares_queue.h"
#include "logic/map_objects/attackable.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/map_objects/tribes/wareworker.h"

class InteractivePlayer;

namespace Widelands {

class EditorGameBase;
class PortDock;
class Request;
struct Requirements;
class Soldier;
class TribeDescr;
class WareInstance;
struct WareList;

/*
Warehouse
*/
struct WarehouseSupply;

class WarehouseDescr : public BuildingDescr {
public:
	WarehouseDescr(const std::string& init_descname,
	               const LuaTable& t,
	               const EditorGameBase& egbase);
	~WarehouseDescr() override {
	}

	Building& create_object() const override;

	uint32_t get_conquers() const override {
		return conquers_;
	}

	uint32_t get_heal_per_second() const {
		return heal_per_second_;
	}

private:
	int32_t conquers_;
	uint32_t heal_per_second_;
	DISALLOW_COPY_AND_ASSIGN(WarehouseDescr);
};

class Warehouse : public Building, public Attackable, public SoldierControl {
	friend class PortDock;
	friend class MapBuildingdataPacket;

	MO_DESCR(WarehouseDescr)

public:
	/**
	 * Each ware and worker type has an associated per-warehouse
	 * stock policy that defines whether it will be stocked by this
	 * warehouse.
	 *
	 * \note The values of this enum are written directly into savegames,
	 * so be careful when changing them.
	 */
	enum class StockPolicy {
		/**
	    * The default policy allows stocking wares without any special priority.
	    */
		kNormal = 0,

		/**
	    * As long as there are warehouses with this policy for a ware, all
	    * available unstocked supplies will be transferred to warehouses
	    * with this policy.
	    */
		kPrefer = 1,

		/**
	    * If a ware has this stock policy, no more of this ware will enter
	    * the warehouse.
	    */
		kDontStock = 2,

		/**
	    * Like \ref kDontStock, but in addition, existing stock of this ware
	    * will be transported out of the warehouse over time.
	    */
		kRemove = 3,
	};

	/**
	 * Whether worker indices in count_workers() have to match exactly.
	 */
	enum class Match {
		/**
	    * Return the number of workers with matching indices.
	    */
		kExact,

		/**
	    * Return the number of workers with matching indices or
	    * which are more experienced workers of the given lower type.
	    */
		kCompatible
	};

	Warehouse(const WarehouseDescr&);
	virtual ~Warehouse();

	void load_finish(EditorGameBase&) override;

	/// Called only when the oject is logically created in the simulation. If
	/// called again, such as when the object is loaded from a savegame, it will
	/// cause bugs.
	///
	/// * Calls Building::init.
	/// * Creates an idle_request for each ware and worker type.
	/// * Sets a next_spawn time for each buildable worker type without cost
	///   that the owning player is allowed to create and schedules act for for
	///   the spawn.
	/// * Schedules act for military stuff (and sets next_military_act_).
	/// * Sees the area (since a warehouse is considered to be always occupied).
	/// * Conquers land if the the warehouse type is configured to do that.
	/// * Sends a message to the player about the creation of this warehouse.
	/// * Sets up @ref PortDock for ports
	bool init(EditorGameBase&) override;

	void cleanup(EditorGameBase&) override;

	void destroy(EditorGameBase&) override;

	void restore_portdock_or_destroy(EditorGameBase&);

	void act(Game& game, uint32_t data) override;

	void set_economy(Economy*) override;

	const WareList& get_wares() const;
	const WareList& get_workers() const;

	/**
	 * Returns a vector of all incorporated workers. These are the workers
	 * that are still present in the game, not just a stock figure.
	 */
	Workers get_incorporated_workers();

	void insert_wares(DescriptionIndex, Quantity count);
	void remove_wares(DescriptionIndex, Quantity count);
	void insert_workers(DescriptionIndex, Quantity count);
	void remove_workers(DescriptionIndex, Quantity count);

	/* SoldierControl implementation */
	std::vector<Soldier*> present_soldiers() const override;
	std::vector<Soldier*> stationed_soldiers() const override {
		return present_soldiers();
	}
	Quantity min_soldier_capacity() const override {
		return 0;
	}
	Quantity max_soldier_capacity() const override {
		return 4294967295U;
	}
	Quantity soldier_capacity() const override {
		return max_soldier_capacity();
	}
	void set_soldier_capacity(Quantity /* capacity */) override {
		throw wexception("Not implemented for a Warehouse!");
	}
	void drop_soldier(Soldier&) override {
		throw wexception("Not implemented for a Warehouse!");
	}
	int outcorporate_soldier(EditorGameBase&, Soldier&) override;
	int incorporate_soldier(EditorGameBase&, Soldier& soldier) override;

	bool fetch_from_flag(Game&) override;

	Quantity count_workers(const Game&, DescriptionIndex worker, const Requirements&, Match);
	Worker& launch_worker(Game&, DescriptionIndex worker, const Requirements&);

	// Adds the worker to the inventory. Takes ownership and might delete
	// 'worker'.
	void incorporate_worker(EditorGameBase&, Worker* worker);

	WareInstance& launch_ware(Game&, DescriptionIndex);
	bool do_launch_ware(Game&, WareInstance&);

	// Adds the ware to our inventory. Takes ownership and might delete 'ware'.
	void incorporate_ware(EditorGameBase&, WareInstance* ware);

	bool can_create_worker(Game&, DescriptionIndex) const;
	void create_worker(Game&, DescriptionIndex);

	Quantity get_planned_workers(Game&, DescriptionIndex index) const;
	void plan_workers(Game&, DescriptionIndex index, Quantity amount);
	std::vector<Quantity> calc_available_for_worker(Game&, DescriptionIndex index) const;

	void enable_spawn(Game&, uint8_t worker_types_without_cost_index);
	void disable_spawn(uint8_t worker_types_without_cost_index);

	// Begin Attackable implementation
	Player& owner() const override {
		return Building::owner();
	}
	bool can_attack() override;
	void aggressor(Soldier&) override;
	bool attack(Soldier&) override;
	// End Attackable implementation

	void receive_ware(Game&, DescriptionIndex ware) override;
	void receive_worker(Game&, Worker& worker) override;

	StockPolicy get_ware_policy(DescriptionIndex ware) const;
	StockPolicy get_worker_policy(DescriptionIndex ware) const;
	StockPolicy get_stock_policy(WareWorker waretype, DescriptionIndex wareindex) const;
	void set_ware_policy(DescriptionIndex ware, StockPolicy policy);
	void set_worker_policy(DescriptionIndex ware, StockPolicy policy);

	// Get the portdock if this is a port.
	PortDock* get_portdock() const {
		return portdock_;
	}

	// Returns the waresqueue of the expedition if this is a port.
	// Will throw an exception otherwise.
	InputQueue& inputqueue(DescriptionIndex, WareWorker) override;

	void log_general_info(const EditorGameBase&) override;

protected:
	/// Initializes the container sizes for the owner's tribe.
	void init_containers(Player& owner);

private:
	void init_portdock(EditorGameBase& egbase);

	/**
	 * Plan to produce a certain worker type in this warehouse. This means
	 * requesting all the necessary wares, if multiple different wares types are
	 * needed.
	 */
	struct PlannedWorkers {
		/// Index of the worker type we plan to create
		DescriptionIndex index;

		/// How many workers of this type are we supposed to create?
		Quantity amount;

		/// Requests to obtain the required build costs
		std::vector<Request*> requests;

		void cleanup();
	};

	static void request_cb(Game&, Request&, DescriptionIndex, Worker*, PlayerImmovable&);
	void check_remove_stock(Game&);

	bool load_finish_planned_worker(PlannedWorkers& pw);
	void update_planned_workers(Game&, PlannedWorkers& pw);
	void update_all_planned_workers(Game&);

	WarehouseSupply* supply_;

	std::vector<StockPolicy> ware_policy_;
	std::vector<StockPolicy> worker_policy_;

	// Workers who live here at the moment
	using WorkerList = std::vector<Worker*>;
	using IncorporatedWorkers = std::map<DescriptionIndex, WorkerList>;
	IncorporatedWorkers incorporated_workers_;
	std::vector<Time> next_worker_without_cost_spawn_;
	Time next_military_act_;
	Time next_stock_remove_act_;

	std::vector<PlannedWorkers> planned_workers_;

	PortDock* portdock_;

	// This is information for portdock, to know whether it should
	// try to recreate itself
	bool cleanup_in_progress_;
};
}

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_WAREHOUSE_H
