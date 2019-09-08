/*
 * Copyright (C) 2004-2019 by the Widelands Development Team
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

#include "economy/economy.h"

#include <memory>
#include <unordered_set>

#include <boost/bind.hpp>

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/cmd_call_economy_balance.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/route.h"
#include "economy/routeastar.h"
#include "economy/router.h"
#include "economy/warehousesupply.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"

namespace Widelands {

Serial Economy::last_economy_serial_ = 0;

void Economy::initialize_serial() {
	log("Initializing economy serial\n");
	last_economy_serial_ = 0;
}

Economy::Economy(Player& player) : Economy(player, last_economy_serial_++) {
}

Economy::Economy(Player& player, Serial init_serial)
   : serial_(init_serial), owner_(player), request_timerid_(0), options_window_(nullptr) {
	last_economy_serial_ = std::max(last_economy_serial_, serial_ + 1);
	const TribeDescr& tribe = player.tribe();
	DescriptionIndex const nr_wares = player.egbase().tribes().nrwares();
	DescriptionIndex const nr_workers = player.egbase().tribes().nrworkers();
	wares_.set_nrwares(nr_wares);
	workers_.set_nrwares(nr_workers);

	ware_target_quantities_ = new TargetQuantity[nr_wares];
	for (DescriptionIndex i = 0; i < nr_wares; ++i) {
		TargetQuantity tq;
		if (tribe.has_ware(i)) {
			tq.permanent = tribe.get_ware_descr(i)->default_target_quantity(tribe.name());
		} else {
			tq.permanent = 0;
		}
		tq.last_modified = 0;
		ware_target_quantities_[i] = tq;
	}
	worker_target_quantities_ = new TargetQuantity[nr_workers];
	for (DescriptionIndex i = 0; i < nr_workers; ++i) {
		TargetQuantity tq;
		tq.permanent = tribe.get_worker_descr(i)->default_target_quantity();
		tq.last_modified = 0;
		worker_target_quantities_[i] = tq;
	}

	router_.reset(new Router(boost::bind(&Economy::reset_all_pathfinding_cycles, this)));
}

Economy::~Economy() {
	Notifications::publish(NoteEconomy{serial_, serial_, NoteEconomy::Action::kDeleted});

	if (requests_.size())
		log("Warning: Economy still has requests left on destruction\n");
	if (flags_.size())
		log("Warning: Economy still has flags left on destruction\n");
	if (warehouses_.size())
		log("Warning: Economy still has warehouses left on destruction\n");

	delete[] ware_target_quantities_;
	delete[] worker_target_quantities_;
}

/**
 * \return an arbitrary flag in this economy.
 */
Flag* Economy::get_arbitrary_flag() {
	if (flags_.empty())
		return nullptr;

	return flags_[0];
}

/**
 * Two flags have been connected; check whether their economies should be
 * merged.
 * Since we could merge into both directions, we preserve the economy that is
 * currently bigger (should be more efficient).
 */
void Economy::check_merge(Flag& f1, Flag& f2) {
	Economy* e1 = f1.get_economy();
	Economy* e2 = f2.get_economy();
	if (e1 != e2) {
		if (e1->get_nrflags() < e2->get_nrflags())
			std::swap(e1, e2);
		e1->merge(*e2);
	}
}

/**
 * Notify the economy that there may no longer be a connection between
 * the given flags in the road and seafaring network.
 */
void Economy::check_split(Flag& f1, Flag& f2) {
	assert(&f1 != &f2);
	assert(f1.get_economy() == f2.get_economy());

	Economy* e = f1.get_economy();
	// No economy in the editor.
	if (!e)
		return;

	e->split_checks_.push_back(std::make_pair(OPtr<Flag>(&f1), OPtr<Flag>(&f2)));
	e->rebalance_supply();  // the real split-checking is done during rebalance
}

void Economy::check_splits() {
	EditorGameBase& egbase = owner().egbase();
	while (split_checks_.size()) {
		Flag* f1 = split_checks_.back().first.get(egbase);
		Flag* f2 = split_checks_.back().second.get(egbase);
		split_checks_.pop_back();

		if (!f1 || !f2) {
			if (!f1 && !f2)
				continue;
			if (!f1)
				f1 = f2;
			if (f1->get_economy() != this)
				continue;

			// Handle the case when two or more roads are removed simultaneously
			RouteAStar<AStarZeroEstimator> astar(*router_, wwWORKER, AStarZeroEstimator());
			astar.push(*f1);
			std::set<OPtr<Flag>> reachable;
			while (RoutingNode* current = astar.step())
				reachable.insert(&current->base_flag());
			if (reachable.size() != flags_.size())
				split(reachable);
			continue;
		}

		// If one (or both) of the flags have already been split off, we do not need to re-check
		if (f1->get_economy() != this || f2->get_economy() != this)
			continue;

		// Start an A-star searches from f1 with a heuristic bias towards f2,
		// because we do not need to do anything if f1 is still connected to f2.
		// If f2 is not reached by the search, split off all the nodes that have been
		// reached from f1. These nodes induce a connected subgraph.
		// This means that the newly created economy, which contains all the
		// flags that have been split, is already connected.
		RouteAStar<AStarEstimator> astar(
		   *router_, wwWORKER, AStarEstimator(*egbase.mutable_map(), *f2));
		astar.push(*f1);
		std::set<OPtr<Flag>> reachable;

		for (;;) {
			RoutingNode* current = astar.step();
			if (!current) {
				split(reachable);
				break;
			} else if (current == f2)
				break;
			reachable.insert(&current->base_flag());
		}
	}
}

/**
 * Calculate a route between two flags.
 *
 * This functionality has been moved to Router(). This is currently
 * merely a delegator.
 */
bool Economy::find_route(
   Flag& start, Flag& end, Route* const route, WareWorker const type, int32_t const cost_cutoff) {
	assert(start.get_economy() == this);
	assert(end.get_economy() == this);
	return router_->find_route(
	   start, end, route, type, cost_cutoff, *owner().egbase().mutable_map());
}

struct ZeroEstimator {
	int32_t operator()(RoutingNode& /* node */) const {
		return 0;
	}
};

/**
 * Find the warehouse closest to the given starting flag.
 *
 * If the search was successful and \p route is non-null,
 * a route is also computed.
 *
 * \param start starting flag
 * \param type whether to path-find as if the path were for a ware
 * \param route if non-null, fill in a route to the warehouse
 * \param cost_cutoff if positive, find paths of at most
 * that length (in milliseconds)
 */
Warehouse* Economy::find_closest_warehouse(Flag& start,
                                           WareWorker type,
                                           Route* route,
                                           uint32_t cost_cutoff,
                                           const Economy::WarehouseAcceptFn& acceptfn) {
	if (!warehouses().size())
		return nullptr;

	// A-star with zero estimator = Dijkstra
	RouteAStar<ZeroEstimator> astar(*router_, type);
	astar.push(start);

	while (RoutingNode* current = astar.step()) {
		if (cost_cutoff && current->mpf_realcost > static_cast<int32_t>(cost_cutoff))
			return nullptr;

		Flag& flag = current->base_flag();
		if (upcast(Warehouse, warehouse, flag.get_building())) {
			if (!acceptfn || acceptfn(*warehouse)) {
				if (route)
					astar.routeto(flag, *route);
				return warehouse;
			}
		}
	}

	return nullptr;
}

/**
 * Add a flag to the flag array.
 * Only call from Flag init and split/merger code!
 */
void Economy::add_flag(Flag& flag) {
	assert(flag.get_economy() == nullptr);

	flags_.push_back(&flag);
	flag.set_economy(this);

	flag.reset_path_finding_cycle();
}

/**
 * Remove a flag from the flag array.
 * Only call from Flag cleanup and split/merger code!
 */
void Economy::remove_flag(Flag& flag) {
	assert(flag.get_economy() == this);

	do_remove_flag(flag);

	// automatically delete the economy when it becomes empty.
	if (flags_.empty()) {
		owner_.remove_economy(serial_);
	}
}

/**
 * Remove the flag, but don't delete the economy automatically.
 * This is called from the merge code.
 */
void Economy::do_remove_flag(Flag& flag) {
	flag.set_economy(nullptr);

	// fast remove
	for (Flags::iterator flag_iter = flags_.begin(); flag_iter != flags_.end(); ++flag_iter) {
		if (*flag_iter == &flag) {
			*flag_iter = *(flags_.end() - 1);
			return flags_.pop_back();
		}
	}
	throw wexception("trying to remove nonexistent flag");
}

/**
 * Callback for the incredibly rare case that the \ref Router pathfinding
 * cycle wraps around.
 */
void Economy::reset_all_pathfinding_cycles() {
	for (Flag* flag : flags_) {
		flag->reset_path_finding_cycle();
	}
}

/**
 * Set the target quantities for the given DescriptionIndex to the
 * numbers given in permanent. Also update the last
 * modification time.
 *
 * This is called from Cmd_ResetTargetQuantity and Cmd_SetTargetQuantity
 */
void Economy::set_ware_target_quantity(DescriptionIndex const ware_type,
                                       Quantity const permanent,
                                       Time const mod_time) {
	assert(owner().egbase().tribes().ware_exists(ware_type));
	TargetQuantity& tq = ware_target_quantities_[ware_type];
	tq.permanent = permanent;
	tq.last_modified = mod_time;
}

void Economy::set_worker_target_quantity(DescriptionIndex const ware_type,
                                         Quantity const permanent,
                                         Time const mod_time) {
	assert(owner().egbase().tribes().worker_exists(ware_type));
	TargetQuantity& tq = worker_target_quantities_[ware_type];
	tq.permanent = permanent;
	tq.last_modified = mod_time;
}

/**
 * Call this whenever some entity created a ware, e.g. when a lumberjack
 * has felled a tree.
 * This is also called when a ware is added to the economy through trade or
 * a merger.
 */
void Economy::add_wares(DescriptionIndex const id, Quantity const count) {
	wares_.add(id, count);
	start_request_timer();

	// TODO(unknown): add to global player inventory?
}
void Economy::add_workers(DescriptionIndex const id, Quantity const count) {
	workers_.add(id, count);
	start_request_timer();

	// TODO(unknown): add to global player inventory?
}

/**
 * Call this whenever a ware is destroyed or consumed, e.g. food has been
 * eaten or a warehouse has been destroyed.
 * This is also called when a ware is removed from the economy through trade or
 * a split of the Economy.
 */
void Economy::remove_wares(DescriptionIndex const id, Quantity const count) {
	assert(owner_.egbase().tribes().ware_exists(id));
	wares_.remove(id, count);

	// TODO(unknown): remove from global player inventory?
}

/**
 * Call this whenever a worker is destroyed.
 * This is also called when a worker is removed from the economy through
 * a split of the Economy.
 */
void Economy::remove_workers(DescriptionIndex const id, Quantity const count) {
	workers_.remove(id, count);

	// TODO(unknown): remove from global player inventory?
}

/**
 * Add the warehouse to our list of warehouses.
 * This also adds the wares in the warehouse to the economy. However, if wares
 * are added to the warehouse in the future, add_wares() must be called.
 */
void Economy::add_warehouse(Warehouse& wh) {
	warehouses_.push_back(&wh);
}

/**
 * Remove the warehouse and its wares from the economy.
 */
void Economy::remove_warehouse(Warehouse& wh) {
	for (size_t i = 0; i < warehouses_.size(); ++i)
		if (warehouses_[i] == &wh) {
			warehouses_[i] = *warehouses_.rbegin();
			warehouses_.pop_back();
			return;
		}

	//  This assert was modified, since on loading, warehouses might try to
	//  remove themselves from their own economy, though they weren't added
	//  (since they weren't initialized)
	assert(warehouses_.empty());
}

/**
 * Consider the request, try to fulfill it immediately or queue it for later.
 * Important: This must only be called by the \ref Request class.
 */
void Economy::add_request(Request& req) {
	assert(req.is_open());
	assert(!has_request(req));

	assert(&owner());

	requests_.push_back(&req);

	// Try to fulfill the request
	start_request_timer();
}

/**
 * \return true if the given Request is registered with the \ref Economy, false
 * otherwise
 */
bool Economy::has_request(Request& req) {
	return std::find(requests_.begin(), requests_.end(), &req) != requests_.end();
}

/**
 * Remove the request from this economy.
 * Important: This must only be called by the \ref Request class.
 */
void Economy::remove_request(Request& req) {
	RequestList::iterator const it = std::find(requests_.begin(), requests_.end(), &req);

	if (it == requests_.end()) {
		FORMAT_WARNINGS_OFF
		log("WARNING: remove_request(%p) not in list\n", &req);
		FORMAT_WARNINGS_ON
		return;
	}

	*it = *requests_.rbegin();

	requests_.pop_back();
}

/**
 * Add a supply to our list of supplies.
 */
void Economy::add_supply(Supply& supply) {
	supplies_.add_supply(supply);
	start_request_timer();
}

/**
 * Remove a supply from our list of supplies.
 */
void Economy::remove_supply(Supply& supply) {
	supplies_.remove_supply(supply);
}

bool Economy::needs_ware(DescriptionIndex const ware_type) const {
	Quantity const t = ware_target_quantity(ware_type).permanent;

	// we have a target quantity set
	if (t > 0) {
		Quantity quantity = 0;
		for (const Warehouse* wh : warehouses_) {
			quantity += wh->get_wares().stock(ware_type);
			if (t <= quantity)
				return false;
		}
		return true;

		// we have target quantity set to 0, we need to check if there is an open request
	} else {
		for (const Request* temp_req : requests_) {
			const Request& req = *temp_req;

			if (req.get_type() == wwWARE && req.get_index() == ware_type)
				return true;
		}
		return false;
	}
}

bool Economy::needs_worker(DescriptionIndex const worker_type) const {
	Quantity const t = worker_target_quantity(worker_type).permanent;

	// we have a target quantity set
	if (t > 0) {
		Quantity quantity = 0;
		for (const Warehouse* wh : warehouses_) {
			quantity += wh->get_workers().stock(worker_type);
			if (t <= quantity)
				return false;
		}
		return true;

		// we have target quantity set to 0, we need to check if there is an open request
	} else {
		for (const Request* temp_req : requests_) {
			const Request& req = *temp_req;

			if (req.get_type() == wwWORKER && req.get_index() == worker_type)
				return true;
		}
		return false;
	}
}

/**
 * Add e's flags to this economy.
 *
 * Also transfer all wares and wares request. Try to resolve the new ware
 * requests if possible.
 */
void Economy::merge(Economy& e) {
	for (const DescriptionIndex& ware_index : owner_.tribe().wares()) {
		TargetQuantity other_tq = e.ware_target_quantities_[ware_index];
		TargetQuantity& this_tq = ware_target_quantities_[ware_index];
		if (this_tq.last_modified < other_tq.last_modified) {
			this_tq = other_tq;
		}
	}

	for (const DescriptionIndex& worker_index : owner_.tribe().workers()) {
		TargetQuantity other_tq = e.worker_target_quantities_[worker_index];
		TargetQuantity& this_tq = worker_target_quantities_[worker_index];
		if (this_tq.last_modified < other_tq.last_modified) {
			this_tq = other_tq;
		}
	}

	//  If the options window for e is open, but not the one for this, the user
	//  should still have an options window after the merge.
	if (e.get_options_window() && !get_options_window()) {
		Notifications::publish(NoteEconomy{e.serial(), serial_, NoteEconomy::Action::kMerged});
	}

	for (std::vector<Flag*>::size_type i = e.get_nrflags() + 1; --i;) {
		assert(i == e.get_nrflags());

		Flag& flag = *e.flags_[0];

		e.do_remove_flag(flag);  // do not delete other economy yet!
		add_flag(flag);
	}

	// Remember that the other economy may not have been connected before the merge
	split_checks_.insert(split_checks_.end(), e.split_checks_.begin(), e.split_checks_.end());
	owner_.remove_economy(e.serial());
}

/**
 * Split the given set of flags off into a new economy.
 */
void Economy::split(const std::set<OPtr<Flag>>& flags) {
	assert(!flags.empty());

	Economy* e = owner_.create_economy();

	for (const DescriptionIndex& ware_index : owner_.tribe().wares()) {
		e->ware_target_quantities_[ware_index] = ware_target_quantities_[ware_index];
	}

	for (const DescriptionIndex& worker_index : owner_.tribe().workers()) {
		e->worker_target_quantities_[worker_index] = worker_target_quantities_[worker_index];
	}

	for (const OPtr<Flag>& temp_flag : flags) {
		Flag& flag = *temp_flag.get(owner().egbase());
		assert(flags_.size() > 1);  // We will not be deleted in remove_flag, right?
		remove_flag(flag);
		e->add_flag(flag);
	}

	// As long as rebalance commands are tied to specific flags, we
	// need this, because the flag that rebalance commands for us were
	// tied to might have been moved into the other economy
	start_request_timer();
}

/**
 * Make sure the request timer is running.
 */
void Economy::start_request_timer(int32_t const delta) {
	if (upcast(Game, game, &owner_.egbase()))
		game->cmdqueue().enqueue(
		   new CmdCallEconomyBalance(game->get_gametime() + delta, this, request_timerid_));
}

/**
 * Find the supply that is best suited to fulfill the given request.
 * \return 0 if no supply is found, the best supply otherwise
 */
Supply* Economy::find_best_supply(Game& game, const Request& req, int32_t& cost) {
	assert(req.is_open());

	Route buf_route0, buf_route1;
	Supply* best_supply = nullptr;
	Route* best_route = nullptr;
	int32_t best_cost = -1;
	Flag& target_flag = req.target_flag();

	available_supplies_.clear();

	for (size_t i = 0; i < supplies_.get_nrsupplies(); ++i) {
		Supply& supp = supplies_[i];

		// Just skip if supply does not provide required ware
		if (!supp.nr_supplies(game, req))
			continue;

		const SupplyProviders provider = supp.provider_type(&game);

		// We generally ignore disponible wares on ship as it is not possible to reliably
		// calculate route (transportation time)
		if (provider == SupplyProviders::kShip) {
			continue;
		}

		const Widelands::Coords provider_position =
		   supp.get_position(game)->base_flag().get_position();

		const uint32_t dist = game.map().calc_distance(target_flag.get_position(), provider_position);

		UniqueDistance ud = {dist, supp.get_position(game)->serial(), provider};

		// std::map quarantees uniqueness, practically it means that if more wares are on the same
		// flag, only
		// first one will be inserted into available_supplies
		available_supplies_.insert(std::make_pair(ud, &supplies_[i]));
	}

	// Now available supplies have been sorted by distance to requestor
	for (auto& supplypair : available_supplies_) {
		Supply& supp = *supplypair.second;

		Route* const route = best_route != &buf_route0 ? &buf_route0 : &buf_route1;
		// will be cleared by find_route()

		if (!find_route(
		       supp.get_position(game)->base_flag(), target_flag, route, req.get_type(), best_cost)) {
			if (!best_route) {
				log("Economy::find_best_supply: Error, COULD NOT FIND A ROUTE!");
				// To help to debug this a bit:
				log(" ... ware at: %3dx%3d, requestor at: %3dx%3d!",
				    supp.get_position(game)->base_flag().get_position().x,
				    supp.get_position(game)->base_flag().get_position().y, target_flag.get_position().x,
				    target_flag.get_position().y);
			}
			continue;
		}
		best_supply = &supp;
		best_route = route;
		best_cost = route->get_totalcost();
	}

	if (!best_route)
		return nullptr;

	cost = best_cost;
	return best_supply;
}

struct RequestSupplyPair {
	TrackPtr<Request> request;
	TrackPtr<Supply> supply;
	int32_t priority;

	/**
	 * pairid is an explicit tie-breaker for comparison.
	 *
	 * Without it, the pair priority queue would use an implicit, system
	 * dependent tie breaker, which in turn causes desyncs.
	 */
	uint32_t pairid;

	struct Compare {
		bool operator()(const RequestSupplyPair& p1, const RequestSupplyPair& p2) {
			return p1.priority == p2.priority ? p1.pairid < p2.pairid : p1.priority < p2.priority;
		}
	};
};

using RSPairQueue = std::
   priority_queue<RequestSupplyPair, std::vector<RequestSupplyPair>, RequestSupplyPair::Compare>;

struct RSPairStruct {
	RSPairQueue queue;
	uint32_t pairid;
	int32_t nexttimer;

	RSPairStruct() : pairid(0), nexttimer(0) {
	}
};

/**
 * Walk all Requests and find potential transfer candidates.
 */
void Economy::process_requests(Game& game, RSPairStruct* supply_pairs) {
	// Algorithm can decide that wares are not to be delivered to constructionsite
	// right now, therefore we need to shcedule next pairing
	bool postponed_pairing_needed = false;
	for (Request* temp_req : requests_) {
		Request& req = *temp_req;

		// We somehow get desynced request lists that don't trigger desync
		// alerts, so add info to the sync stream here.
		{
			::StreamWrite& ss = game.syncstream();
			ss.unsigned_8(SyncEntry::kProcessRequests);
			ss.unsigned_8(req.get_type());
			ss.unsigned_8(req.get_index());
			ss.unsigned_32(req.target().serial());
		}

		int32_t cost;  // estimated time in milliseconds to fulfill Request
		Supply* const supp = find_best_supply(game, req, cost);

		if (!supp)
			continue;

		if (!supp->is_active()) {
			// Calculate the time the building will be forced to idle waiting
			// for the request
			int32_t const idletime = game.get_gametime() + 15000 + 2 * cost - req.get_required_time();
			// If the building wouldn't have to idle, we wait with the request
			if (idletime < -200) {
				if (supply_pairs->nexttimer < 0 || supply_pairs->nexttimer > -idletime)
					supply_pairs->nexttimer = -idletime;

				continue;
			}
		}

		int32_t const priority = req.get_priority(cost);
		if (priority < 0) {
			// We dont "pair" the req with supply now, and dont set s.nexttimer right now
			// but should not forget about this productionsite waiting for the building material
			postponed_pairing_needed = true;
			continue;
		}

		// Otherwise, consider this request/supply pair for queueing
		RequestSupplyPair rsp;
		rsp.request = &req;
		rsp.supply = supp;
		rsp.priority = priority;
		rsp.pairid = ++supply_pairs->pairid;

		supply_pairs->queue.push(rsp);
	}
	if (postponed_pairing_needed && supply_pairs->nexttimer < 0) {
		// so no other pair set the timer, so we set them now for after 30 seconds
		supply_pairs->nexttimer = 30 * 1000;
	}
}

/**
 * Try to fulfill open requests with available supplies.
 */
void Economy::balance_requestsupply(Game& game) {
	RSPairStruct rsps;
	rsps.nexttimer = -1;

	//  Try to fulfill Requests.
	process_requests(game, &rsps);

	//  Now execute request/supply pairs.
	while (!rsps.queue.empty()) {
		RequestSupplyPair rsp = rsps.queue.top();

		rsps.queue.pop();

		if (!rsp.request || !rsp.supply || !has_request(*rsp.request) ||
		    !rsp.supply->nr_supplies(game, *rsp.request)) {
			rsps.nexttimer = 200;
			continue;
		}

		rsp.request->start_transfer(game, *rsp.supply);
		rsp.request->set_last_request_time(game.get_gametime());

		//  for multiple wares
		if (rsp.request && has_request(*rsp.request))
			rsps.nexttimer = 200;
	}

	if (rsps.nexttimer > 0) {  //  restart the timer, if necessary
		start_request_timer(rsps.nexttimer);
	}
}

std::unique_ptr<Soldier> Economy::soldier_prototype_ =
   nullptr;  // minimal invasive fix of bug 1236538

/**
 * Check whether there is a supply for the given request. If the request is a
 * worker request without supply, attempt to create a new worker in a warehouse.
 */
void Economy::create_requested_worker(Game& game, DescriptionIndex index) {
	uint32_t demand = 0;

	bool soldier_level_check;
	const TribeDescr& tribe = owner().tribe();
	const WorkerDescr& w_desc = *tribe.get_worker_descr(index);
	Request* open_request = nullptr;

	// Make a dummy soldier, which should never be assigned to any economy
	// Minimal invasive fix of bug 1236538: never create a rookie for a request
	// that required a hero.
	if (upcast(const SoldierDescr, s_desc, &w_desc)) {
		if (!soldier_prototype_) {
			Soldier* test_rookie = static_cast<Soldier*>(&(s_desc->create_object()));
			soldier_prototype_.reset(test_rookie);
		}
		soldier_level_check = true;
	} else {
		soldier_level_check = false;
	}

	for (Request* temp_req : requests_) {
		const Request& req = *temp_req;

		if (req.get_type() != wwWORKER || req.get_index() != index)
			continue;

		// need to check for each request separately, because esp. soldier
		// requests have different specific requirements
		if (supplies_.have_supplies(game, req))
			continue;

		// Requests for heroes should not trigger the creation of more rookies
		if (soldier_level_check) {
			if (!(req.get_requirements().check(*soldier_prototype_)))
				continue;
		}

		uint32_t current_demand = req.get_open_count();
		demand += current_demand;
		if (current_demand > 0) {
			open_request = temp_req;
		}
	}

	if (!demand)
		return;

	// We have worker demand that is not fulfilled by supplies
	// Find warehouses where we can create the required workers,
	// and collect stats about existing build prerequisites
	const WorkerDescr::Buildcost& cost = w_desc.buildcost();
	std::vector<Quantity> total_available;
	Quantity total_planned = 0;

	total_available.insert(total_available.begin(), cost.size(), 0);

	for (uint32_t n_wh = 0; n_wh < warehouses().size(); ++n_wh) {
		Warehouse* wh = warehouses_[n_wh];

		uint32_t planned = wh->get_planned_workers(game, index);
		total_planned += planned;

		while (wh->can_create_worker(game, index)) {
			wh->create_worker(game, index);
			if (!--demand)
				return;
		}

		std::vector<Quantity> wh_available = wh->calc_available_for_worker(game, index);
		assert(wh_available.size() == total_available.size());

		for (Quantity idx = 0; idx < total_available.size(); ++idx)
			total_available[idx] += wh_available[idx];
	}

	// Couldn't create enough workers now.
	// Let's see how many we have resources for that may be scattered
	// throughout the economy.
	uint32_t can_create = std::numeric_limits<uint32_t>::max();
	uint32_t idx = 0;
	uint32_t scarcest_idx = 0;
	bool plan_at_least_one = false;
	for (const auto& bc : cost) {
		uint32_t cc = total_available[idx] / bc.second;
		if (cc <= can_create) {
			scarcest_idx = idx;
			can_create = cc;
		}

		// if the target quantity of a resource is set to 0
		// plan at least one worker, so a request for that resource is triggered
		DescriptionIndex id_w = tribe.ware_index(bc.first);
		if (id_w != INVALID_INDEX && 0 == ware_target_quantity(id_w).permanent) {
			plan_at_least_one = true;
		}
		idx++;
	}

	if (total_planned > can_create && (!plan_at_least_one || total_planned > 1)) {
		// Eliminate some excessive plans, to make sure we never request more than
		// there are supplies for (otherwise, cyclic transportation might happen)
		// except in case of planAtLeastOne we continue to plan at least one
		// Note that supplies might suddenly disappear outside our control because
		// of loss of land or silly player actions.
		Warehouse* wh_with_plan = nullptr;
		for (uint32_t n_wh = 0; n_wh < warehouses().size(); ++n_wh) {
			Warehouse* wh = warehouses_[n_wh];

			uint32_t planned = wh->get_planned_workers(game, index);
			uint32_t reduce = std::min(planned, total_planned - can_create);

			if (plan_at_least_one && planned > 0) {
				wh_with_plan = wh;
			}
			wh->plan_workers(game, index, planned - reduce);
			total_planned -= reduce;
		}

		// in case of planAtLeastOne undo a set to zero
		if (nullptr != wh_with_plan && 0 == total_planned)
			wh_with_plan->plan_workers(game, index, 1);

	} else if (total_planned < demand) {
		uint32_t plan_goal = std::min(can_create, demand);

		for (uint32_t n_wh = 0; n_wh < warehouses().size(); ++n_wh) {
			Warehouse* wh = warehouses_[n_wh];
			uint32_t supply = wh->calc_available_for_worker(game, index)[scarcest_idx];

			total_planned -= wh->get_planned_workers(game, index);
			uint32_t plan = std::min(supply, plan_goal - total_planned);
			wh->plan_workers(game, index, plan);
			total_planned += plan;
		}

		// plan at least one if required and if we haven't done already
		// we are going to ignore stock policies of all warehouses here completely
		// the worker we are making is not going to be stocked, there is a request for him
		if (plan_at_least_one && 0 == total_planned) {
			Warehouse* wh = find_closest_warehouse(open_request->target_flag());
			if (nullptr == wh)
				wh = warehouses_[0];
			wh->plan_workers(game, index, 1);
		}
	}
}

/**
 * Walk all Requests and find requests of workers than aren't supplied. Then
 * try to create the worker at warehouses.
 */
void Economy::create_requested_workers(Game& game) {
	if (!warehouses().size())
		return;

	for (const DescriptionIndex& worker_index : owner().tribe().workers()) {
		if (owner().is_worker_type_allowed(worker_index) &&
		    owner().tribe().get_worker_descr(worker_index)->is_buildable()) {
			create_requested_worker(game, worker_index);
		}
	}
}

/**
 * Helper function for \ref handle_active_supplies
 */
static bool accept_warehouse_if_policy(Warehouse& wh,
                                       WareWorker type,
                                       DescriptionIndex ware,
                                       StockPolicy policy) {
	return wh.get_stock_policy(type, ware) == policy;
}

/**
 * Send all active supplies (wares that are outside on the road network without
 * being sent to a specific request) to a warehouse.
 */
void Economy::handle_active_supplies(Game& game) {
	if (!warehouses().size())
		return;

	using Assignments = std::vector<std::pair<Supply*, Warehouse*>>;
	Assignments assignments;

	for (uint32_t idx = 0; idx < supplies_.get_nrsupplies(); ++idx) {
		Supply& supply = supplies_[idx];
		if (supply.has_storage())
			continue;

		WareWorker type;
		DescriptionIndex ware;
		supply.get_ware_type(type, ware);

		bool haveprefer = false;
		bool havenormal = false;

		// One of preferred warehouses (if any) with lowest stock of ware/worker
		Warehouse* preferred_wh = nullptr;
		// Stock of particular ware in preferred warehouse
		uint32_t preferred_wh_stock = std::numeric_limits<uint32_t>::max();

		for (uint32_t nwh = 0; nwh < warehouses_.size(); ++nwh) {
			Warehouse* wh = warehouses_[nwh];
			StockPolicy policy = wh->get_stock_policy(type, ware);
			if (policy == StockPolicy::kPrefer) {
				haveprefer = true;

				// Getting count of worker/ware
				uint32_t current_stock;
				if (type == WareWorker::wwWARE) {
					current_stock = wh->get_wares().stock(ware);
				} else {
					current_stock = wh->get_workers().stock(ware);
				}
				// Stocks lower then in previous one?
				if (current_stock < preferred_wh_stock) {
					preferred_wh = wh;
					preferred_wh_stock = current_stock;
				}
			}
			if (policy == StockPolicy::kNormal)
				havenormal = true;
		}
		if (!havenormal && !haveprefer && type == wwWARE)
			continue;

		// We either have one preferred warehouse picked up or walk on roads to find nearest one
		Warehouse* wh = nullptr;
		if (preferred_wh) {
			wh = preferred_wh;
		} else {
			wh = find_closest_warehouse(supply.get_position(game)->base_flag(), type, nullptr, 0,
			                            (!havenormal) ? WarehouseAcceptFn() :
			                                            boost::bind(&accept_warehouse_if_policy, _1,
			                                                        type, ware, StockPolicy::kNormal));
		}
		if (!wh) {
			log("Warning: Economy::handle_active_supplies "
			    "didn't find warehouse\n");
			return;
		}

		assignments.push_back(std::make_pair(&supply, wh));
	}

	// Actually start with the transfers in a separate second phase,
	// to avoid potential future problems caused by the supplies_ changing
	// under us in some way.
	::StreamWrite& ss = game.syncstream();
	ss.unsigned_8(SyncEntry::kHandleActiveSupplies);
	ss.unsigned_32(assignments.size());

	for (const auto& temp_assignment : assignments) {
		ss.unsigned_32(temp_assignment.first->get_position(game)->serial());
		ss.unsigned_32(temp_assignment.second->serial());

		temp_assignment.first->send_to_storage(game, temp_assignment.second);
	}
}

/**
 * Balance Requests and Supplies by collecting and weighing pairs, and
 * starting transfers for them.
 */
void Economy::balance(uint32_t const timerid) {
	if (request_timerid_ != timerid) {
		return;
	}
	++request_timerid_;

	Game& game = dynamic_cast<Game&>(owner().egbase());

	check_splits();

	create_requested_workers(game);

	balance_requestsupply(game);

	handle_active_supplies(game);
}
}  // namespace Widelands
