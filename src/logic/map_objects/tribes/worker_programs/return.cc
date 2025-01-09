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
 * Return home, drop any ware we're carrying onto our building's flag.
 *
 * iparam1 = 0: don't drop ware on flag, 1: do drop ware on flag
 */
bool Worker::run_return(Game& game, State& state, const Action& action) {
	++state.ivar1;
	start_task_return(game, action.iparam1 != 0);
	return true;
}

}  // namespace Widelands
