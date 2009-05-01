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

#ifndef S__ROUTER_H
#define S__ROUTER_H

#include <stdint.h>
#include <vector>

namespace Widelands {
class IRoute;
class ITransportCostCalculator;
class RoutingNode;

/**
 * This class finds the best route between Nodes (Flags) in an economy.
 * The functionality was split from Economy
 */
struct Router {
	Router();

	bool find_route
		(RoutingNode & start, RoutingNode & end,
		 IRoute * route,
		 bool    wait,
		 int32_t cost_cutoff,
		 ITransportCostCalculator   & cost_calculator,
		 std::vector<RoutingNode *> & nodes);

private:
	uint32_t mpf_cycle;       ///< pathfinding cycle, see Flag::mpf_cycle
};

}
#endif


