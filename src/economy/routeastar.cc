/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include "economy/routeastar.h"

#include "economy/iroute.h"
#include "economy/router.h"

namespace Widelands {

BaseRouteAStar::BaseRouteAStar(Router & router, WareWorker type) :
	m_type(type),
	mpf_cycle(router.assign_cycle())
{
}

/**
 * Recover a shortest route from one of the initial nodes
 * set up by @ref RouteAStar::push to the destination node @p to.
 * The route is stored in @p route.
 */
void BaseRouteAStar::routeto(RoutingNode & to, IRoute & route)
{
	assert(!to.cookie().is_active());
	assert(to.mpf_cycle == mpf_cycle);

	route.init(to.mpf_realcost);
	for (RoutingNode * node = &to; node; node = node->mpf_backlink)
		route.insert_as_first(node);
}

} // namespace Widelands
