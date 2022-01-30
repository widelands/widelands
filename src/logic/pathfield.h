/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_PATHFIELD_H
#define WL_LOGIC_PATHFIELD_H

#include <memory>

#include "logic/cookie_priority_queue.h"
#include "logic/map_objects/tribes/wareworker.h"

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
		bool operator()(const Pathfield& a, const Pathfield& b, WareWorker ww) const {
			return a.cost(ww) < b.cost(ww);
		}
	};

	using Queue = CookiePriorityQueue<Pathfield, LessCost>;

	Queue::Cookie heap_cookie;
	int32_t real_cost;   //  true cost up to this field
	int32_t estim_cost;  //  estimated cost till goal
	uint16_t cycle;
	uint8_t backlink;  //  how we got here (WALK_*)

	int32_t cost(WareWorker) const {
		return real_cost + estim_cost;
	}
	Queue::Cookie& cookie(WareWorker) {
		return heap_cookie;
	}
};

struct Pathfields {
	std::unique_ptr<Pathfield[]> fields;
	uint16_t cycle;

	explicit Pathfields(uint32_t nrfields);
};

/**
 * Efficiently manages \ref Pathfields instances.
 *
 * This allows the use of more than one such structure at once,
 * which is required for pathfinding reentrancy.
 */
struct PathfieldManager {
	PathfieldManager();

	void set_size(uint32_t nrfields);
	std::shared_ptr<Pathfields> allocate();

private:
	void clear(const std::shared_ptr<Pathfields>& pf);

	using List = std::vector<std::shared_ptr<Pathfields>>;

	uint32_t nrfields_;
	List list_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PATHFIELD_H
