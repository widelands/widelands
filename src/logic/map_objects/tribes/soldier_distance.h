/*
 * Copyright (C) 2002-2026 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_DISTANCE_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_DISTANCE_H

namespace Widelands {

class Soldier;

/** Describes the distance of a soldier to a target, with an operator to order by distance. */
struct SoldierDistance {
	Soldier* s;
	int dist;

	SoldierDistance(Soldier* a, int d) : dist(d) {
		s = a;
	}

	struct Greater {
		bool operator()(const SoldierDistance& a, const SoldierDistance& b) {
			return (a.dist > b.dist);
		}
	};
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_SOLDIER_DISTANCE_H
