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
 * scout=\<radius\> \<time\>
 *
 * Find a spot that is in the fog of war and go there to see what's up.
 *
 * iparam1 = radius where the scout initially searches for unseen fields
 * iparam2 = maximum search time (in msecs)
 */
bool Worker::run_scout(Game& game, State& state, const Action& action) {
	molog(game.get_gametime(), "  Try scouting for %i ms with search in radius of %i\n",
	      action.iparam2, action.iparam1);
	if (upcast(ProductionSite, productionsite, get_location(game))) {
		productionsite->unnotify_player();
	}
	++state.ivar1;
	start_task_scout(game, action.iparam1, action.iparam2);
	// state reference may be invalid now
	return true;
}

}  // namespace Widelands
