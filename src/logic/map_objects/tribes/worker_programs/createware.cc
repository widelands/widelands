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

#include "logic/player.h"

namespace Widelands {

/**
 * createware=\<waretype\>
 *
 * The worker will create and carry an ware of the given type.
 *
 * sparam1 = ware name
 */
bool Worker::run_createware(Game& game, State& state, const Action& action) {

	if (WareInstance* const ware = fetch_carried_ware(game)) {
		molog(game.get_gametime(), "  Still carrying a ware! Delete it.\n");
		ware->schedule_destroy(game);
	}

	Player& player = *get_owner();
	DescriptionIndex const wareid(action.iparam1);
	WareInstance& ware = *new WareInstance(wareid, player.tribe().get_ware_descr(wareid));
	ware.init(game);

	set_carried_ware(game, &ware);

	// For statistics, inform the user that a ware was produced
	player.ware_produced(wareid);

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
