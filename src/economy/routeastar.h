/*
 * Copyright (C) 2011-2018 by the Widelands Development Team
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

#ifndef WL_ECONOMY_ROUTEASTAR_H
#define WL_ECONOMY_ROUTEASTAR_H

#include "economy/itransport_cost_calculator.h"
#include "economy/routing_node.h"

namespace Widelands {

struct IRoute;
struct Router;

struct BaseRouteAStar {
	BaseRouteAStar(Router& router, WareWorker type);

	void routeto(RoutingNode& to, IRoute& route);

protected:
	RoutingNode::Queue open_;
	WareWorker type_;
	RoutingNodeNeighbours neighbours_;
	uint32_t mpf_cycle;
};

/**
 * This helper facilitates semi-custom A-star type routing over
 * the @ref RoutingNode (i.e. @ref Flag) network of an eocnomy.
 *
 * The template is parameterized by an @em estimator. The estimator
 * is a functor that takes a reference to a @ref RoutingNode and
 * returns a cost estimate for the remaining distance.
 * As usual for A-star, the shortest path is only guaranteed to be
 * found if the estimator always returns an under-estimate.
 *
 * Typical usage is something like the following:
 * @code
 * RouteAStar<Estimator> astar(router, wwWARE, estimator);
 * astar.push(startnode);
 *
 * while (RoutingNode * current = astar.step()) {
 *   if (current == &destnode) {
 *     astar.routeto(current, route);
 *     break;
 *   }
 * }
 * @endcode
 *
 * @warning It is currently impossible to have two RouteAStar instances
 * running concurrently.
 *
 * The closedset of A* is emulated by giving each node a field that indicates when it was
 * last touched in routing (mpf_cycle). The router itself also has such a field,
 * which is increased for each run of the algorithm, see @ref Router::assign_cycle.
 *
 * @see MapAStar
 */
template <typename Est_> struct RouteAStar : BaseRouteAStar {
	using Estimator = Est_;

	RouteAStar(Router& router, WareWorker type, const Estimator& est = Estimator());

	void push(RoutingNode& node, int32_t cost = 0, RoutingNode* backlink = nullptr);
	RoutingNode* step();

private:
	Estimator estimator_;
};

/**
 * Initialize the A-star run. The @p type affects the calculations for the actual cost,
 * while @p est is used to estimate the remaining cost to destination at each newly
 * seen node.
 */
template <typename Est_>
RouteAStar<Est_>::RouteAStar(Router& router, WareWorker type, const Estimator& est)
   : BaseRouteAStar(router, type), estimator_(est) {
}

/**
 * Update the real cost to reach @p node.
 *
 * In particular, this is used to populate the open queue with the initial source nodes.
 */
template <typename Est_>
void RouteAStar<Est_>::push(RoutingNode& node, int32_t cost, RoutingNode* backlink) {
	if (node.mpf_cycle != mpf_cycle) {
		node.mpf_cycle = mpf_cycle;
		node.mpf_backlink = backlink;
		node.mpf_realcost = cost;
		node.mpf_estimate = estimator_(node);
		open_.push(&node);
	} else if (node.mpf_cookie.is_active() && cost <= node.mpf_realcost) {
		node.mpf_backlink = backlink;
		node.mpf_realcost = cost;
		open_.decrease_key(&node);
	}
}

template <typename Est_> RoutingNode* RouteAStar<Est_>::step() {
	if (open_.empty())
		return nullptr;

	// Keep the neighbours vector around to avoid excessive amounts of memory
	// allocations and frees.
	// Note that the C++ standard does not state whether clear will reset
	// the reserved memory, but most implementations do not.
	neighbours_.clear();

	RoutingNode* current = open_.top();
	open_.pop(current);

	current->get_neighbours(type_, neighbours_);

	for (RoutingNodeNeighbour& temp_neighbour : neighbours_) {
		RoutingNode& neighbour = *temp_neighbour.get_neighbour();

		// We have already found the best path
		// to this neighbour, no need to visit it again.
		if (neighbour.mpf_cycle == mpf_cycle && !neighbour.cookie().is_active())
			continue;

		int32_t realcost = current->mpf_realcost + temp_neighbour.get_cost();
		push(neighbour, realcost, current);
	}

	return current;
}

/**
 * This estimator, for use with @ref RouteAStar, biases the search
 * so that it should move quickly towards the given destination.
 */
struct AStarEstimator {
	AStarEstimator(ITransportCostCalculator& calc, RoutingNode& dest)
	   : calc_(calc), dest_(dest.get_position()) {
	}

	int32_t operator()(RoutingNode& current) const {
		return calc_.calc_cost_estimate(current.get_position(), dest_);
	}

private:
	ITransportCostCalculator& calc_;
	Coords dest_;
};

/**
 * This estimator, for use with @ref RouteAStar, always returns zero,
 * which means that the resulting search is effectively Dijkstra's algorithm.
 */
struct AStarZeroEstimator {
	int32_t operator()(RoutingNode&) const {
		return 0;
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_ROUTEASTAR_H
