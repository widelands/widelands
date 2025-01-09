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
 * createbob=\<bob name\> \<bob name\> ...
 *
 * Plants a bob (critter usually, maybe also worker later on), randomly selected from one of the
 * given types.
 *
 * sparamv = possible bobs
 */
bool Worker::run_createbob(Game& game, State& state, const Action& action) {
	int32_t const idx = game.logic_rand() % action.sparamv.size();

	const std::string& bob = action.sparamv[idx];
	DescriptionIndex index = owner_.load()->tribe().worker_index(bob);
	if (owner_.load()->tribe().has_worker(index)) {
		game.create_worker(get_position(), index, owner_);
	} else {
		const DescriptionIndex critter = game.descriptions().critter_index(bob);
		if (critter == INVALID_INDEX) {
			molog(game.get_gametime(), "  WARNING: Unknown bob %s\n", bob.c_str());
			send_signal(game, "fail");
			pop_task(game);
			return true;
		}
		game.create_critter(get_position(), critter);
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
