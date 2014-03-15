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

#include "economy/economy.h"

#include <boost/bind.hpp>

#include "economy/cmd_call_economy_balance.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "economy/route.h"
#include "economy/routeastar.h"
#include "economy/router.h"
#include "economy/warehousesupply.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

Economy::Economy(Player & player) :
	m_owner(player),
	m_request_timerid(0)
{
	const Tribe_Descr & tribe = player.tribe();
	Ware_Index const nr_wares   = tribe.get_nrwares();
	Ware_Index const nr_workers = tribe.get_nrworkers();
	m_wares.set_nrwares(nr_wares);
	m_workers.set_nrwares(nr_workers);

	player.add_economy(*this);

	m_ware_target_quantities   = new Target_Quantity[nr_wares  .value()];
	for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
		Target_Quantity tq;
		tq.permanent =
			tribe.get_ware_descr(i)->default_target_quantity();
		tq.last_modified = 0;
		m_ware_target_quantities[i.value()] = tq;
	}
	m_worker_target_quantities = new Target_Quantity[nr_workers.value()];
	for (Ware_Index i = Ware_Index::First(); i < nr_workers; ++i) {
		Target_Quantity tq;
		tq.permanent =
			tribe.get_worker_descr(i)->default_target_quantity();
		tq.last_modified = 0;
		m_worker_target_quantities[i.value()] = tq;
	}

	m_router =
		 new Router(boost::bind(&Economy::_reset_all_pathfinding_cycles, this));
}

Economy::~Economy()
{
	m_owner.remove_economy(*this);

	if (m_requests.size())
		log("Warning: Economy still has requests left on destruction\n");
	if (m_flags.size())
		log("Warning: Economy still has flags left on destruction\n");
	if (m_warehouses.size())
		log("Warning: Economy still has warehouses left on destruction\n");

	delete[] m_ware_target_quantities;
	delete[] m_worker_target_quantities;

	delete m_router;
}


/**
 * \return an arbitrary flag in this economy.
 */
Flag* Economy::get_arbitrary_flag()
{
	if (m_flags.empty())
		return nullptr;

	return m_flags[0];
}

/**
 * Two flags have been connected; check whether their economies should be
 * merged.
 * Since we could merge into both directions, we preserve the economy that is
 * currently bigger (should be more efficient).
*/
void Economy::check_merge(Flag & f1, Flag & f2)
{
	Economy * e1 = f1.get_economy();
	Economy * e2 = f2.get_economy();
	if (e1 != e2) {
		if (e1->get_nrflags() < e2->get_nrflags())
			std::swap(e1, e2);
		e1->_merge(*e2);
	}
}

/**
 * Notify the economy that there may no longer be a connection between
 * the given flags in the road and seafaring network.
 */
void Economy::check_split(Flag & f1, Flag & f2)
{
	assert(&f1 != &f2);
	assert(f1.get_economy() == f2.get_economy());

	Economy * e = f1.get_economy();
	// No economy in the editor.
	if (not e)
		return;

	e->m_split_checks.push_back(std::make_pair(OPtr<Flag>(&f1), OPtr<Flag>(&f2)));
	e->rebalance_supply(); // the real split-checking is done during rebalance
}

void Economy::_check_splits()
{
	Editor_Game_Base & egbase = owner().egbase();
	Map & map = egbase.map();

	while (m_split_checks.size()) {
		Flag * f1 = m_split_checks.back().first.get(egbase);
		Flag * f2 = m_split_checks.back().second.get(egbase);
		m_split_checks.pop_back();

		if (!f1 || !f2) {
			if (!f1 && !f2)
				continue;
			if (!f1)
				f1 = f2;
			if (f1->get_economy() != this)
				continue;

			// Handle the case when two or more roads are removed simultaneously
			RouteAStar<AStarZeroEstimator> astar(*m_router, wwWORKER, AStarZeroEstimator());
			astar.push(*f1);
			std::set<OPtr<Flag> > reachable;
			while (RoutingNode * current = astar.step())
				reachable.insert(&current->base_flag());
			if (reachable.size() != m_flags.size())
				_split(reachable);
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
		RouteAStar<AStarEstimator> astar(*m_router, wwWORKER, AStarEstimator(map, *f2));
		astar.push(*f1);
		std::set<OPtr<Flag> > reachable;

		for (;;) {
			RoutingNode * current = astar.step();
			if (!current) {
				_split(reachable);
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
bool Economy::find_route
	(Flag & start, Flag & end,
	 Route * const route,
	 WareWorker const type,
	 int32_t const cost_cutoff)
{
	assert(start.get_economy() == this);
	assert(end  .get_economy() == this);

	Map & map = owner().egbase().map();

	return
		m_router->find_route(start, end, route, type, cost_cutoff, map);
}

struct ZeroEstimator {
	int32_t operator()(RoutingNode & /* node */) const {return 0;}
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
Warehouse * Economy::find_closest_warehouse
	(Flag & start, WareWorker type, Route * route, uint32_t cost_cutoff,
	 const Economy::WarehouseAcceptFn & acceptfn)
{
	if (!warehouses().size())
		return nullptr;

	// A-star with zero estimator = Dijkstra
	RouteAStar<ZeroEstimator> astar(*m_router, type);
	astar.push(start);

	while (RoutingNode * current = astar.step()) {
		if
			(cost_cutoff && current->mpf_realcost >
			 static_cast<int32_t>(cost_cutoff))
			return nullptr;

		Flag & flag = current->base_flag();
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
void Economy::add_flag(Flag & flag)
{
	assert(flag.get_economy() == nullptr);

	m_flags.push_back(&flag);
	flag.set_economy(this);

	flag.reset_path_finding_cycle();
}

/**
 * Remove a flag from the flag array.
 * Only call from Flag cleanup and split/merger code!
*/
void Economy::remove_flag(Flag & flag)
{
	assert(flag.get_economy() == this);

	_remove_flag(flag);

	// automatically delete the economy when it becomes empty.
	if (m_flags.empty())
		delete this;
}

/**
 * Remove the flag, but don't delete the economy automatically.
 * This is called from the merge code.
*/
void Economy::_remove_flag(Flag & flag)
{
	flag.set_economy(nullptr);

	// fast remove
	container_iterate(Flags, m_flags, i)
		if (*i.current == &flag) {
			*i.current = *(i.get_end() - 1);
			return m_flags.pop_back();
		}
	throw wexception("trying to remove nonexistent flag");
}

/**
 * Callback for the incredibly rare case that the \ref Router pathfinding
 * cycle wraps around.
 */
void Economy::_reset_all_pathfinding_cycles()
{
	container_iterate(Flags, m_flags, i)
		(*i.current)->reset_path_finding_cycle();
}

/**
 * Set the target quantities for the given Ware_Index to the
 * numbers given in permanent. Also update the last
 * modification time.
 *
 * This is called from Cmd_ResetTargetQuantity and Cmd_SetTargetQuantity
 */
void Economy::set_ware_target_quantity
	(Ware_Index const ware_type,
	 uint32_t   const permanent,
	 Time       const mod_time)
{
	Target_Quantity & tq = m_ware_target_quantities[ware_type.value()];
	tq.permanent = permanent;
	tq.last_modified = mod_time;
}


void Economy::set_worker_target_quantity
	(Ware_Index const ware_type,
	 uint32_t   const permanent,
	 Time       const mod_time)
{
	Target_Quantity & tq = m_worker_target_quantities[ware_type.value()];
	tq.permanent = permanent;
	tq.last_modified = mod_time;
}


/**
 * Call this whenever some entity created a ware, e.g. when a lumberjack
 * has felled a tree.
 * This is also called when a ware is added to the economy through trade or
 * a merger.
*/
void Economy::add_wares(Ware_Index const id, uint32_t const count)
{
	//log("%p: add(%i, %i)\n", this, id, count);

	m_wares.add(id, count);
	_start_request_timer();

	// TODO: add to global player inventory?
}
void Economy::add_workers(Ware_Index const id, uint32_t const count)
{
	//log("%p: add(%i, %i)\n", this, id, count);

	m_workers.add(id, count);
	_start_request_timer();

	// TODO: add to global player inventory?
}

/**
 * Call this whenever a ware is destroyed or consumed, e.g. food has been
 * eaten or a warehouse has been destroyed.
 * This is also called when a ware is removed from the economy through trade or
 * a split of the Economy.
*/
void Economy::remove_wares(Ware_Index const id, uint32_t const count)
{
	assert(id < m_owner.tribe().get_nrwares());
	//log("%p: remove(%i, %i) from %i\n", this, id, count, m_wares.stock(id));

	m_wares.remove(id, count);

	// TODO: remove from global player inventory?
}

/**
 * Call this whenever a worker is destroyed.
 * This is also called when a worker is removed from the economy through
 * a split of the Economy.
 */
void Economy::remove_workers(Ware_Index const id, uint32_t const count)
{
	//log("%p: remove(%i, %i) from %i\n", this, id, count, m_workers.stock(id));

	m_workers.remove(id, count);

	// TODO: remove from global player inventory?
}

/**
 * Add the warehouse to our list of warehouses.
 * This also adds the wares in the warehouse to the economy. However, if wares
 * are added to the warehouse in the future, add_wares() must be called.
*/
void Economy::add_warehouse(Warehouse & wh)
{
	m_warehouses.push_back(&wh);
}

/**
 * Remove the warehouse and its wares from the economy.
*/
void Economy::remove_warehouse(Warehouse & wh)
{
	for (size_t i = 0; i < m_warehouses.size(); ++i)
		if (m_warehouses[i] == &wh) {
			m_warehouses[i] = *m_warehouses.rbegin();
			m_warehouses.pop_back();
			return;
		}


	//  This assert was modified, since on loading, warehouses might try to
	//  remove themselves from their own economy, though they weren't added
	//  (since they weren't initialized)
	assert(m_warehouses.empty());
}

/**
 * Consider the request, try to fulfill it immediately or queue it for later.
 * Important: This must only be called by the \ref Request class.
*/
void Economy::add_request(Request & req)
{
	assert(req.is_open());
	assert(!_has_request(req));

	assert(&owner());

	m_requests.push_back(&req);

	// Try to fulfill the request
	_start_request_timer();
}

/**
 * \return true if the given Request is registered with the \ref Economy, false
 * otherwise
*/
bool Economy::_has_request(Request & req)
{
	return
		std::find(m_requests.begin(), m_requests.end(), &req)
		!=
		m_requests.end();
}

/**
 * Remove the request from this economy.
 * Important: This must only be called by the \ref Request class.
*/
void Economy::remove_request(Request & req)
{
	RequestList::iterator const it =
		std::find(m_requests.begin(), m_requests.end(), &req);

	if (it == m_requests.end()) {
		log("WARNING: remove_request(%p) not in list\n", &req);
		return;
	}

	*it = *m_requests.rbegin();

	m_requests.pop_back();
}

/**
 * Add a supply to our list of supplies.
*/
void Economy::add_supply(Supply & supply)
{
	m_supplies.add_supply(supply);
	_start_request_timer();
}


/**
 * Remove a supply from our list of supplies.
*/
void Economy::remove_supply(Supply & supply)
{
	m_supplies.remove_supply(supply);
}


bool Economy::needs_ware(Ware_Index const ware_type) const {
	uint32_t const t = ware_target_quantity(ware_type).permanent;
	uint32_t quantity = 0;
	container_iterate_const(std::vector<Warehouse *>, m_warehouses, wh) {
		quantity += (*wh)->get_wares().stock(ware_type);
		if (t <= quantity)
			return false;
	}
	return true;
}


bool Economy::needs_worker(Ware_Index const worker_type) const {
	uint32_t const t = worker_target_quantity(worker_type).permanent;
	uint32_t quantity = 0;
	container_iterate_const(std::vector<Warehouse *>, m_warehouses, wh) {
		quantity += (*wh)->get_workers().stock(worker_type);
		if (t <= quantity)
			return false;
	}
	return true;
}


/**
 * Add e's flags to this economy.
 *
 * Also transfer all wares and wares request. Try to resolve the new ware
 * requests if possible.
*/
void Economy::_merge(Economy & e)
{
	for (Ware_Index::value_t i = m_owner.tribe().get_nrwares().value(); i;) {
		--i;
		Target_Quantity other_tq = e.m_ware_target_quantities[i];
		Target_Quantity & this_tq = m_ware_target_quantities[i];
		if (this_tq.last_modified < other_tq.last_modified)
			this_tq = other_tq;
	}
	for (Ware_Index::value_t i = m_owner.tribe().get_nrworkers().value(); i;) {
		--i;
		Target_Quantity other_tq = e.m_worker_target_quantities[i];
		Target_Quantity & this_tq = m_worker_target_quantities[i];
		if (this_tq.last_modified < other_tq.last_modified)
			this_tq = other_tq;
	}

	//  If the options window for e is open, but not the one for *this, the user
	//  should still have an options window after the merge. Create an options
	//  window for *this where the options window for e is, to give the user
	//  some continuity.
	if
		(e.m_optionswindow_registry.window and
		 not m_optionswindow_registry.window)
	{
		m_optionswindow_registry.x = e.m_optionswindow_registry.x;
		m_optionswindow_registry.y = e.m_optionswindow_registry.y;
		show_options_window();
	}

	for (std::vector<Flag *>::size_type i = e.get_nrflags() + 1; --i;) {
		assert(i == e.get_nrflags());

		Flag & flag = *e.m_flags[0];

		e._remove_flag(flag); // do not delete other economy yet!
		add_flag(flag);
	}

	// Remember that the other economy may not have been connected before the merge
	m_split_checks.insert(m_split_checks.end(), e.m_split_checks.begin(), e.m_split_checks.end());

	// implicitly delete the economy
	delete &e;
}

/**
 * Split the given set of flags off into a new economy.
 */
void Economy::_split(const std::set<OPtr<Flag> > & flags)
{
	assert(!flags.empty());

	Economy & e = *new Economy(m_owner);

	for (Ware_Index::value_t i = m_owner.tribe().get_nrwares  ().value(); i;) {
		--i;
		e.m_ware_target_quantities[i] = m_ware_target_quantities[i];
	}
	for (Ware_Index::value_t i = m_owner.tribe().get_nrworkers().value(); i;) {
		--i;
		e.m_worker_target_quantities[i] = m_worker_target_quantities[i];
	}

	container_iterate_const(std::set<OPtr<Flag> >, flags, it) {
		Flag & flag = *it.current->get(owner().egbase());
		assert(m_flags.size() > 1);  // We will not be deleted in remove_flag, right?
		remove_flag(flag);
		e.add_flag(flag);
	}

	// As long as rebalance commands are tied to specific flags, we
	// need this, because the flag that rebalance commands for us were
	// tied to might have been moved into the other economy
	_start_request_timer();
}

/**
 * Make sure the request timer is running.
 */
void Economy::_start_request_timer(int32_t const delta)
{
	if (upcast(Game, game, &m_owner.egbase()))
		game->cmdqueue().enqueue
			(new Cmd_Call_Economy_Balance
			 	(game->get_gametime() + delta, this, m_request_timerid));
}


/**
 * Find the supply that is best suited to fulfill the given request.
 * \return 0 if no supply is found, the best supply otherwise
*/
Supply * Economy::_find_best_supply
	(Game & game, const Request & req, int32_t & cost)
{
	assert(req.is_open());

	Route buf_route0, buf_route1;
	Supply * best_supply = nullptr;
	Route  * best_route  = nullptr;
	int32_t  best_cost   = -1;
	Flag & target_flag = req.target_flag();

	for (size_t i = 0; i < m_supplies.get_nrsupplies(); ++i) {
		Supply & supp = m_supplies[i];

		// Check requirements
		if (!supp.nr_supplies(game, req))
			continue;

		Route * const route =
			best_route != &buf_route0 ? &buf_route0 : &buf_route1;
		// will be cleared by find_route()

		if
			(!
			 find_route
			 	(supp.get_position(game)->base_flag(),
			 	 target_flag,
			 	 route,
			 	 req.get_type(),
			 	 best_cost))
		{
			if (!best_route)
				throw wexception
					("Economy::find_best_supply: COULD NOT FIND A ROUTE!");
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
	TrackPtr<Supply>  supply;
	int32_t priority;

	/**
	 * pairid is an explicit tie-breaker for comparison.
	 *
	 * Without it, the pair priority queue would use an implicit, system
	 * dependent tie breaker, which in turn causes desyncs.
	 */
	uint32_t pairid;

	struct Compare {
		bool operator()
			(const RequestSupplyPair & p1, const RequestSupplyPair & p2)
		{
			return
				p1.priority == p2.priority ? p1.pairid < p2.pairid :
				p1.priority <  p2.priority;
		}
	};
};

typedef
	std::priority_queue
	<RequestSupplyPair,
	std::vector<RequestSupplyPair>,
	RequestSupplyPair::Compare>
	RSPairQueue;

struct RSPairStruct {
	RSPairQueue queue;
	uint32_t pairid;
	int32_t nexttimer;

	RSPairStruct() : pairid(0), nexttimer(0) {}
};

/**
 * Walk all Requests and find potential transfer candidates.
*/
void Economy::_process_requests(Game & game, RSPairStruct & s)
{
	container_iterate_const(RequestList, m_requests, i) {
		Request & req = **i.current;

		// We somehow get desynced request lists that don't trigger desync
		// alerts, so add info to the sync stream here.
		{
			::StreamWrite & ss = game.syncstream();
			ss.Unsigned8 (req.get_type  ());
			ss.Unsigned8 (req.get_index ().value());
			ss.Unsigned32(req.target    ().serial());
		}

		int32_t cost; // estimated time in milliseconds to fulfill Request
		Supply * const supp = _find_best_supply(game, req, cost);

		if (!supp)
			continue;

		if (!supp->is_active()) {
			// Calculate the time the building will be forced to idle waiting
			// for the request
			int32_t const idletime =
				game.get_gametime() + 15000 + 2 * cost - req.get_required_time();
			// If the building wouldn't have to idle, we wait with the request
			if (idletime < -200) {
				if (s.nexttimer < 0 || s.nexttimer > -idletime)
					s.nexttimer = -idletime;

				continue;
			}
		}

		int32_t const priority = req.get_priority(cost);
		if (priority < 0)
			continue;

		// Otherwise, consider this request/supply pair for queueing
		RequestSupplyPair rsp;
		rsp.request  = &req;
		rsp.supply = supp;
		rsp.priority = priority;
		rsp.pairid = ++s.pairid;

		s.queue.push(rsp);
	}
}

/**
 * Try to fulfill open requests with available supplies.
 */
void Economy::_balance_requestsupply(Game & game)
{
	RSPairStruct rsps;
	rsps.nexttimer = -1;

	//  Try to fulfill Requests.
	_process_requests(game, rsps);

	//  Now execute request/supply pairs.
	while (!rsps.queue.empty()) {
		RequestSupplyPair rsp = rsps.queue.top();

		rsps.queue.pop();

		if
			(!rsp.request                ||
			 !rsp.supply                 ||
			 !_has_request(*rsp.request) ||
			 !rsp.supply->nr_supplies(game, *rsp.request))
		{
			rsps.nexttimer = 200;
			continue;
		}

		rsp.request->start_transfer(game, *rsp.supply);
		rsp.request->set_last_request_time(game.get_gametime());

		//  for multiple wares
		if (rsp.request && _has_request(*rsp.request))
			rsps.nexttimer = 200;
	}

	if (rsps.nexttimer > 0) //  restart the timer, if necessary
		_start_request_timer(rsps.nexttimer);
}


std::unique_ptr<Soldier> Economy::m_soldier_prototype = nullptr; // minimal invasive fix of bug 1236538

/**
 * Check whether there is a supply for the given request. If the request is a
 * worker request without supply, attempt to create a new worker in a warehouse.
 */
void Economy::_create_requested_worker(Game & game, Ware_Index index)
{
	unsigned demand = 0;

	bool soldier_level_check;
	const Tribe_Descr & tribe = owner().tribe();
	const Worker_Descr & w_desc = *tribe.get_worker_descr(index);

	// Make a dummy soldier, which should never be assigned to any economy
	// Minimal invasive fix of bug 1236538: never create a rookie for a request
	// that required a hero.
	if (upcast(const Soldier_Descr, s_desc, &w_desc)) {
		if (!m_soldier_prototype) {
			Soldier* test_rookie = static_cast<Soldier*> (&(s_desc->create_object()));
			m_soldier_prototype.reset(test_rookie);
		}
		soldier_level_check = true;
	} else {
		soldier_level_check = false;
	}


	container_iterate_const(RequestList, m_requests, j) {
		const Request & req = **j.current;

		if (req.get_type() != wwWORKER || req.get_index() != index)
			continue;

		// need to check for each request separately, because esp. soldier
		// requests have different specific requirements
		if (m_supplies.have_supplies(game, req))
			continue;

		// Requests for heroes should not trigger the creation of more rookies
		if (soldier_level_check)
		{
			if (not (req.get_requirements().check(*m_soldier_prototype)))
				continue;
		}

		demand += req.get_open_count();
	}

	if (!demand)
		return;

	// We have worker demand that is not fulfilled by supplies
	// Find warehouses where we can create the required workers,
	// and collect stats about existing build prerequisites
	const Worker_Descr::Buildcost & cost = w_desc.buildcost();
	std::vector<uint32_t> total_available;
	uint32_t total_planned = 0;

	total_available.insert(total_available.begin(), cost.size(), 0);

	for (uint32_t n_wh = 0; n_wh < warehouses().size(); ++n_wh) {
		Warehouse * wh = m_warehouses[n_wh];

		uint32_t planned = wh->get_planned_workers(game, index);
		total_planned += planned;

		while (wh->can_create_worker(game, index)) {
			wh->create_worker(game, index);
			if (!--demand)
				return;
		}

		std::vector<uint32_t> wh_available =
			wh->calc_available_for_worker(game, index);
		assert(wh_available.size() == total_available.size());

		for (uint32_t idx = 0; idx < total_available.size(); ++idx)
			total_available[idx] += wh_available[idx];
	}

	// Couldn't create enough workers now.
	// Let's see how many we have resources for that may be scattered
	// throughout the economy.
	uint32_t can_create = std::numeric_limits<uint32_t>::max();
	uint32_t idx = 0;
	uint32_t scarcest_idx = 0;
	container_iterate_const(Worker_Descr::Buildcost, cost, bc) {
		uint32_t cc = total_available[idx] / bc.current->second;
		if (cc <= can_create) {
			scarcest_idx = idx;
			can_create = cc;
		}
		idx++;
	}

	if (total_planned > can_create) {
		// Eliminate some excessive plans, to make sure we never request more than
		// there are supplies for (otherwise, cyclic transportation might happen)
		// Note that supplies might suddenly disappear outside our control because
		// of loss of land or silly player actions.
		for (uint32_t n_wh = 0; n_wh < warehouses().size(); ++n_wh) {
			Warehouse * wh = m_warehouses[n_wh];

			uint32_t planned = wh->get_planned_workers(game, index);
			uint32_t reduce = std::min(planned, total_planned - can_create);
			wh->plan_workers(game, index, planned - reduce);
			total_planned -= reduce;
		}
	} else if (total_planned < demand) {
		uint32_t plan_goal = std::min(can_create, demand);

		for (uint32_t n_wh = 0; n_wh < warehouses().size(); ++n_wh) {
			Warehouse * wh = m_warehouses[n_wh];
			uint32_t supply =
				wh->calc_available_for_worker(game, index)[scarcest_idx];

			total_planned -= wh->get_planned_workers(game, index);
			uint32_t plan = std::min(supply, plan_goal - total_planned);
			wh->plan_workers(game, index, plan);
			total_planned += plan;
		}
	}
}

/**
 * Walk all Requests and find requests of workers than aren't supplied. Then
 * try to create the worker at warehouses.
 */
void Economy::_create_requested_workers(Game & game)
{
	if (!warehouses().size())
		return;

	const Tribe_Descr & tribe = owner().tribe();
	for
		(Ware_Index index = Ware_Index::First();
		 index < tribe.get_nrworkers(); ++index)
	{
		if (!owner().is_worker_type_allowed(index))
			continue;
		if (!tribe.get_worker_descr(index)->is_buildable())
			continue;

		_create_requested_worker(game, index);
	}
}

/**
 * Helper function for \ref _handle_active_supplies
 */
static bool accept_warehouse_if_policy
	(Warehouse & wh, WareWorker type,
	 Ware_Index ware, Warehouse::StockPolicy policy)
{
	return wh.get_stock_policy(type, ware) == policy;
}

/**
 * Send all active supplies (wares that are outside on the road network without
 * being sent to a specific request) to a warehouse.
 */
void Economy::_handle_active_supplies(Game & game)
{
	if (!warehouses().size())
		return;

	typedef std::vector<std::pair<Supply *, Warehouse *> > Assignments;
	Assignments assignments;

	for (uint32_t idx = 0; idx < m_supplies.get_nrsupplies(); ++idx) {
		Supply & supply = m_supplies[idx];
		if (supply.has_storage())
			continue;

		WareWorker type;
		Ware_Index ware;
		supply.get_ware_type(type, ware);

		bool haveprefer = false;
		bool havenormal = false;
		for (uint32_t nwh = 0; nwh < m_warehouses.size(); ++nwh) {
			Warehouse * wh = m_warehouses[nwh];
			Warehouse::StockPolicy policy = wh->get_stock_policy(type, ware);
			if (policy == Warehouse::SP_Prefer) {
				haveprefer = true;
				break;
			}
			if (policy == Warehouse::SP_Normal)
				havenormal = true;
		}
		if (!havenormal && !haveprefer && type == wwWARE)
			continue;

		Warehouse * wh = find_closest_warehouse
			(supply.get_position(game)->base_flag(), type, nullptr, 0,
			 (!haveprefer && !havenormal)
			 ?
			 WarehouseAcceptFn()
			 :
			 boost::bind
				(&accept_warehouse_if_policy,
				 _1, type, ware,
				 haveprefer ? Warehouse::SP_Prefer : Warehouse::SP_Normal));

		if (!wh) {
			log
				("Warning: Economy::_handle_active_supplies "
				 "didn't find warehouse\n");
			return;
		}

		assignments.push_back(std::make_pair(&supply, wh));
	}

	// Actually start with the transfers in a separate second phase,
	// to avoid potential future problems caused by the m_supplies changing
	// under us in some way.
	::StreamWrite & ss = game.syncstream();
	ss.Unsigned32(0x02decafa); // appears as facade02 in sync stream
	ss.Unsigned32(assignments.size());

	container_iterate_const(Assignments, assignments, it) {
		ss.Unsigned32(it.current->first->get_position(game)->serial());
		ss.Unsigned32(it.current->second->serial());

		it.current->first->send_to_storage(game, it.current->second);
	}
}

/**
 * Balance Requests and Supplies by collecting and weighing pairs, and
 * starting transfers for them.
*/
void Economy::balance(uint32_t const timerid)
{
	if (m_request_timerid != timerid)
		return;
	++m_request_timerid;

	Game & game = ref_cast<Game, Editor_Game_Base>(owner().egbase());

	_check_splits();

	_create_requested_workers (game);

	_balance_requestsupply(game);

	_handle_active_supplies(game);
}

}
