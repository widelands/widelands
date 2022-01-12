/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
#ifndef WL_ECONOMY_ROUTER_H
#define WL_ECONOMY_ROUTER_H

#include <cstdint>
#include <functional>

#include "logic/map_objects/tribes/wareworker.h"

namespace Widelands {
class ITransportCostCalculator;
struct IRoute;
struct RoutingNode;

/**
 * This class finds the best route between Nodes (Flags) in an economy.
 * The functionality was split from Economy
 */
struct Router {
	using ResetCycleFn = std::function<void()>;

	explicit Router(const ResetCycleFn& reset);

	bool find_route(RoutingNode& start,
	                RoutingNode& end,
	                IRoute* route,
	                WareWorker type,
	                int32_t cost_cutoff,
	                ITransportCostCalculator& cost_calculator);
	uint32_t assign_cycle();

private:
	ResetCycleFn reset_;
	uint32_t mpf_cycle;  ///< pathfinding cycle, see Flag::mpf_cycle
};
}  // namespace Widelands
#endif  // end of include guard: WL_ECONOMY_ROUTER_H
