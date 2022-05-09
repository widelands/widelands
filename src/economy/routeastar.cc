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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "economy/routeastar.h"

#include <cassert>

#include "base/wexception.h"
#include "economy/iroute.h"
#include "economy/router.h"

namespace Widelands {

BaseRouteAStar::BaseRouteAStar(Router& router, WareWorker type)
   : open_(type), type_(type), mpf_cycle(router.assign_cycle()) {
}

/**
 * Recover a shortest route from one of the initial nodes
 * set up by @ref RouteAStar::push to the destination node @p to.
 * The route is stored in @p route.
 */
void BaseRouteAStar::routeto(RoutingNode& to, IRoute& route) {
	if (to.cookie(type_).is_active()) {
		throw wexception("BaseRouteAStar::routeto should not have an active cookie.");
	}
	assert(mpf_cycle == (type_ == wwWARE ? to.mpf_cycle_ware : to.mpf_cycle_worker));

	route.init(type_ == wwWARE ? to.mpf_realcost_ware : to.mpf_realcost_worker);
	for (RoutingNode* node = &to; node != nullptr;
	     node = (type_ == wwWARE ? node->mpf_backlink_ware : node->mpf_backlink_worker)) {
		route.insert_as_first(node);
	}
}

}  // namespace Widelands
