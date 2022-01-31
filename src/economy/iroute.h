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

#ifndef WL_ECONOMY_IROUTE_H
#define WL_ECONOMY_IROUTE_H

namespace Widelands {

struct RoutingNode;

/**
 * This class represents a Route Interface, a virtual base class
 * for a Route used for routing of wares for workers.
 *
 * This class was introduced to hide the implementation of route
 * and hide some ugly helper functions. It is therefore not a complete
 * interface, it only shows what was needed for refactoring.
 *
 * Most parts of the code still use the Route class directly and not this
 * interface
 **/
struct IRoute {
	virtual ~IRoute() {
	}

	virtual void init(int32_t) = 0;
	virtual void insert_as_first(RoutingNode* node) = 0;
};
}  // namespace Widelands
#endif  // end of include guard: WL_ECONOMY_IROUTE_H
