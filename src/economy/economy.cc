/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "economy.h"

// Package includes
#include "flag.h"
#include "route.h"
#include "cmd_call_economy_balance.h"

#include "game.h"
#include "player.h"
#include "request.h"
#include "tribe.h"
#include "upcast.h"
#include "warehouse.h"
#include "warehousesupply.h"
#include "wexception.h"

namespace Widelands {
Economy::Economy(Player & player) :
	m_owner(player),
m_rebuilding(false),
m_request_timerid(0),
mpf_cycle(0)
{
	Tribe_Descr const & tribe = player.tribe();
	Ware_Index const nr_wares = tribe.get_nrwares();
	m_workers.set_nrwares(tribe.get_nrworkers());
	m_wares.set_nrwares(nr_wares);

	player.add_economy(*this);

	m_target_quantities = new Target_Quantity[nr_wares.value()];
	for (Ware_Index i = Ware_Index::First(); i < nr_wares; ++i) {
		Target_Quantity tq;
		tq.temporary = tq.permanent =
			tribe.get_ware_descr(i)->default_target_quantity();
		tq.last_modified = 0;
		m_target_quantities[i.value()] = tq;
	}

}

Economy::~Economy()
{
	assert(!m_rebuilding);

	m_owner.remove_economy(*this);

	if (m_requests.size())
		log("Warning: Economy still has requests left on destruction\n");
	if (m_flags.size())
		log("Warning: Economy still has flags left on destruction\n");
	if (m_warehouses.size())
		log("Warning: Economy still has warehouses left on destruction\n");

	delete[] m_target_quantities;
}


/**
 * \return an arbitrary flag in this economy, or 0 if no flag exists
 */
Flag & Economy::get_arbitrary_flag()
{
	assert(m_flags.size());
	return *m_flags[0];
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
		e1->do_merge(*e2);
	}
}

/// If the two flags can no longer reach each other (pathfinding!), the economy
/// gets split.
///
/// Should we create the new economy starting at f1 or f2? Ideally, we'd split
/// off in a way that the new economy will be relatively small.
///
/// Unfortunately, there's no easy way to tell in advance which of the two
/// resulting economies will be smaller (the problem is even NP-complete), so
/// we use a heuristic.
/// NOTE There is a way; parallel counting. If for example one has size 100 and
/// NOTE the other has size 1, we start counting (to 1) in the first. Then we
/// NOTE switch to the second and count (to 1) there. Then we switch to the
/// NOTE first and count (to 2) there. Then we switch to the second and have
/// NOTE nothing more to count. We are done and know that the second is not
/// NOTE larger than the first.
/// NOTE
/// NOTE We have not done more than n * (s + 1) counting operations, where n is
/// NOTE the number of parallel entities (2 in this example) and s is the size
/// NOTE of the smallest entity (1 in this example). So instead of risking to
/// NOTE make a bad guess and change 100 entities, we count 4 and change 1.
/// NOTE                                                                --sigra
///
/// Using f2 is just a guess, but if anything f2 is probably best: it will be
/// the end point of a road. Since roads are typically built from the center of
/// a country outwards, and since splits are more likely to happen outwards,
/// the economy at the end point is probably smaller in average. It's all just
/// guesswork though ;)
/// NOTE Many roads are built when a new building has just been placed. For
/// NOTE those cases, the guess is bad because the user typically builds from
/// NOTE the new building's flag to some existing flag (at the headquarter or
/// NOTE somewhere in his larger road network). This is also what the user
/// NOTE interface makes the player do when it enters roadbuilding mode after
/// NOTE placing a flag that is not connected with roads.               --sigra
void Economy::check_split(Flag & f1, Flag & f2)
{
	assert(&f1 != &f2);
	assert(f1.get_economy() == f2.get_economy());

	Economy & e = *f1.get_economy();

	if (not e.find_route(f1, f2, 0, false))
		e.do_split(f2);
}

/**
 * Provides the flexible priority queue to maintain the open list.
 *
 * This is more flexible than a standard priority_queue (fast boost() to
 * adjust cost)
*/
struct FlagQueue {
	FlagQueue() {}
	~FlagQueue() {}

	void flush() {m_data.clear();}

	// Return the best node and readjust the tree
	// Basic idea behind the algorithm:
	//  1. the top slot of the tree is empty
	//  2. if this slot has both children:
	//       fill this slot with one of its children or with slot[_size], whichever
	//       is best;
	//       if we filled with slot[_size], stop
	//       otherwise, repeat the algorithm with the child slot
	//     if it doesn't have any children (l >= _size)
	//       put slot[_size] in its place and stop
	//     if only the left child is there
	//       arrange left child and slot[_size] correctly and stop
	Flag * pop()
	{
		if (m_data.empty())
			return 0;

		Flag * head = m_data[0];

		uint32_t const nsize = m_data.size() - 1;
		uint32_t fix = 0;
		while (fix < nsize) {
			uint32_t l = fix * 2 + 1;
			uint32_t r = fix * 2 + 2;
			if (l >= nsize) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->mpf_heapindex = fix;
				break;
			}
			if (r >= nsize) {
				if (m_data[nsize]->cost() <= m_data[l]->cost()) {
					m_data[fix] = m_data[nsize];
					m_data[fix]->mpf_heapindex = fix;
				} else {
					m_data[fix] = m_data[l];
					m_data[fix]->mpf_heapindex = fix;
					m_data[l] = m_data[nsize];
					m_data[l]->mpf_heapindex = l;
				}
				break;
			}

			if (m_data[nsize]->cost() <= m_data[l]->cost() && m_data[nsize]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[nsize];
				m_data[fix]->mpf_heapindex = fix;
				break;
			}
			if (m_data[l]->cost() <= m_data[r]->cost()) {
				m_data[fix] = m_data[l];
				m_data[fix]->mpf_heapindex = fix;
				fix = l;
			} else {
				m_data[fix] = m_data[r];
				m_data[fix]->mpf_heapindex = fix;
				fix = r;
			}
		}

		m_data.pop_back();

		debug(0, "pop");

		head->mpf_heapindex = -1;

		return head;
	}

	// Add a new node and readjust the tree
	// Basic idea:
	//  1. Put the new node in the last slot
	//  2. If parent slot is worse than self, exchange places and recurse
	// Note that I rearranged this a bit so swap isn't necessary
	void push(Flag *t)
	{
		uint32_t slot = m_data.size();
		m_data.push_back(static_cast<Flag *>(0));

		while (slot > 0) {
			uint32_t parent = (slot - 1) / 2;

			if (m_data[parent]->cost() < t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->mpf_heapindex = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->mpf_heapindex = slot;

		debug(0, "push");
	}

	// Rearrange the tree after a node has become better, i.e. move the
	// node up
	// Pushing algorithm is basically the same as in push()
	void boost(Flag *t)
	{
		uint32_t slot = t->mpf_heapindex;

		assert(slot < m_data.size());
		assert(m_data[slot] == t);

		while (slot > 0) {
			uint32_t parent = (slot - 1) / 2;

			if (m_data[parent]->cost() <= t->cost())
				break;

			m_data[slot] = m_data[parent];
			m_data[slot]->mpf_heapindex = slot;
			slot = parent;
		}
		m_data[slot] = t;
		t->mpf_heapindex = slot;

		debug(0, "boost");
	}

	// Recursively check integrity
	void debug(uint32_t node, char const * const str)
	{
		uint32_t l = node * 2 + 1;
		uint32_t r = node * 2 + 2;
		if (m_data[node]->mpf_heapindex != static_cast<int32_t>(node)) {
			fprintf(stderr, "%s: mpf_heapindex integrity!\n", str);
			abort();
		}
		if (l < m_data.size()) {
			if (m_data[node]->cost() > m_data[l]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				abort();
			}
			debug(l, str);
		}
		if (r < m_data.size()) {
			if (m_data[node]->cost() > m_data[r]->cost()) {
				fprintf(stderr, "%s: Integrity failure\n", str);
				abort();
			}
			debug(r, str);
		}
	}

private:
	std::vector<Flag *> m_data;
};

/**
 * Calcaluate a route between two flags.
 *
 * The calculated route is stored in route if it exists.
 *
 * For two flags from the same economy, this function should always be
 * successful, except when it's called from check_split()
 *
 * \note route will be cleared before storing the result.
 *
 * \param start, end start and endpoint of the route
 * \param route the calculated route
 * \param wait UNDOCUMENTED
 * \param cost_cutoff maximum cost for desirable routes. If no route cheaper
 * than this can be found, return false
 *
 * \return true if a route has been found, false otherwise
 *
 * \todo Document parameter wait
*/
bool Economy::find_route
	(Flag & start, Flag & end,
	 Route * const route,
	 bool    const wait,
	 int32_t const cost_cutoff)
{
	assert(start.get_economy() == this);
	assert(end  .get_economy() == this);

	Map & map = owner().egbase().map();

	// advance the path-finding cycle
	++mpf_cycle;
	if (!mpf_cycle) { // reset all cycle fields
		for (uint32_t i = 0; i < m_flags.size(); ++i)
			m_flags[i]->mpf_cycle = 0;
		++mpf_cycle;
	}

	// Add the starting flag into the open list
	FlagQueue Open;
	Flag *current;

	start.mpf_cycle    = mpf_cycle;
	start.mpf_backlink = 0;
	start.mpf_realcost = 0;
	start.mpf_estimate =
		map.calc_cost_estimate(start.get_position(), end.get_position());

	Open.push(&start);

	while ((current = Open.pop())) {
		if (current == &end)
			break; // found our goal

		if (cost_cutoff >= 0 && current->mpf_realcost > cost_cutoff)
			return false;

		// Loop through all neighbouring flags
		Neighbour_list neighbours;

		current->get_neighbours(&neighbours);

		for (uint32_t i = 0; i < neighbours.size(); ++i) {
			Flag * const neighbour = neighbours[i].flag;
			int32_t cost;
			int32_t wait_cost = 0;

			//  No need to find the optimal path when only checking connectivity.
			if (neighbour == &end && !route)
				return true;

			if (wait)
				wait_cost =
					(current->m_item_filled + neighbour->m_item_filled)
					*
					neighbours[i].cost
					/
					2;
			cost = current->mpf_realcost + neighbours[i].cost + wait_cost;

			if (neighbour->mpf_cycle != mpf_cycle) {
				// add to open list
				neighbour->mpf_cycle = mpf_cycle;
				neighbour->mpf_realcost = cost;
				neighbour->mpf_estimate = map.calc_cost_estimate
					(neighbour->get_position(), end.get_position());
				neighbour->mpf_backlink = current;
				Open.push(neighbour);
			} else if (cost + neighbour->mpf_estimate < neighbour->cost()) {
				// found a better path to a field that's already Open
				neighbour->mpf_realcost = cost;
				neighbour->mpf_backlink = current;
				if (neighbour->mpf_heapindex != -1) // This neighbour is already 'popped', skip it
					Open.boost(neighbour);
			}
		}
	}

	if (!current) // path not found
		return false;

	// Unwind the path to form the route
	if (route) {
		route->clear();
		route->m_totalcost = end.mpf_realcost;

		for (Flag * flag = &end;; flag = flag->mpf_backlink) {
			route->m_route.insert(route->m_route.begin(), flag);
			if (flag == &start)
				break;
		}
	}

	return true;
}


/**
 * Add a flag to the flag array.
 * Only call from Flag init and split/merger code!
*/
void Economy::add_flag(Flag & flag)
{
	assert(flag.get_economy() == 0);

	m_flags.push_back(&flag);
	flag.set_economy(this);
	flag.mpf_cycle = 0;
}

/**
 * Remove a flag from the flag array.
 * Only call from Flag cleanup and split/merger code!
*/
void Economy::remove_flag(Flag & flag)
{
	assert(flag.get_economy() == this);

	do_remove_flag(flag);

	// automatically delete the economy when it becomes empty.
	if (m_flags.empty())
		delete this;
}

/**
 * Remove the flag, but don't delete the economy automatically.
 * This is called from the merge code.
*/
void Economy::do_remove_flag(Flag & flag)
{
	flag.set_economy(0);

	// fast remove
	container_iterate(Flags, m_flags, i)
		if (*i.current == &flag) {
			*i.current = *(i.end - 1);
			return m_flags.pop_back();
		}
	throw wexception("trying to remove nonexistent flag");
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

	// TODO: add to global player inventory?
}
void Economy::add_workers(Ware_Index const id, uint32_t const count)
{
	//log("%p: add(%i, %i)\n", this, id, count);

	m_workers.add(id, count);

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

	Target_Quantity & tq = m_target_quantities[id.value()];
	tq.temporary =
		tq.temporary <= tq.permanent + count ?
		tq.permanent : tq.temporary - count;

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
void Economy::add_warehouse(Warehouse *wh)
{
	m_warehouses.push_back(wh);
}

/**
 * Remove the warehouse and its wares from the economy.
*/
void Economy::remove_warehouse(Warehouse *wh)
{
	uint32_t i;
	for (i = 0; i < m_warehouses.size(); ++i)
		if (m_warehouses[i] == wh) {
			if (i < m_warehouses.size() - 1)
				m_warehouses[i] = m_warehouses[m_warehouses.size() - 1];
			break;
		}


	//  This assert was modified, since on loading, warehouses might try to
	//  remove themselves from their own economy, though they weren't added
	//  (since they weren't initialized)
	assert(i != m_warehouses.size() || m_warehouses.empty());

	if (m_warehouses.size())
		m_warehouses.pop_back();
}

/**
 * Consider the request, try to fulfill it immediately or queue it for later.
 * Important: This must only be called by the \ref Request class.
*/
void Economy::add_request(Request & req)
{
	assert(req.is_open());
	assert(!have_request(req));

	assert(&owner());

	m_requests.push_back(&req);

	// Try to fulfill the request
	start_request_timer();
}

/**
 * \return true if the given Request is registered with the \ref Economy, false
 * otherwise
*/
bool Economy::have_request(Request & req)
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
	start_request_timer();
}


/**
 * Remove a supply from our list of supplies.
*/
void Economy::remove_supply(Supply & supply)
{
	m_supplies.remove_supply(supply);
}


bool Economy::needs_ware(Ware_Index const ware_type) const {
	size_t const nr_supplies = m_supplies.get_nrsupplies();
	uint32_t const t = target_quantity(ware_type).temporary;
	uint32_t quantity = 0;
	for (size_t i = 0; i < nr_supplies; ++i)
		if (upcast(WarehouseSupply const, warehouse_supply, &m_supplies[i])) {
			quantity += warehouse_supply->stock_wares(ware_type);
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
void Economy::do_merge(Economy & e)
{
	for (Ware_Index::value_t i = m_owner.tribe().get_nrwares().value(); i;) {
		--i;
		Target_Quantity other_tq = e.m_target_quantities[i];
		Target_Quantity & this_tq = m_target_quantities[i];
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
		m_optionswindow_registry.x = e.m_optionswindow_registry.x;
		show_options_window();
	}


	m_rebuilding = true;

	// Be careful around here. The last e->remove_flag() will cause the other
	// economy to delete itself.
	for (std::vector<Flag *>::size_type i = e.get_nrflags() + 1; --i;) {
		assert(i == e.get_nrflags());

		Flag & flag = *e.m_flags[0];

		e.do_remove_flag(flag);
		add_flag(flag);
	}

	// Fix up Supply/Request after rebuilding
	m_rebuilding = false;

	// implicitly delete the economy
	delete &e;
}

/// Flag initial_flag and all its direct and indirect neighbours are put into a
/// new economy.
void Economy::do_split(Flag & initial_flag)
{
	Economy & e = *new Economy(m_owner);

	for (Ware_Index::value_t i = m_owner.tribe().get_nrwares().value(); i;) {
		--i;
		e.m_target_quantities[i] = m_target_quantities[i];
	}

	m_rebuilding = true;
	e.m_rebuilding = true;

	// Use a vector instead of a set to ensure parallel simulation
	std::vector<Flag *> open;

	open.push_back(&initial_flag);
	while (open.size()) {
		Flag & flag = **open.rbegin();
		open.pop_back();

		if (flag.get_economy() != this)
			continue;

		// move this flag to the new economy
		remove_flag(flag);
		e.add_flag(flag);

		//  check all neighbours; if they aren't in the new economy yet, add them
		// to the list (note: roads and buildings are reassigned via Flag::set_economy)
		Neighbour_list neighbours;
		flag.get_neighbours(&neighbours);

		for (uint32_t i = 0; i < neighbours.size(); ++i) {
			Flag & n = *neighbours[i].flag;

			if (n.get_economy() == this)
				open.push_back(&n);
		}
	}

	// Fix Supply/Request after rebuilding
	m_rebuilding = false;
	e.m_rebuilding = false;

	// As long as rebalance commands are tied to specific flags, we
	// need this, because the flag that rebalance commands for us were
	// tied to might have been moved into the other economy
	start_request_timer();
}

/**
 * Make sure the request timer is running.
 */
void Economy::start_request_timer(int32_t const delta)
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
Supply * Economy::find_best_supply
	(Game & game, Request const & req, int32_t & cost)
{
	assert(req.is_open());

	Route buf_route0, buf_route1;
	Supply *best_supply = 0;
	Route *best_route = 0;
	int32_t best_cost = -1;
	Flag & target_flag = req.target_flag();

	for (size_t i = 0; i < m_supplies.get_nrsupplies(); ++i) {
		Supply & supp = m_supplies[i];

		// idle requests only get active supplies
		if (req.is_idle() and not supp.is_active()) {
			/* unless the warehouse REALLY needs the supply */
			if (req.get_priority(0) > 100) { //  100 is the 'real idle' priority
				//check if the supply is at current target
				if (&target_flag == &supp.get_position(game)->base_flag()) {
					//assert(false);
					continue;
				}
			} else if (not supp.is_active()) {
				continue;
			}
		}

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
			 	 false,
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
		return 0;

	cost = best_cost;
	return best_supply;
}

struct RequestSupplyPair {
	bool is_item;
	bool is_worker;
	Ware_Index ware;
	TrackPtr<Request> request;
	TrackPtr<Supply>  supply;
	int32_t priority;

	/**
	 * pairid is an explicit tie-breaker for comparison.
	 *
	 * Without it, the pair priority queue would use an implicit, system dependent
	 * tie breaker, which in turn causes desyncs.
	 */
	uint32_t pairid;

	struct Compare {
		bool operator()
			(RequestSupplyPair const & p1, RequestSupplyPair const & p2)
		{
			return
				p1.priority == p2.priority ? p1.pairid < p2.pairid :
				p1.priority <  p2.priority;
		}
	};
};

typedef
std::priority_queue
<RequestSupplyPair, std::vector<RequestSupplyPair>, RequestSupplyPair::Compare>
RSPairQueue;

struct RSPairStruct {
	RSPairQueue queue;
	uint32_t pairid;
	int32_t nexttimer;

	RSPairStruct() : pairid(0) {}
};

/**
 * Walk all Requests and find potential transfer candidates.
*/
void Economy::process_requests(Game & game, RSPairStruct & s)
{
	container_iterate_const(RequestList, m_requests, i) {
		Request & req = **i.current;
		int32_t cost; // estimated time in milliseconds to fulfill Request

		// We somehow get desynced request lists that don't trigger desync
		// alerts, so add info to the sync stream here.
		{
			::StreamWrite & ss = game.syncstream();
			ss.Unsigned8 (req.get_type  ());
			ss.Unsigned8 (req.get_index ().value());
			ss.Unsigned32(req.target    ().serial());
		}

		Ware_Index const ware_index = req.get_index();
		Supply * const supp = find_best_supply(game, req, cost);

		if (!supp)
			continue;

		if (!req.is_idle() and not supp->is_active()) {
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

		int32_t const priority = req.get_priority (cost);
		if (priority < 0)
			continue;

		// Otherwise, consider this request/supply pair for queueing
		RequestSupplyPair rsp;

		rsp.is_item = false;
		rsp.is_worker = false;

		switch (req.get_type()) {
		case Request::WARE:    rsp.is_item    = true; break;
		case Request::WORKER:  rsp.is_worker  = true; break;
		default:
			assert(false);
		}

		rsp.ware = ware_index;
		rsp.request  = &req;
		rsp.supply = supp;
		rsp.priority = priority;
		rsp.pairid = ++s.pairid;

		s.queue.push(rsp);
	}

	// TODO: This function should be called from time to time
	create_requested_workers (game);
}


/**
 * Walk all Requests and find requests of workers than aren't supplied. Then
 * try to create the worker at warehouses.
*/
void Economy::create_requested_workers(Game & game)
{
	/*
		Find the request of workers that can not be supplied
	*/
	if (get_nr_warehouses() > 0) {
		Tribe_Descr const & tribe = owner().tribe();
		container_iterate_const(RequestList, m_requests, j) {
			Request const & req = **j.current;

			if (!req.is_idle() && req.get_type() == Request::WORKER) {
				Ware_Index const index = req.get_index();
				int32_t num_wares = 0;
				Worker_Descr * const w_desc = tribe.get_worker_descr(index);

				// Ignore it if is a worker that cann't be buildable
				if (!w_desc->buildable())
					continue;

				for (size_t i = 0; i < m_supplies.get_nrsupplies(); ++i)
					num_wares += m_supplies[i].nr_supplies(game, req);

				// If there aren't enough supplies...
				if (num_wares == 0) {
					bool created_worker = false;
					uint32_t n_wh = 0;
					while (n_wh < get_nr_warehouses()) {
						if (m_warehouses[n_wh]->can_create_worker(game, index)) {
							m_warehouses[n_wh]->create_worker(game, index);
							created_worker = true;
							//break;
						} // if (m_warehouses[n_wh]
						++n_wh;
					} // while (n_wh < get_nr_warehouses())
					if (! created_worker) { //  fix to nearest warehouse
						Warehouse & nearest = *m_warehouses[0];
						Worker_Descr::Buildcost const & cost = w_desc->buildcost();
						container_iterate_const(Worker_Descr::Buildcost, cost, bc_it)
							if
								(Ware_Index const w_id =
								 	tribe.ware_index(bc_it.current->first.c_str()))
								nearest.set_needed(w_id, bc_it.current->second);
					}
				} // if (num_wares == 0)
			} // if (req->is_open())
		} // for (RequestList::iterator
	} // if (get_nr_warehouses())
}

/**
 * Balance Requests and Supplies by collecting and weighing pairs, and
 * starting transfers for them.
*/
void Economy::balance_requestsupply(uint32_t const timerid)
{
	if (m_request_timerid != timerid)
		return;
	++m_request_timerid;

	RSPairStruct rsps;
	rsps.nexttimer = -1;

	if (upcast(Game, game, &m_owner.egbase())) {

		//  Try to fulfill non-idle Requests.
		process_requests(*game, rsps);

		//  Now execute request/supply pairs.
		while (rsps.queue.size()) {
			RequestSupplyPair rsp = rsps.queue.top();

			rsps.queue.pop();

			if
				(!rsp.request               ||
				 !rsp.supply                ||
				 !have_request(*rsp.request) ||
				 !rsp.supply->nr_supplies(*game, *rsp.request))
			{
				rsps.nexttimer = 200;
				continue;
			}

			rsp.request->start_transfer(*game, *rsp.supply);
			rsp.request->set_last_request_time(owner().egbase().get_gametime());

			//  for multiple wares
			if (rsp.request && have_request(*rsp.request))
				rsps.nexttimer = 200;
		}

		if (rsps.nexttimer > 0) //  restart the timer, if necessary
			start_request_timer(rsps.nexttimer);
	}
}

#define CURRENT_ECONOMY_VERSION 2

void Economy::Read(FileRead & fr, Game &, Map_Map_Object_Loader *)
{
	uint16_t const version = fr.Unsigned16();

	try {
		if (1 <= version and version <= CURRENT_ECONOMY_VERSION) {
			if (2 <= version)
				try {
					Tribe_Descr const & tribe = owner().tribe();
					while (Time const last_modified = fr.Unsigned32()) {
						char const * const ware_type_name = fr.CString();
						uint32_t const permanent = fr.Unsigned32();
						uint32_t const temporary = fr.Unsigned32();
						Ware_Index const ware_type =
							tribe.ware_index(ware_type_name);
						if (not ware_type)
							log
								("WARNING: target quantity configured for \"%s\", "
								 "which is not a ware type defined in tribe %s, "
								 "ignoring\n",
								 ware_type_name, tribe.name().c_str());
						else if
							(tribe.get_ware_descr(ware_type)->default_target_quantity()
							 ==
							 std::numeric_limits<uint32_t>::max())
							log
								("WARNING: target quantity configured for %s, which "
								 "should not have target quantity, ignoring\n",
								 ware_type_name);
						else {
							Target_Quantity & tq =
								m_target_quantities[ware_type.value()];
							if (tq.last_modified)
								throw wexception
									("duplicated entry for %s", ware_type_name);
							tq.permanent         = permanent;
							tq.temporary         = temporary;
							tq.last_modified     = last_modified;
						}
					}
				} catch (_wexception const & e) {
					throw wexception("target quantities: %s", e.what());
				}
			m_request_timerid = fr.Unsigned32();
		} else {
			throw wexception("unknown version %u", version);
		}
	} catch (std::exception const & e) {
		throw wexception("economy: %s", e.what());
	}
}

void Economy::Write(FileWrite & fw, Game &, Map_Map_Object_Saver *)
{
	fw.Unsigned16(CURRENT_ECONOMY_VERSION);
	Tribe_Descr const & tribe = owner().tribe();
	for (Ware_Index i = tribe.get_nrwares(); i.value();) {
		--i;
		Target_Quantity const & tq = m_target_quantities[i.value()];
		if (Time const last_modified = tq.last_modified) {
			fw.Unsigned32(last_modified);
			fw.CString(tribe.get_ware_descr(i)->name());
			fw.Unsigned32(tq.permanent);
			fw.Unsigned32(tq.temporary);
		}
	}
	fw.Unsigned32(0); //  terminator
	fw.Unsigned32(m_request_timerid);
}


}
