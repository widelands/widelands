/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#include "economy/router.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#include "economy/iroute.h"
#include "economy/itransport_cost_calculator.h"
#include "economy/routeastar.h"
#include "economy/routing_node.h"

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
 * Calculate a route between two nodes.
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
 * \param type whether the route is being calculated for a ware or a worker;
 *        this affects the cost calculations
 * \param cost_cutoff if non-negative: maximum cost for desirable routes.
 *        If no route cheaper than this can be found, return false.
 *        Set this parameter to -1 to allow arbitrarily large routes.
 *
 * \return true if a route has been found, false otherwise
 */
bool Router::find_route
	(RoutingNode & start, RoutingNode & end,
	 IRoute * const route,
	 WareWorker const type,
	 int32_t const cost_cutoff,
	 ITransportCostCalculator & cost_calculator)
{
	RouteAStar<AStarEstimator> astar(*this, type, AStarEstimator(cost_calculator, end));

	astar.push(start);

	while (RoutingNode * current = astar.step()) {
		if (cost_cutoff >= 0 && current->mpf_realcost > cost_cutoff)
			return false;

		if (current == &end) {
			// found our goal
			if (route)
				astar.routeto(end, *route);
			return true;
		}
	}

	return false;
}

} // namespace Widelands
