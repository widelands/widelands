/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#include "logic/map_objects/tribes/worker.h"

namespace Widelands {

/**
 * repeatsearch=\<repeat #\> \<radius\> \<subcommand\>
 *
 * Walk around the starting point randomly within a certain radius, and
 * execute the subcommand for some of the fields.
 *
 * iparam1 = maximum repeat #
 * iparam2 = radius
 * sparam1 = subcommand
 */
bool Worker::run_repeatsearch(Game& game, State& state, const Action& action) {
	molog(game.get_gametime(), "  Start Repeat Search (%i attempts, %i radius -> %s)\n",
	      action.iparam1, action.iparam2, action.sparam1.c_str());

	++state.ivar1;
	start_task_geologist(game, action.iparam1, action.iparam2, action.sparam1);
	return true;
}

}  // namespace Widelands
