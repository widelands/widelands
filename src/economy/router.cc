/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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
#include "iroute.h"
#include "itransport_cost_calculator.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace Widelands {

/*************************************************************************/
/*                         Router Implementation                         */
/*************************************************************************/
Router::Router(const ResetCycleFn & reset) : m_reset(reset), mpf_cycle(0) {}

uint32_t Router::assign_cycle()
{
	++mpf_cycle;
	if (!mpf_cycle) { // reset all cycle fields
		m_reset();
		++mpf_cycle;
	}

	return mpf_cycle;
}

/**
 * Calculate a route between two nodes. This is using the A* algorithm, the
 * closedset is 'emulated' by giving each node an index field of when it was
 * last touched in routing (mpf_cycle). The router itself also has such a field
 * and increases it every time find_route is called.
 *
 * The calculated route is stored in route if it exists.
 *
 * For two nodes (Flags) from the same economy, this function should always be
 * successful, except when it's called from check_split() or if cost_cutoff is
 * specified and no cheap route could be found.
 *
 * \note route will be init()ed before storing the result.
 *
 * \param start, end start and endpoint of the route
 * \param route the calculated route, can be 0 to only check connectivity
 * \param wait If true, the cost for waiting on a flag is considered (if this
 *        route is for a ware)
 * \param cost_cutoff maximum cost for desirable routes. If no route cheaper
 *        than this can be found, return false
 *
 * \return true if a route has been found, false otherwise
*/
bool Router::find_route
	(RoutingNode & start, RoutingNode & end,
	 IRoute * const route,
	 bool    const wait,
	 int32_t const cost_cutoff,
	 ITransportCostCalculator   & cost_calculator)
{
	assign_cycle();

	// Add the starting node into the open list
	RoutingNode::Queue Open;
	RoutingNode * current;

	start.mpf_cycle    = mpf_cycle;
	start.mpf_backlink = 0;
	start.mpf_realcost = 0;
	start.mpf_estimate =
		cost_calculator.calc_cost_estimate
			(start.get_position(), end.get_position());

	Open.push(&start);

	for (;;) {
		if (Open.empty()) // path not found
			return false;
		current = Open.top();
		Open.pop(current);
		if (current == &end)
			break; // found our goal

		if (cost_cutoff >= 0 && current->mpf_realcost > cost_cutoff)
			return false;

		// Loop through all neighbouring nodes
		RoutingNodeNeighbours neighbours;

		current->get_neighbours(neighbours);

		for (uint32_t i = 0; i < neighbours.size(); ++i) {
			RoutingNode & neighbour = *neighbours[i].get_neighbour();
			int32_t cost;
			int32_t wait_cost = 0;

			//  No need to find the optimal path when only checking connectivity.
			if (&neighbour == &end && !route)
				return true;

			// We have already found the best path
			// to this neighbour, no need to visit it again.
			if
				(neighbour.mpf_cycle == mpf_cycle &&
				 !neighbour.cookie().is_active())
				continue;

			/*
			 * If this is a ware transport (so we have to wait on full flags)
			 * add a weighting factor depending on the fullness of the two
			 * flags onto the general cost
			 */
			if (wait) {
				wait_cost =
					(current->get_waitcost() + neighbour.get_waitcost())
					* neighbours[i].get_cost() / 2;
			}
			cost = current->mpf_realcost + neighbours[i].get_cost() + wait_cost;

			if (neighbour.mpf_cycle != mpf_cycle) {
				// add to open list
				neighbour.mpf_cycle = mpf_cycle;
				neighbour.mpf_realcost = cost;
				neighbour.mpf_estimate = cost_calculator.calc_cost_estimate
					(neighbour.get_position(), end.get_position());
				neighbour.mpf_backlink = current;
				Open.push(&neighbour);
			} else if (cost < neighbour.mpf_realcost) {
				// found a better path to a field that's already Open
				neighbour.mpf_realcost = cost;
				neighbour.mpf_backlink = current;
				Open.decrease_key(&neighbour);
			}
		}
	}

	// Unwind the path to form the route
	if (route) {
		route->init(end.mpf_realcost);

		for (RoutingNode * node = &end;; node = node->mpf_backlink) {
			route->insert_as_first(node);
			if (node == &start)
				break;
		}
	}

	return true;
}

}
