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

#include "economy/flag.h"

namespace Widelands {

/**
 * Wait for available capacity on a flag.
 */
const Bob::Task Worker::taskWaitforcapacity = {
   "waitforcapacity", static_cast<Bob::Ptr>(&Worker::waitforcapacity_update), nullptr,
   static_cast<Bob::Ptr>(&Worker::waitforcapacity_pop), true};

/**
 * Checks the capacity of the flag.
 *
 * If there is none, a wait task is pushed, and the worker is added to the
 * flag's wait queue. The function returns true in this case.
 * If the flag still has capacity, the function returns false and doesn't
 * act at all.
 */
bool Worker::start_task_waitforcapacity(Game& game, Flag& flag) {
	if (flag.has_capacity()) {
		return false;
	}

	push_task(game, taskWaitforcapacity);

	top_state().objvar1 = &flag;

	flag.wait_for_capacity(game, *this);

	return true;
}

void Worker::waitforcapacity_update(Game& game, State& /* state */) {
	std::string signal = get_signal();

	if (!signal.empty()) {
		if (signal == "wakeup") {
			signal_handled();
		}
		return pop_task(game);
	}

	return skip_act();  //  wait indefinitely
}

void Worker::waitforcapacity_pop(Game& game, State& state) {
	if (upcast(Flag, flag, state.objvar1.get(game))) {
		flag->skip_wait_for_capacity(game, *this);
	}
}

/**
 * Called when the flag we waited on has now got capacity left.
 * Return true if we actually woke up due to this.
 */
bool Worker::wakeup_flag_capacity(Game& game, Flag& flag) {
	if (State const* const state = get_state()) {
		if (state->task == &taskWaitforcapacity) {
			molog(game.get_gametime(), "[waitforcapacity]: Wake up: flag capacity.\n");

			if (state->objvar1.get(game) != &flag) {
				throw wexception("MO(%u): wakeup_flag_capacity: Flags do not match.", serial());
			}
			send_signal(game, "wakeup");
			return true;
		}
	}
	return false;
}

}  // namespace Widelands
