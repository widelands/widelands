/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#ifndef ECONOMY_H
#define ECONOMY_H

#include <memory>
#include <set>
#include <vector>

#include <boost/function.hpp>
#include <boost/utility.hpp>

#include "logic/instances.h"
#include "logic/warelist.h"
#include "logic/wareworker.h"
#include "economy/supply_list.h"
#include "ui_basic/unique_window.h"


namespace Widelands {

class Game;
class Player;
class Soldier;
class Warehouse;
struct Flag;
struct RSPairStruct;
class Request;
struct Route;
struct Router;
struct Supply;

/**
 * Each Economy represents all building and flags, which are connected over the same
 * street network. In general a player can own multiple Economys, which
 * operate independently from each other.
 *
 * Every Economy tracks the amount of wares inside of it and how high the
 * demand for each ware is.
 *
 * \paragraph Merging and splitting
 *
 * During the course of a game Economy objects can be merged when new roads and ports are created,
 * or split when roads and ports are destroyed.
 *
 * Splitting and merging economies are relatively expensive operations,
 * and in particular during game shutdown or when a large network is destroyed
 * in a military operation, cascading economy splits could take a lot of processing time.
 * For this reason, economies do not split immediately when a road is destroyed,
 * but instead keep track of where a potential split occured and evaluate the split lazily.
 *
 * This means that two flags which are connected by the road (and seafaring) network
 * are \b always in the same economy, but two flags in the same economy are not always
 * connected by roads or the seafaring network - though of course, most code operates
 * on the assumption that they are, with fallbacks for when they aren't.
 */
class Economy : boost::noncopyable {
public:
	friend class EconomyDataPacket;

	/// Configurable target quantity for the supply of a ware type in the
	/// economy.
	///
	/// This affects the result of \ref needs_ware and thereby the demand checks
	/// in production programs. A ware type is considered to be needed if there
	/// are less than the permanent target quantity stored in warehouses in the
	/// economy.
	///
	/// The last_modified time is used to determine which setting to use when
	/// economies are merged. The setting that was modified most recently will
	/// be used for the merged economy.
	struct Target_Quantity {
		uint32_t permanent;
		Time     last_modified;
	};

	Economy(Player &);
	~Economy();

	Player & owner() const {return m_owner;}

	static void check_merge(Flag &, Flag &);
	static void check_split(Flag &, Flag &);

	bool find_route
		(Flag & start, Flag & end,
		 Route * route,
		 WareWorker type,
		 int32_t cost_cutoff = -1);

	typedef boost::function<bool (Warehouse &)> WarehouseAcceptFn;
	Warehouse * find_closest_warehouse
		(Flag & start, WareWorker type = wwWORKER, Route * route = nullptr,
		 uint32_t cost_cutoff = 0,
		 const WarehouseAcceptFn & acceptfn = WarehouseAcceptFn());

	std::vector<Flag *>::size_type get_nrflags() const {return m_flags.size();}
	void    add_flag(Flag &);
	void remove_flag(Flag &);

	// Returns an arbitrary flag or nullptr if this is an economy without flags
	// (i.e. an Expedition ship).
	Flag* get_arbitrary_flag();

	void set_ware_target_quantity  (Ware_Index, uint32_t, Time);
	void set_worker_target_quantity(Ware_Index, uint32_t, Time);

	void    add_wares  (Ware_Index, uint32_t count = 1);
	void remove_wares  (Ware_Index, uint32_t count = 1);

	void    add_workers(Ware_Index, uint32_t count = 1);
	void remove_workers(Ware_Index, uint32_t count = 1);

	void    add_warehouse(Warehouse &);
	void remove_warehouse(Warehouse &);
	const std::vector<Warehouse *>& warehouses() const {return m_warehouses;}

	void    add_request(Request &);
	void remove_request(Request &);

	void    add_supply(Supply &);
	void remove_supply(Supply &);

	/// information about this economy
	WareList::count_type stock_ware  (Ware_Index const i) {
		return m_wares  .stock(i);
	}
	WareList::count_type stock_worker(Ware_Index const i) {
		return m_workers.stock(i);
	}

	/// Whether the economy needs more of this ware type.
	/// Productionsites may ask this before they produce, to avoid depleting a
	/// ware type by overproducing another from it.
	bool needs_ware(Ware_Index) const;

	/// Whether the economy needs more of this worker type.
	/// Productionsites may ask this before they produce, to avoid depleting a
	/// ware type by overproducing a worker type from it.
	bool needs_worker(Ware_Index) const;

	const Target_Quantity & ware_target_quantity  (Ware_Index const i) const {
		return m_ware_target_quantities[i.value()];
	}
	Target_Quantity       & ware_target_quantity  (Ware_Index const i)       {
		return m_ware_target_quantities[i.value()];
	}
	const Target_Quantity & worker_target_quantity(Ware_Index const i) const {
		return m_worker_target_quantities[i.value()];
	}
	Target_Quantity       & worker_target_quantity(Ware_Index const i)       {
		return m_worker_target_quantities[i.value()];
	}

	void show_options_window();
	UI::UniqueWindow::Registry m_optionswindow_registry;


	const WareList & get_wares  () const {return m_wares;}
	const WareList & get_workers() const {return m_workers;}

	///< called by \ref Cmd_Call_Economy_Balance
	void balance(uint32_t timerid);

	void rebalance_supply() {_start_request_timer();}

private:
/*************/
/* Functions */
/*************/
	void _remove_flag(Flag &);
	void _reset_all_pathfinding_cycles();

	void _merge(Economy &);
	void _check_splits();
	void _split(const std::set<OPtr<Flag> > &);

	void _start_request_timer(int32_t delta = 200);

	Supply * _find_best_supply(Game &, const Request &, int32_t & cost);
	void _process_requests(Game &, RSPairStruct &);
	void _balance_requestsupply(Game &);
	void _handle_active_supplies(Game &);
	void _create_requested_workers(Game &);
	void _create_requested_worker(Game &, Ware_Index);

	bool   _has_request(Request &);

/*************/
/* Variables */
/*************/
	typedef std::vector<Request *> RequestList;

	Player & m_owner;

	typedef std::vector<Flag *> Flags;
	Flags m_flags;
	WareList m_wares;     ///< virtual storage with all wares in this Economy
	WareList m_workers;   ///< virtual storage with all workers in this Economy
	std::vector<Warehouse *> m_warehouses;

	RequestList m_requests; ///< requests
	SupplyList m_supplies;

	Target_Quantity        * m_ware_target_quantities;
	Target_Quantity        * m_worker_target_quantities;
	Router                 * m_router;

	typedef std::pair<OPtr<Flag>, OPtr<Flag> > SplitPair;
	std::vector<SplitPair> m_split_checks;

	/**
	 * ID for the next request balancing timer. Used to throttle
	 * excessive calls to the request/supply balancing logic.
	 */
	uint32_t m_request_timerid;

	static std::unique_ptr<Soldier> m_soldier_prototype;

};

}

#endif
