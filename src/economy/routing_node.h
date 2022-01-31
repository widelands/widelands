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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_ECONOMY_ROUTING_NODE_H
#define WL_ECONOMY_ROUTING_NODE_H

#include "logic/cookie_priority_queue.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct Flag;
struct RoutingNode;

/**
 * @see RoutingNode::get_neighbours
 */
struct RoutingNodeNeighbour {
	RoutingNodeNeighbour(RoutingNode* const f, int32_t const cost) : nb_(f), cost_(cost) {
	}
	RoutingNode* get_neighbour() const {
		return nb_;
	}
	int32_t get_cost() const {
		return cost_;
	}

private:
	RoutingNode* nb_;
	int32_t cost_;  /// Cost to get from me to the neighbour (Cost for road)
};
using RoutingNodeNeighbours = std::vector<RoutingNodeNeighbour>;

/**
 * A routing node is a field with a cost attached to it
 * plus some status variables needed for path finding.
 *
 * The only routing Node in Widelands is a Flag currently,
 * this interface has been extracted to reduce coupling
 */
struct RoutingNode {
	struct LessCost {
		bool operator()(const RoutingNode& a, const RoutingNode& b, WareWorker type) const {
			return a.cost(type) < b.cost(type);
		}
	};
	using Queue = CookiePriorityQueue<RoutingNode, LessCost>;

	uint32_t mpf_cycle_ware;
	Queue::Cookie mpf_cookie_ware;
	int32_t mpf_realcost_ware;       ///< real cost of getting to this flag
	RoutingNode* mpf_backlink_ware;  ///< flag where we came from
	int32_t mpf_estimate_ware;       ///< estimate of cost to destination

	uint32_t mpf_cycle_worker;
	Queue::Cookie mpf_cookie_worker;
	int32_t mpf_realcost_worker;       ///< real cost of getting to this flag
	RoutingNode* mpf_backlink_worker;  ///< flag where we came from
	int32_t mpf_estimate_worker;       ///< estimate of cost to destination

public:
	RoutingNode()
	   : mpf_cycle_ware(0),
	     mpf_realcost_ware(0),
	     mpf_backlink_ware(nullptr),
	     mpf_estimate_ware(0),
	     mpf_cycle_worker(0),
	     mpf_realcost_worker(0),
	     mpf_backlink_worker(nullptr),
	     mpf_estimate_worker(0) {
	}
	virtual ~RoutingNode() {
	}

	void reset_path_finding_cycle(WareWorker which) {
		switch (which) {
		case wwWARE:
			mpf_cycle_ware = 0;
			break;
		case wwWORKER:
			mpf_cycle_worker = 0;
			break;
		}
	}

	int32_t cost(WareWorker which) const {
		return (which == wwWARE) ? mpf_realcost_ware + mpf_estimate_ware :
                                 mpf_realcost_worker + mpf_estimate_worker;
	}
	Queue::Cookie& cookie(WareWorker which) {
		return which == wwWARE ? mpf_cookie_ware : mpf_cookie_worker;
	}

	virtual Flag& base_flag() = 0;
	virtual void get_neighbours(WareWorker type, RoutingNodeNeighbours&) = 0;
	virtual const Coords& get_position() const = 0;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_ROUTING_NODE_H
