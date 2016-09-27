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

#ifndef WL_ECONOMY_ROUTING_NODE_H
#define WL_ECONOMY_ROUTING_NODE_H

#include <vector>

#include "logic/cookie_priority_queue.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct Flag;
struct RoutingNode;
struct Road;

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
		bool operator()(const RoutingNode& a, const RoutingNode& b) const {
			return a.cost() < b.cost();
		}
	};
	using Queue = CookiePriorityQueue<RoutingNode, LessCost>;

	uint32_t mpf_cycle;
	Queue::Cookie mpf_cookie;
	int32_t mpf_realcost;       ///< real cost of getting to this flag
	RoutingNode* mpf_backlink;  ///< flag where we came from
	int32_t mpf_estimate;       ///< estimate of cost to destination

public:
	RoutingNode() : mpf_cycle(0), mpf_realcost(0), mpf_backlink(nullptr), mpf_estimate(0) {
	}
	virtual ~RoutingNode() {
	}

	void reset_path_finding_cycle() {
		mpf_cycle = 0;
	}

	int32_t cost() const {
		return mpf_realcost + mpf_estimate;
	}
	Queue::Cookie& cookie() {
		return mpf_cookie;
	}

	virtual Flag& base_flag() = 0;
	virtual void get_neighbours(WareWorker type, RoutingNodeNeighbours&) = 0;
	virtual const Coords& get_position() const = 0;
};
}

#endif  // end of include guard: WL_ECONOMY_ROUTING_NODE_H
