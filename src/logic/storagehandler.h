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

#ifndef STORAGEHANDLER_H
#define STORAGEHANDLER_H

#include <memory>

#include "economy/supply.h"
#include "logic/requirements.h"
#include "logic/storage.h"
#include "logic/warelist.h"

#define STORAGE_REMOVAL_INTERVAL 4000

namespace Widelands {

class Worker_Descr;

class Economy;
class StorageSupply;

class StorageHandler: public Storage {
friend struct Map_Buildingdata_Data_Packet;
public:
	// Spawning. vector is {next_act, interval, stock_max, [counter]}
	typedef std::map<Ware_Index, std::vector<uint32_t>> SpawnMapType;

	StorageHandler(Building& building);
	virtual ~StorageHandler();
	/**
	 * Must be called once
	 */
	void init(Editor_Game_Base &);
	/**
	 * To be called after loading
	 */
	void load_finish(Editor_Game_Base &);
	/**
	 * Cleanup handled objects and requests
	 */
	void cleanup(Editor_Game_Base &);
	/**
	 * Must be called regularly
	 * \return the time, from now, where a next act should occur
	 */
	uint32_t act(Game &);
	/**
	 * Change the economy
	 */
	void set_economy(Economy *);
	/**
	 * Return the number of workers satisfying the given requirement that this
	 * storage has in stock
	 */
	uint32_t count_workers(Game & game, Ware_Index ware, const Requirements & req = Requirements());
	/**
	 * Launch all workers in the storage.
	 * \param excluded : A WareList with some non-zero value for the
	 * worker type to exclude
	 */
	void launch_all_workers(Game& game, bool exp_only = false, const WareList& excluded = WareList());
	/**
	 * Introduce auto spawning of the specified ware
	 * \param idx The ware to autospawn
	 * \param interval The interval between spawns. default to 2500
	 * \param stock_max While the stock is above this value, spawning is disabled. Default to 100.
	 * 0 for infinite spawning
	 * \param counter The amount of wares to spawn. default to 0 (infinite).
	 * \param dont_excdeed if true (default), ware will be removed as well to not exceed stock_max
	 */
	void add_ware_spawn
		(Ware_Index idx, uint32_t interval = 2500, uint32_t stock_max = 100,
		 uint32_t counter = 0, bool dont_exceed = true);
	/**
	 * Disable autospawning of the specified ware
	 */
	void remove_ware_spawn(Ware_Index idx);
	bool is_ware_spawn_set(Ware_Index idx);
	/**
	 * Introduce auto spawning of the specified worker
	 * \param idx The worker to autospawn
	 * \param interval The interval between spawns. default to 2500 (2,5s)
	 * \param stock_max While the stock is above this value, spawning is disabled. Default to 100.
	 * 0 for infinite spawning
	 * \param counter The amount of workers to spawn. default to 0.
	 * \param dont_excdeed if true (default), ware will be removed as well to not exceed stock_max
	 */
	void add_worker_spawn
		(Ware_Index idx, uint32_t interval = 2500, uint32_t stock_max = 100,
		 uint32_t counter = 0,  bool dont_exceed = true);
	/**
	 * Remove autospawning of the specified worker
	 */
	void remove_worker_spawn(Ware_Index idx);
	bool is_worker_spawn_set(Ware_Index idx);
	/**
	 * «return the building associated with this storage
	 */
	Building& get_building() const {return m_building;}

	// Sorage implementation
	virtual Player & owner() const;
	virtual const WareList & get_wares() const;
	virtual const WareList & get_workers() const;
	virtual void insert_wares  (Ware_Index, uint32_t count);
	virtual void remove_wares  (Ware_Index, uint32_t count);
	virtual void insert_workers(Ware_Index, uint32_t count);
	virtual void remove_workers(Ware_Index, uint32_t count);
	virtual WareInstance & launch_ware(Game &, Ware_Index);
	virtual void do_launch_ware(Game &, WareInstance&);
	virtual void incorporate_ware(Editor_Game_Base &, WareInstance &);
	virtual Worker & launch_worker(Game &, Ware_Index, const Requirements & = Requirements());
	virtual void incorporate_worker(Editor_Game_Base &, Worker &);
	virtual StockPolicy get_ware_policy(Ware_Index ware) const;
	virtual StockPolicy get_worker_policy(Ware_Index ware) const;
	virtual StockPolicy get_stock_policy(WareWorker waretype, Ware_Index wareindex) const;
	virtual void set_ware_policy(Ware_Index ware, StockPolicy policy);
	virtual void set_worker_policy(Ware_Index ware, StockPolicy policy);
	virtual bool can_create_worker(Game& game, Ware_Index worker_idx);
	virtual void create_worker(Game& game, Ware_Index worker_idx);
	virtual uint32_t get_planned_workers(Game &, Ware_Index index) const;
	virtual void plan_workers(Game &, Ware_Index index, uint32_t amount);
	virtual std::vector<uint32_t> calc_available_for_worker(Game &, Ware_Index index) const;
private:
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
	/**
	 * Store attributes of stocked worker so they can be removed from the game
	 * without loosing experience & co informations
	 */
	struct StockedWorkerAtr {
		Ware_Index index;
		const Worker_Descr* descr;
		uint32_t atck_lvl;
		uint32_t evade_lvl;
		uint32_t defense_lvl;
		uint32_t exp_or_hp_lvl; // experience or hp level for soldiers
	};

	// return time for next act
	uint32_t update_spawns(Game& game, uint32_t gametime);
	// return time for next act
	uint32_t update_removals(Game & game, uint32_t gametime);
	void update_all_planned_workers(Game& game);
	void update_planned_workers(Game& game, PlannedWorkers& planned_workers);
	StockedWorkerAtr* store_worker_atr(Worker& w);
	Worker* create_with_atr(Game& game, StockedWorkerAtr atr);
	static void planned_worker_callback
		(Game            &       game,
		Request         &,
		Ware_Index        const ware,
		Worker          * const w,
		PlayerImmovable &       target);
	bool load_finish_planned_workers(PlannedWorkers & pw);

	//fields
	Building & m_building;
	std::unique_ptr<StorageSupply> m_supply;
	// policies
	std::vector<StockPolicy> m_ware_policy;
	std::vector<StockPolicy> m_worker_policy;
	// Spawning
	SpawnMapType m_spawn_wares;
	SpawnMapType m_spawn_workers;
	// ware removal
	uint32_t m_removal_next_act;
	// planned workers
	std::vector<PlannedWorkers> m_planned_workers;
	// Attributes of stocked workers
	std::vector<StockedWorkerAtr> m_stocked_workers_atr;
};

/**
 * A supply implementation for storages
 */
class StorageSupply : public Supply {
public:
	StorageSupply(StorageHandler* handler);
	virtual ~StorageSupply();

	void set_economy(Economy *);

	void set_nrwares  (Ware_Index);
	void set_nrworkers(Ware_Index);

	const WareList & get_wares  () const {return m_wares;}
	const WareList & get_workers() const {return m_workers;}
	uint32_t stock_wares  (Ware_Index const i) const {
		return m_wares  .stock(i);
	}
	uint32_t stock_workers(Ware_Index const i) const {
		return m_workers.stock(i);
	}
	void add_wares     (Ware_Index, uint32_t count);
	void remove_wares  (Ware_Index, uint32_t count);
	void add_workers   (Ware_Index, uint32_t count);
	void remove_workers(Ware_Index, uint32_t count);

	// Supply implementation
	virtual PlayerImmovable * get_position(Game &);
	virtual bool is_active() const throw ();
	virtual bool has_storage() const throw ();
	virtual void get_ware_type(WareWorker & type, Ware_Index & ware) const;

	virtual void send_to_storage(Game &, StorageOwner * storage_owner);
	virtual uint32_t nr_supplies(const Game &, const Request &) const;
	virtual WareInstance & launch_item(Game &, const Request &);
	virtual Worker & launch_worker(Game &, const Request &);
private:
	StorageHandler & m_handler;
	Economy   * m_economy;
	WareList    m_wares;
	WareList    m_workers; //  we use this to keep the soldiers
};

}

#endif
