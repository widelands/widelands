/*
 * Copyright (C) 2008-2010 by the Widelands Development Team
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

#ifndef PATHFIELD_H
#define PATHFIELD_H

#include <vector>

#include <boost/shared_ptr.hpp>
#include <stdint.h>

#include "cookie_priority_queue.h"

namespace Widelands {

/**
 * Used in pathfinding. For better encapsulation, pathfinding structures
 * are separate from normal fields
 *
 * Costs are in milliseconds to walk.
 *
 * Note: member sizes chosen so that we get a 16byte (=nicely aligned)
 * structure
 */
struct Pathfield {
	struct LessCost {
		bool operator()(const Pathfield & a, const Pathfield & b) const {
			return a.cost() < b.cost();
		}
	};

	typedef cookie_priority_queue<Pathfield, LessCost> Queue;

	Queue::cookie heap_cookie;
	int32_t real_cost;  //  true cost up to this field
	int32_t estim_cost; //  estimated cost till goal
	uint16_t cycle;
	uint8_t  backlink;   //  how we got here (WALK_*)

	int32_t cost() const {return real_cost + estim_cost;}
	Queue::cookie & cookie() {return heap_cookie;}
};

struct Pathfields {
	std::unique_ptr<Pathfield[]> fields;
	uint16_t cycle;

	Pathfields(uint32_t nrfields);
};

/**
 * Efficiently manages \ref Pathfields instances.
 *
 * This allows the use of more than one such structure at once,
 * which is required for pathfinding reentrancy.
 */
struct PathfieldManager {
	PathfieldManager();

	void setSize(uint32_t nrfields);
	boost::shared_ptr<Pathfields> allocate();

private:
	void clear(const boost::shared_ptr<Pathfields> & pf);

	typedef std::vector<boost::shared_ptr<Pathfields> > List;

	uint32_t m_nrfields;
	List m_list;
};

}

#endif
