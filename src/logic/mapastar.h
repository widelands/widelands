/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAPASTAR_H
#define WL_LOGIC_MAPASTAR_H

#include <memory>

#include "logic/map.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "logic/pathfield.h"

namespace Widelands {

struct MapAStarBase {
	explicit MapAStarBase(Map& m, WareWorker type)
	   : map(m), pathfields(m.pathfieldmgr_->allocate()), queue(type) {
	}

	bool empty() const {
		return queue.empty();
	}
	void pathto(Coords dest, Path& path) const;

protected:
	Pathfield& pathfield(Coords where) {
		return pathfields->fields[map.get_index(where, map.get_width())];
	}
	const Pathfield& pathfield(Coords where) const {
		return pathfields->fields[map.get_index(where, map.get_width())];
	}

	Map& map;
	std::shared_ptr<Pathfields> pathfields;
	Pathfield::Queue queue;
};

struct StepEvalAStar {
	explicit StepEvalAStar(Coords target)
	   : target_(target), estimator_bias_(0), conservative_(true), swim_(false) {
	}

	int32_t estimate(Map& map, FCoords pos) const {
		int32_t est = estimator_bias_;
		if (conservative_)
			est += map.calc_cost_lowerbound(pos, target_);
		else
			est += map.calc_cost_estimate(pos, target_);
		return est;
	}

	int32_t stepcost(
	   const Map& map, FCoords from, int32_t /* fromcost */, WalkingDir dir, FCoords to) const {
		if ((swim_ && !(to.field->nodecaps() & MOVECAPS_SWIM)) ||
		    (!swim_ && !(to.field->nodecaps() & MOVECAPS_WALK)))
			return -1;

		return map.calc_cost(from, dir);
	}

	Coords target_;
	int32_t estimator_bias_;
	bool conservative_;
	bool swim_;
};

/**
 * Allow customized A-star type searches through a map.
 *
 * The template parameter must be a struct type that matches the following "interface":
 * @code
 * struct StepEval {
 *   int32_t estimate(Map & map, FCoords pos) const;
 *   int32_t stepcost(Map & map, FCoords from, int32_t fromcost, WalkingDir dir, FCoords to) const;
 * };
 * @endcode
 * The estimate function is called once for every newly discovered field,
 * and is supposed to return an A-star type estimate for the distance to the goal.
 * This estimate must be conservative if A-star is to find an optimal path.
 *
 * The stepcost function is called for every newly discovered edge. It must return
 * the cost for traversing the edge (which may be zero), or a negative number
 * if the edge cannot be traversed.
 *
 * Typical usage:
 * @code
 * MapAStar<Helper> astar(map, helper);
 * astar.push(start_pos);
 * int32_t cost;
 * FCoords cur;
 * while (astar.step(cur, cost)) {
 *   if (cur == dest)
 *     break;
 * }
 * @endcode
 */
template <typename StepEval> struct MapAStar : MapAStarBase {
	MapAStar(Map& map_, const StepEval& eval_, WareWorker type)
	   : MapAStarBase(map_, type), eval(eval_) {
	}

	void push(Coords pos, int32_t cost = 0);
	bool step(FCoords& cur, int32_t& cost);

private:
	const StepEval& eval;
};

/**
 * Add the given field as an initial point of search
 * unless a lower cost path to it has already been found,
 * or it has already been returned from @ref step.
 */
template <typename StepEval> void MapAStar<StepEval>::push(Coords pos, int32_t cost) {
	Pathfield& pf = pathfield(pos);
	if (pf.cycle != pathfields->cycle) {
		pf.cycle = pathfields->cycle;
		pf.backlink = IDLE;
		pf.real_cost = cost;
		pf.estim_cost = eval.estimate(map, map.get_fcoords(pos));
		queue.push(&pf);
	} else if (pf.cookie(queue.type()).is_active() && cost <= pf.real_cost) {
		pf.backlink = IDLE;
		pf.real_cost = cost;
		queue.decrease_key(&pf);
	}
}

/**
 * Retrieve the next field from the queue, discover its neighbours,
 * and return it in @p cur.
 *
 * @return true if a field was returned, or false if the search is over.
 */
template <typename StepEval> bool MapAStar<StepEval>::step(FCoords& cur, int32_t& cost) {
	if (queue.empty())
		return false;

	Pathfield* curpf = queue.top();
	queue.pop(curpf);

	cur.field = &map[curpf - pathfields->fields.get()];
	map.get_coords(*cur.field, cur);
	cost = curpf->real_cost;

	// avoid bias by using different orders when pathfinding
	static const int8_t order1[] = {WALK_NW, WALK_NE, WALK_E, WALK_SE, WALK_SW, WALK_W};
	static const int8_t order2[] = {WALK_NW, WALK_W, WALK_SW, WALK_SE, WALK_E, WALK_NE};
	int8_t const* direction = ((cur.x + cur.y) & 1) ? order1 : order2;

	// Check all the 6 neighbours
	for (uint32_t i = 6; i; i--, direction++) {
		FCoords neighb(map.get_neighbour(cur, *direction));
		Pathfield& neighbpf = pathfield(neighb);

		// Field is closed already
		if (neighbpf.cycle == pathfields->cycle && !neighbpf.heap_cookie.is_active())
			continue;

		int32_t stepcost = eval.stepcost(map, cur, cost, WalkingDir(*direction), neighb);

		if (stepcost < 0)
			continue;

		int32_t newcost = cost + stepcost;

		if (neighbpf.cycle != pathfields->cycle) {
			neighbpf.cycle = pathfields->cycle;
			neighbpf.real_cost = newcost;
			neighbpf.estim_cost = eval.estimate(map, neighb);
			neighbpf.backlink = *direction;
			queue.push(&neighbpf);
		} else if (neighbpf.real_cost > newcost) {
			neighbpf.real_cost = newcost;
			neighbpf.backlink = *direction;
			queue.decrease_key(&neighbpf);
		}
	}

	return true;
}

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAPASTAR_H
