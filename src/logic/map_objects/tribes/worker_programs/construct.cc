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
 * If we are currently carrying some ware ware, hand it off to the currently
 * selected immovable (\ref objvar1) for construction.
 */
bool Worker::run_construct(Game& game, State& state, const Action& /* action */) {
	Immovable* imm = dynamic_cast<Immovable*>(state.objvar1.get(game));
	if (imm == nullptr) {
		molog(game.get_gametime(), "run_construct: no objvar1 immovable set");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	imm->set_reserved_by_worker(false);
	WareInstance* ware = get_carried_ware(game);
	if (ware == nullptr) {
		molog(game.get_gametime(), "run_construct: no ware being carried");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	DescriptionIndex wareindex = ware->descr_index();
	if (!imm->construct_ware(game, wareindex)) {
		molog(game.get_gametime(), "run_construct: construct_ware failed");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	// Update consumption statistic
	get_owner()->ware_consumed(wareindex, 1);

	ware = fetch_carried_ware(game);
	ware->remove(game);

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
