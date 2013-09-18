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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#ifndef ROUTER_H
#define ROUTER_H

#include <vector>

#include <boost/function.hpp>
#include <stdint.h>

#include "logic/wareworker.h"

namespace Widelands {
struct IRoute;
struct ITransportCostCalculator;
struct RoutingNode;

/**
 * This class finds the best route between Nodes (Flags) in an economy.
 * The functionality was split from Economy
 */
struct Router {
	typedef boost::function<void ()> ResetCycleFn;

	Router(const ResetCycleFn & reset);

	bool find_route
		(RoutingNode & start, RoutingNode & end,
		 IRoute * route,
		 WareWorker type,
		 int32_t cost_cutoff,
		 ITransportCostCalculator   & cost_calculator);
	uint32_t assign_cycle();

private:
	ResetCycleFn m_reset;
	uint32_t mpf_cycle;       ///< pathfinding cycle, see Flag::mpf_cycle
};

}
#endif


