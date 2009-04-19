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

#ifndef S__ROUTING_NODE_H
#define S__ROUTING_NODE_H

// Only for Neighbour_list
#include <vector>

namespace Widelands {
class RoutingNode;
struct Flag;
struct Road;

struct Neighbour {
	Flag * flag;
	Road * road;
	int32_t    cost;
};
typedef std::vector<Neighbour> Neighbour_list;



/**
 * A routing node is a field with a cost attached to it
 * plus some status variables needed for path finding. 
 *
 * The only routing Node in Widelands is a Flag currently, 
 * this interface has been extracted to reduce coupling
 */
class RoutingNode {
    friend class Router;
    friend class FlagQueue;

private:
	uint32_t                    mpf_cycle;
	int32_t                     mpf_heapindex;
	int32_t                     mpf_realcost; ///< real cost of getting to this flag
	RoutingNode                *mpf_backlink; ///< flag where we came from
	int32_t                     mpf_estimate; ///< estimate of cost to destination

public:
    RoutingNode() {} 
    virtual ~RoutingNode() {}

    void reset_path_finding_cycle(void) { mpf_cycle = 0; }

	int32_t cost() const {return mpf_realcost + mpf_estimate;}
	
	virtual void get_neighbours(Neighbour_list *) = 0;
    virtual Coords get_position() const = 0; 
};

}
#endif 


