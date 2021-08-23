/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#include <functional>
#include <memory>

#include "base/macros.h"
#include "economy/supply.h"
#include "economy/supply_list.h"
#include "logic/map_objects/map_object.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"

namespace Widelands {

class Economy;
struct Flag;
class ProductionSite;
struct RSPairStruct;
struct Route;
struct Router;
class WorkerDescr;

struct NoteEconomy {
	CAN_BE_SENT_AS_NOTE(NoteId::Economy)

	// When 2 economies have been merged, this is the economy number that has
	// been removed, while the other one is the number of the resulting economy.
	// For all other messages old_economy == new_economy.
	Widelands::Serial old_economy;
	Widelands::Serial new_economy;

	enum class Action { kMerged, kDeleted };
	const Action action;
};

constexpr Quantity kEconomyTargetInfinity = std::numeric_limits<Quantity>::max();

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
 *
 * Everything that has economies now has one economy that handles only wares and one that handles
 * only workers. The reason for this design is that two road networks connected only by ferries
 * are the same economy from the ware point of view, but separate economies from a worker's point
 * of view. This fix involves the least amount of code duplication.
 */
class Economy {
public:
	friend class EconomyDataPacket;

	// Initialize the global serial on game start
	static void initialize_serial();

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
		Time last_modified;
	};

	explicit Economy(Player&, WareWorker);
	explicit Economy(Player&, Serial serial, WareWorker);  // For saveloading
	~Economy();

	Serial serial() const {
		return serial_;
	}

	Player& owner() const {
		return owner_;
	}

	WareWorker type() const {
		return type_;
	}

	static void check_merge(const Flag&, const Flag&, WareWorker);
	static void check_split(Flag&, Flag&, WareWorker);

	bool find_route(Flag& start, Flag& end, Route* route, int32_t cost_cutoff = -1);

	using WarehouseAcceptFn = std::function<bool(Warehouse&)>;
	Warehouse* find_closest_warehouse(Flag& start,
	                                  Route* route = nullptr,
	                                  uint32_t cost_cutoff = 0,
	                                  const WarehouseAcceptFn& acceptfn = WarehouseAcceptFn());

	std::vector<Flag*>::size_type get_nrflags() const {
		return flags_.size();
	}
	void add_flag(Flag&);
	void remove_flag(Flag&);

	// Returns an arbitrary flag or nullptr if this is an economy without flags
	// (i.e. an Expedition ship).
	Flag* get_arbitrary_flag(const Economy* other = nullptr);

	void set_target_quantity(WareWorker economy_type, DescriptionIndex, Quantity, Time);

	void
	add_wares_or_workers(DescriptionIndex, Quantity count = 1, Economy* other_economy = nullptr);
	void remove_wares_or_workers(DescriptionIndex, Quantity count = 1);

	void add_warehouse(Warehouse&);
	void remove_warehouse(Warehouse&);
	const std::vector<Warehouse*>& warehouses() const {
		return warehouses_;
	}

	void add_request(Request&);
	void remove_request(Request&);

	void add_supply(Supply&);
	void remove_supply(Supply&);

	/// information about this economy
	Quantity stock_ware_or_worker(DescriptionIndex const i) {
		return wares_or_workers_.stock(i);
	}

	/// Whether the economy needs more of this ware/worker type.
	/// Productionsites may ask this before they produce, to avoid depleting a
	/// ware type by overproducing another from it.
	bool needs_ware_or_worker(DescriptionIndex) const;

	const TargetQuantity& target_quantity(DescriptionIndex const i) const {
		return target_quantities_[i];
	}
	TargetQuantity& target_quantity(DescriptionIndex const i) {
		return target_quantities_[i];
	}

	void* get_options_window() const {
		return options_window_;
	}
	void set_options_window(void* window) {
		options_window_ = window;
	}

	const WareList& get_wares_or_workers() const {
		return wares_or_workers_;
	}

	// Checks whether this economy contains a building of the specified type
	bool has_building(DescriptionIndex) const;
	/**
	 * Of all occupied ProductionSites of the specified type in this economy,
	 * find the one that is closest to the specified flag and return it.
	 * Stopped buildings are also accepted. If `check_inputqueues` is `true`,
	 * buildings with all inputqueues set to zero capacity are ignored.
	 * If no matching site is found, nullptr is returned.
	 */
	ProductionSite*
	find_closest_occupied_productionsite(const Flag&, DescriptionIndex, bool check_inputqueues);

	///< called by \ref Cmd_Call_Economy_Balance
	void balance(uint32_t timerid);

	void rebalance_supply() {
		start_request_timer();
	}

protected:
	static Serial last_economy_serial_;

private:
	// This structs is to store distance from supply to request(or), but to allow unambiguous
	// sorting if distances are the same, we use also serial number of provider and type of provider
	// (flag,
	// warehouse)
	struct UniqueDistance {
		bool operator<(const UniqueDistance& other) const {
			return std::forward_as_tuple(distance, serial, provider_type) <
			       std::forward_as_tuple(other.distance, other.serial, other.provider_type);
		}

		uint32_t distance;
		uint32_t serial;
		SupplyProviders provider_type;
	};

	/*************/
	/* Functions */
	/*************/
	void do_remove_flag(Flag&);
	void reset_all_pathfinding_cycles();

	void merge(Economy&);
	void check_splits();
	void split(const std::set<OPtr<Flag>>&);

	void start_request_timer(const Duration& delta = Duration(200));

	Supply* find_best_supply(Game&, const Request&, int32_t& cost);
	void process_requests(Game&, RSPairStruct* supply_pairs);
	void balance_requestsupply(Game&);
	void handle_active_supplies(Game&);
	void create_requested_workers(Game&);
	void create_requested_worker(Game&, DescriptionIndex);

	bool has_request(Request&) const;

	/*************/
	/* Variables */
	/*************/
	using RequestList = std::vector<Request*>;

	const Serial serial_;

	Player& owner_;

	using Flags = std::vector<Flag*>;
	Flags flags_;
	WareList wares_or_workers_;  ///< virtual storage with all wares/workers in this Economy
	std::vector<Warehouse*> warehouses_;

	WareWorker type_;  ///< whether we are a WareEconomy or a WorkerEconomy

	RequestList requests_;  ///< requests
	SupplyList supplies_;

	TargetQuantity* target_quantities_;
	std::unique_ptr<Router> router_;

	using SplitPair = std::pair<OPtr<Flag>, OPtr<Flag>>;
	std::vector<SplitPair> split_checks_;

	/**
	 * ID for the next request balancing timer. Used to throttle
	 * excessive calls to the request/supply balancing logic.
	 */
	uint32_t request_timerid_;

	static std::unique_ptr<Worker> soldier_prototype_;
	static Worker& soldier_prototype(const WorkerDescr* = nullptr);

	// This is always an EconomyOptionsWindow* (or nullptr) but I don't want a wui dependency here.
	// We cannot use UniqueWindow to make sure an economy never has two windows because the serial
	// may change when merging while the window is open, so we have to keep track of it here.
	void* options_window_;

	// 'list' of unique providers
	std::map<UniqueDistance, Supply*> available_supplies_;

	// Helper function for `find_closest_occupied_productionsite()`
	bool check_building_can_start_working(const ProductionSite&, bool check_inputqueues);

	DISALLOW_COPY_AND_ASSIGN(Economy);
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_ECONOMY_H
