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

#include "logic/map_objects/tribes/soldier.h"

#include "logic/player.h"

namespace Widelands {

Bob::Task const Soldier::taskDie = {"die", static_cast<Bob::Ptr>(&Soldier::die_update), nullptr,
                                    static_cast<Bob::Ptr>(&Soldier::die_pop), true};

void Soldier::start_task_die(Game& game) {
	push_task(game, taskDie);
	top_state().ivar1 = game.get_gametime().get() + 1000;

	// Dead soldier is not owned by a location
	set_location(nullptr);

	const uint32_t anim =
	   descr().get_rand_anim(game, combat_walking_ == CD_COMBAT_W ? "die_w" : "die_e", this);
	start_task_idle(game, anim, 1000);
}

void Soldier::die_update(Game& game, State& state) {
	std::string signal = get_signal();
	molog(game.get_gametime(), "[die] update for player %u's soldier: signal = \"%s\"\n",
	      owner().player_number(), signal.c_str());

	if (!signal.empty()) {
		signal_handled();
	}

	if ((state.ivar1 >= 0) && (Time(state.ivar1) > game.get_gametime())) {
		return schedule_act(game, Time(state.ivar1) - game.get_gametime());
	}

	// When task updated, dead is near!
	return pop_task(game);
}

void Soldier::die_pop(Game& game, State& /* state */) {
	// Destroy the soldier!
	molog(game.get_gametime(), "[die] soldier %u has died\n", serial());
	schedule_destroy(game);
}

}  // namespace Widelands
