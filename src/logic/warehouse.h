/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "economy/request.h"
#include "logic/attackable.h"
#include "logic/building.h"
#include "logic/soldiercontrol.h"
#include "logic/wareworker.h"

struct Interactive_Player;
struct Profile;

namespace Widelands {

class Editor_Game_Base;
class PortDock;
class Request;
struct Requirements;
class Soldier;
struct Tribe_Descr;
class WareInstance;
struct WareList;


/*
Warehouse
*/
struct WarehouseSupply;

struct Warehouse_Descr : public Building_Descr {
	Warehouse_Descr
		(char const * name, char const * descname,
		 const std::string & directory, Profile &, Section & global_s,
		 const Tribe_Descr &);

	virtual Building & create_object() const override;

	virtual uint32_t get_conquers() const override {return m_conquers;}

	uint32_t get_heal_per_second        () const {
		return m_heal_per_second;
	}
private:
	int32_t m_conquers;
	uint32_t m_heal_per_second;
};


class Warehouse : public Building, public Attackable, public SoldierControl {
	friend class PortDock;
	friend struct Map_Buildingdata_Data_Packet;

	MO_DESCR(Warehouse_Descr);

public:
	/**
	 * Each ware and worker type has an associated per-warehouse
	 * stock policy that defines whether it will be stocked by this
	 * warehouse.
	 *
	 * \note The values of this enum are written directly into savegames,
	 * so be careful when changing them.
	 */
	enum StockPolicy {
		/**
		 * The default policy allows stocking wares without any special priority.
		 */
		SP_Normal = 0,

		/**
		 * As long as there are warehouses with this policy for a ware, all
		 * available unstocked supplies will be transferred to warehouses
		 * with this policy.
		 */
		SP_Prefer = 1,

		/**
		 * If a ware has this stock policy, no more of this ware will enter
		 * the warehouse.
		 */
		SP_DontStock = 2,

		/**
		 * Like \ref SP_DontStock, but in addition, existing stock of this ware
		 * will be transported out of the warehouse over time.
		 */
		SP_Remove = 3,
	};

	Warehouse(const Warehouse_Descr &);
	virtual ~Warehouse();

	void load_finish(Editor_Game_Base &) override;

	char const * type_name() const override {return "warehouse";}

	/// Called only when the oject is logically created in the simulation. If
	/// called again, such as when the object is loaded from a savegame, it will
	/// cause bugs.
	///
	/// * Calls Building::init.
	/// * Creates an idle_request for each ware and worker type.
	/// * Sets a next_spawn time for each buildable worker type without cost
	///   that the owning player is allowed to create and schedules act for for
	///   the spawn.
	/// * Schedules act for military stuff (and sets m_next_military_act).
	/// * Sees the area (since a warehouse is considered to be always occupied).
	/// * Conquers land if the the warehouse type is configured to do that.
	/// * Sends a message to the player about the creation of this warehouse.
	/// * Sets up @ref PortDock for ports
	virtual void init(Editor_Game_Base &) override;

	virtual void cleanup(Editor_Game_Base &) override;

	virtual void destroy(Editor_Game_Base &) override;

	virtual void act(Game & game, uint32_t data) override;

	virtual void set_economy(Economy *) override;

	const WareList & get_wares() const;
	const WareList & get_workers() const;

	/**
	 * Returns a vector of all incorporated workers. These are the workers
	 * that are still present in the game, not just a stock figure.
	 */
	Workers get_incorporated_workers();

	void insert_wares  (Ware_Index, uint32_t count);
	void remove_wares  (Ware_Index, uint32_t count);
	void insert_workers(Ware_Index, uint32_t count);
	void remove_workers(Ware_Index, uint32_t count);

	/* SoldierControl implementation */
	std::vector<Soldier *> presentSoldiers() const override;
	std::vector<Soldier *> stationedSoldiers() const override {
		return presentSoldiers();
	}
	uint32_t minSoldierCapacity() const override {return 0;}
	uint32_t maxSoldierCapacity() const override {return 4294967295U;}
	uint32_t soldierCapacity() const override {return maxSoldierCapacity();}
	void setSoldierCapacity(uint32_t /* capacity */) override {
		throw wexception("Not implemented for a Warehouse!");
	}
	void dropSoldier(Soldier &) override {
		throw wexception("Not implemented for a Warehouse!");
	}
	int outcorporateSoldier(Editor_Game_Base &, Soldier &) override;
	int incorporateSoldier(Editor_Game_Base &, Soldier& soldier) override;

	virtual bool fetch_from_flag(Game &) override;

	uint32_t count_workers(const Game &, Ware_Index, const Requirements &);
	Worker & launch_worker(Game &, Ware_Index, const Requirements &);

	// Adds the worker to the inventory. Takes ownership and might delete
	// 'worker'.
	void incorporate_worker(Editor_Game_Base&, Worker* worker);

	WareInstance & launch_ware(Game &, Ware_Index);
	void do_launch_ware(Game &, WareInstance &);

	// Adds the ware to our inventory. Takes ownership and might delete 'ware'.
	void incorporate_ware(Editor_Game_Base&, WareInstance* ware);

	bool can_create_worker(Game &, Ware_Index) const;
	void     create_worker(Game &, Ware_Index);

	uint32_t get_planned_workers(Game &, Ware_Index index) const;
	void plan_workers(Game &, Ware_Index index, uint32_t amount);
	std::vector<uint32_t> calc_available_for_worker
		(Game &, Ware_Index index) const;

	void enable_spawn(Game &, uint8_t worker_types_without_cost_index);
	void disable_spawn(uint8_t worker_types_without_cost_index);

	// Begin Attackable implementation
	virtual Player & owner() const override {return Building::owner();}
	virtual bool canAttack() override;
	virtual void aggressor(Soldier &) override;
	virtual bool attack   (Soldier &) override;
	// End Attackable implementation

	virtual void receive_ware(Game &, Ware_Index ware) override;
	virtual void receive_worker(Game &, Worker & worker) override;

	StockPolicy get_ware_policy(Ware_Index ware) const;
	StockPolicy get_worker_policy(Ware_Index ware) const;
	StockPolicy get_stock_policy(WareWorker waretype, Ware_Index wareindex) const;
	void set_ware_policy(Ware_Index ware, StockPolicy policy);
	void set_worker_policy(Ware_Index ware, StockPolicy policy);

	// Get the portdock if this is a port.
	PortDock * get_portdock() const {return m_portdock;}

	// Returns the waresqueue of the expedition if this is a port. Will
	// assert(false) otherwise.
	virtual WaresQueue& waresqueue(Ware_Index) override;

	virtual void log_general_info(const Editor_Game_Base &) override;

protected:
	/// Create the warehouse information window.
	virtual void create_options_window
		(Interactive_GameBase &, UI::Window * & registry) override;

private:
	void init_portdock(Editor_Game_Base & egbase);

	/**
	 * Plan to produce a certain worker type in this warehouse. This means
	 * requesting all the necessary wares, if multiple different wares types are
	 * needed.
	 */
	struct PlannedWorkers {
		/// Index of the worker type we plan to create
		Ware_Index index;

		/// How many workers of this type are we supposed to create?
		uint32_t amount;

		/// Requests to obtain the required build costs
		std::vector<Request *> requests;

		void cleanup();
	};

	static void request_cb
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);
	void check_remove_stock(Game &);

	bool _load_finish_planned_worker(PlannedWorkers & pw);
	void _update_planned_workers(Game &, PlannedWorkers & pw);
	void _update_all_planned_workers(Game &);

	WarehouseSupply       * m_supply;

	std::vector<StockPolicy> m_ware_policy;
	std::vector<StockPolicy> m_worker_policy;

	// Workers who live here at the moment
	typedef std::vector<Worker *> WorkerList;
	typedef std::map<Ware_Index, WorkerList> IncorporatedWorkers;
	IncorporatedWorkers        m_incorporated_workers;
	uint32_t                 * m_next_worker_without_cost_spawn;
	uint32_t                   m_next_military_act;
	uint32_t m_next_stock_remove_act;

	std::vector<PlannedWorkers> m_planned_workers;

	PortDock * m_portdock;
};

}

#endif
