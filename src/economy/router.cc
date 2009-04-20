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


#include "router.h"

// Package includes
#include "routing_node.h"
#include "route.h"

#include "map.h"

namespace Widelands {

/**
 * Provides the flexible priority queue to maintain the open list.
 *
 * This is more flexible than a standard priority_queue (fast boost() to
 * adjust cost)
*/
struct RoutingNodeQueue {
	RoutingNodeQueue() {}
	~RoutingNodeQueue() {}

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
	RoutingNode * pop()
	{
		if (m_data.empty())
			return 0;

		RoutingNode * head = m_data[0];

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
	void push(RoutingNode *t)
	{
		uint32_t slot = m_data.size();
		m_data.push_back(static_cast<RoutingNode *>(0));

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
	void boost(RoutingNode *t)
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
	std::vector<RoutingNode *> m_data;
};

/*************************************************************************/
/*                         Router Implementation                         */
/*************************************************************************/
Router::Router( void ) :
    mpf_cycle(0) {
}

/**
 * Calculate a route between two nodes.
 *
 * The calculated route is stored in route if it exists.
 *
 * For two nodes (Flags) from the same economy, this function should always be
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
bool Router::find_route
	(RoutingNode & start, RoutingNode & end,
	 Route * const route,
	 bool    const wait,
	 int32_t const cost_cutoff,
     Map& map,
     std::vector<RoutingNode*>& nodes)
{
	// advance the path-finding cycle
	++mpf_cycle;
	if (!mpf_cycle) { // reset all cycle fields
		for (uint32_t i = 0; i < nodes.size(); ++i)
			nodes[i]->mpf_cycle = 0;
		++mpf_cycle;
	}

	// Add the starting node into the open list
	RoutingNodeQueue Open;
	RoutingNode *current;

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

		// Loop through all neighbouring nodes
		RoutingNodeNeighbours neighbours;

		current->get_neighbours(&neighbours);

		for (uint32_t i = 0; i < neighbours.size(); ++i) {
			RoutingNode * const neighbour = neighbours[i].get_neighbour();
			int32_t cost;
			int32_t wait_cost = 0;

			//  No need to find the optimal path when only checking connectivity.
			if (neighbour == &end && !route)
				return true;

			if (wait)
				wait_cost =
					( current->get_waitcost() + neighbour->get_waitcost())
					*
					neighbours[i].get_cost()
					/
					2;
			cost = current->mpf_realcost + neighbours[i].get_cost() + wait_cost;

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

		for (RoutingNode * node = &end;; node = node->mpf_backlink) {
			route->insert_node(node);
			if (node == &start)
				break;
		}
	}

	return true;
}

}
