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

#ifndef STORAGE_H
#define STORAGE_H

#include <vector>

#include "logic/requirements.h"
#include "logic/wareworker.h"
#include "logic/widelands.h"

namespace Widelands {

class Building;
class Worker;
class Editor_Game_Base;
class Game;
class WareInstance;
class WareList;
class Player;

/**
 *  A storage represents wares and workers stocks. It is owned by StrorageOwner buildings.
 * This interface contains various method used to deal with wares requests, prioriies
 * and disponibility.
 */
class Storage {
public:
	/**
	 * Each ware and worker type has an associated per-storage
	 * stock policy that defines whether it will be stocked by this
	 * warehouse.
	 *
	 * \note The values of this enum are written directly into savegames,
	 * so be careful when changing them.
	 */
	enum class StockPolicy : uint8_t {
		/**
		 * The default policy allows stocking wares without any special priority.
		 */
		Normal = 0,

		/**
		 * As long as there are warehouses with this policy for a ware, all
		 * available unstocked supplies will be transferred to warehouses
		 * with this policy.
		 */
		Prefer = 1,

		/**
		 * If a ware has this stock policy, no new items of this ware will enter
		 * the warehouse.
		 */
		DontStock = 2,

		/**
		 * Like \ref SP_DontStock, but in addition, existing stock of this ware
		 * will be transported out of the warehouse over time.
		 */
		Remove = 3,
	};

	/**
	 * \return the player that own this storage
	 */
	virtual Player & owner() const = 0;
	/**
	 * \return the wares present in this storage
	 */
	virtual const WareList & get_wares() const = 0;
	/**
	 * \return the workers present in this storage
	 */
	virtual const WareList & get_workers() const = 0;
	/**
	 * Increase the stock of the specified ware type
	 */
	virtual void insert_wares  (Ware_Index, uint32_t count) = 0;
	/**
	 * Decrease the stock of the specified ware type
	 */
	virtual void remove_wares  (Ware_Index, uint32_t count) = 0;
	/**
	 * Increase the stock of the specified worker type
	 */
	virtual void insert_workers(Ware_Index, uint32_t count) = 0;
	/**
	 * Decrease the stock of the specified worker type
	 */
	virtual void remove_workers(Ware_Index, uint32_t count) = 0;
	/**
	 * Get a specified ware from the storage.
	 * \return the wareinstance
	 */
	virtual WareInstance & launch_ware(Game &, Ware_Index) = 0;
	/**
	 * Launch a ware already instanciated
	 */
	virtual void do_launch_ware(Game &, WareInstance&) = 0;
	/**
	 * Adds the specified ware to the storage. The ware might
	 * be removed from the game following this call.
	 */
	virtual void incorporate_ware(Editor_Game_Base &, WareInstance &) = 0;
	/**
	 * Get the specified worker from the storage.
	 * \return the worker
	 */
	virtual Worker & launch_worker(Game &, Ware_Index, const Requirements & = Requirements()) = 0;
	/**
	 * Adds the specified worker to the storage. The worker might
	 * be removed from the game following this call.
	 */
	virtual void incorporate_worker(Editor_Game_Base &, Worker &) = 0;
	/**
	 * \return the storage stock policy for the specified ware
	 */
	virtual StockPolicy get_ware_policy(Ware_Index ware) const = 0;
	/**
	 * \return the storage stock policy for the specified worker
	 */
	virtual StockPolicy get_worker_policy(Ware_Index ware) const = 0;
	/**
	 * \return the sotrage stock policy for the specified ware or worker
	 */
	virtual StockPolicy get_stock_policy(WareWorker waretype, Ware_Index wareindex) const = 0;
	/**
	 * Set the storage stock policy for the specified ware
	 */
	virtual void set_ware_policy(Ware_Index ware, StockPolicy policy) = 0;
	/**
	 * Set the storage stock policy for the specified worker
	 */
	virtual void set_worker_policy(Ware_Index ware, StockPolicy policy) = 0;
	/**
	 * Return true if the storage has all requirement in stock to create
	 * the specified worker
	 */
	virtual bool can_create_worker(Game& game, Ware_Index ware_index) = 0;
	/**
	 * Create a worker.
	 */
	virtual void create_worker(Game& game, Ware_Index worker_idx) = 0;
	/**
	 * Return the amount of planned worker of the given worker idx
	 */
	virtual uint32_t get_planned_workers(Game &, Ware_Index index) const = 0;
	/**
	 * Plan to create the specified workers
	 */
	virtual void plan_workers(Game &, Ware_Index index, uint32_t amount) = 0;
	/**
	 * Calculate the supply of wares available to this warehouse in each of the
	 * buildcost items for the given worker.
	 *
	 * This is the current stock plus any incoming transfers.
	 */
	virtual std::vector<uint32_t> calc_available_for_worker(Game &, Ware_Index index) const = 0;
};

/**
 * A Storage owner holds a sorage instance.
 */
class StorageOwner {
public:
	/**
	 * Return the storage instance
	 */
	virtual Storage* get_storage() const = 0;
	/**
	 * Return the building owning the storage
	 */
	virtual Building* get_building() = 0;
};

}

#endif
