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

#ifndef WL_ECONOMY_ECONOMY_H
#define WL_ECONOMY_ECONOMY_H

#include <memory>
#include <set>
#include <vector>

#include <boost/function.hpp>
#include <boost/utility.hpp>

#include "base/macros.h"
#include "economy/supply.h"
#include "economy/supply_list.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/tribes/wareworker.h"
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
class Economy {
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
	struct TargetQuantity {
		Quantity permanent;
		Time     last_modified;
	};

	Economy(Player &);
	~Economy();

	Player & owner() const {return owner_;}

	static void check_merge(Flag &, Flag &);
	static void check_split(Flag &, Flag &);

	bool find_route
		(Flag & start, Flag & end,
		 Route * route,
		 WareWorker type,
		 int32_t cost_cutoff = -1);

	using WarehouseAcceptFn = boost::function<bool (Warehouse &)>;
	Warehouse * find_closest_warehouse
		(Flag & start, WareWorker type = wwWORKER, Route * route = nullptr,
		 uint32_t cost_cutoff = 0,
		 const WarehouseAcceptFn & acceptfn = WarehouseAcceptFn());

	std::vector<Flag *>::size_type get_nrflags() const {return flags_.size();}
	void    add_flag(Flag &);
	void remove_flag(Flag &);

	// Returns an arbitrary flag or nullptr if this is an economy without flags
	// (i.e. an Expedition ship).
	Flag* get_arbitrary_flag();

	void set_ware_target_quantity  (DescriptionIndex, Quantity, Time);
	void set_worker_target_quantity(DescriptionIndex, Quantity, Time);

	void    add_wares  (DescriptionIndex, Quantity count = 1);
	void remove_wares  (DescriptionIndex, Quantity count = 1);

	void    add_workers(DescriptionIndex, Quantity count = 1);
	void remove_workers(DescriptionIndex, Quantity count = 1);

	void    add_warehouse(Warehouse &);
	void remove_warehouse(Warehouse &);
	const std::vector<Warehouse *>& warehouses() const {return warehouses_;}

	void    add_request(Request &);
	void remove_request(Request &);

	void    add_supply(Supply &);
	void remove_supply(Supply &);

	/// information about this economy
	Quantity stock_ware  (DescriptionIndex const i) {
		return wares_  .stock(i);
	}
	Quantity stock_worker(DescriptionIndex const i) {
		return workers_.stock(i);
	}

	/// Whether the economy needs more of this ware type.
	/// Productionsites may ask this before they produce, to avoid depleting a
	/// ware type by overproducing another from it.
	bool needs_ware(DescriptionIndex) const;

	/// Whether the economy needs more of this worker type.
	/// Productionsites may ask this before they produce, to avoid depleting a
	/// ware type by overproducing a worker type from it.
	bool needs_worker(DescriptionIndex) const;

	const TargetQuantity & ware_target_quantity  (DescriptionIndex const i) const {
		return ware_target_quantities_[i];
	}
	TargetQuantity       & ware_target_quantity  (DescriptionIndex const i)       {
		return ware_target_quantities_[i];
	}
	const TargetQuantity & worker_target_quantity(DescriptionIndex const i) const {
		return worker_target_quantities_[i];
	}
	TargetQuantity       & worker_target_quantity(DescriptionIndex const i)       {
		return worker_target_quantities_[i];
	}

	void show_options_window();
	UI::UniqueWindow::Registry& optionswindow_registry() {
		return optionswindow_registry_;
	}

	const WareList & get_wares  () const {return wares_;}
	const WareList & get_workers() const {return workers_;}

	///< called by \ref Cmd_Call_Economy_Balance
	void balance(uint32_t timerid);

	void rebalance_supply() {_start_request_timer();}

private:

	// This structs is to store distance from supply to request(or), but to allow unambiguous
	// sorting if distances are the same, we use also serial number of provider and type of provider (flag,
	// warehouse)
	struct UniqueDistance {
		bool operator<(const UniqueDistance& other) const {
       		return std::forward_as_tuple(distance, serial, provider_type)
				<
				std::forward_as_tuple(other.distance, other.serial, other.provider_type);
		}

		uint32_t distance;
		uint32_t serial;
		SupplyProviders provider_type;
	};

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
	void _create_requested_worker(Game &, DescriptionIndex);

	bool   _has_request(Request &);

/*************/
/* Variables */
/*************/
	using RequestList = std::vector<Request *>;

	Player & owner_;

	using Flags = std::vector<Flag *>;
	Flags flags_;
	WareList wares_;     ///< virtual storage with all wares in this Economy
	WareList workers_;   ///< virtual storage with all workers in this Economy
	std::vector<Warehouse *> warehouses_;

	RequestList requests_; ///< requests
	SupplyList supplies_;

	TargetQuantity        * ware_target_quantities_;
	TargetQuantity        * worker_target_quantities_;
	Router                 * router_;

	using SplitPair = std::pair<OPtr<Flag>, OPtr<Flag>>;
	std::vector<SplitPair> split_checks_;

	/**
	 * ID for the next request balancing timer. Used to throttle
	 * excessive calls to the request/supply balancing logic.
	 */
	uint32_t request_timerid_;

	static std::unique_ptr<Soldier> soldier_prototype_;
	UI::UniqueWindow::Registry optionswindow_registry_;

	// 'list' of unique providers
	std::map<UniqueDistance, Supply*> available_supplies;

	DISALLOW_COPY_AND_ASSIGN(Economy);
};

}

#endif  // end of include guard: WL_ECONOMY_ECONOMY_H
