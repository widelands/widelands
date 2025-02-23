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
 * ivar1 - 0: don't change location; 1: change location to the flag
 * objvar1 - the building we're leaving
 */
const Bob::Task Worker::taskLeavebuilding = {
   "leavebuilding", static_cast<Bob::Ptr>(&Worker::leavebuilding_update), nullptr,
   static_cast<Bob::Ptr>(&Worker::leavebuilding_pop), true};

/**
 * Leave the current building.
 * Waits on the buildings leave wait queue if necessary.
 *
 * If changelocation is true, change the location to the flag once we're
 * outside.
 */
void Worker::start_task_leavebuilding(Game& game, bool const changelocation) {
	// Set the wait task
	push_task(game, taskLeavebuilding);
	State& state = top_state();
	state.ivar1 = static_cast<int32_t>(changelocation);
	state.objvar1 = &dynamic_cast<Building&>(*get_location(game));
}

void Worker::leavebuilding_update(Game& game, State& state) {
	const std::string& signal = get_signal();

	if (signal == "wakeup") {
		signal_handled();
	} else if (!signal.empty()) {
		return pop_task(game);
	}

	upcast(Building, building, get_location(game));
	if (building == nullptr) {
		return pop_task(game);
	}

	Flag& baseflag = building->base_flag();

	if (get_position() == building->get_position()) {
		assert(building == state.objvar1.get(game));
		if (!building->leave_check_and_wait(game, *this)) {
			return skip_act();
		}

		if (state.ivar1 != 0) {
			set_location(&baseflag);
		}

		return start_task_move(
		   game, WALK_SE, descr().get_right_walk_anims(does_carry_ware(), this), true);
	}
	const Coords& flagpos = baseflag.get_position();

	if (state.ivar1 != 0) {
		set_location(&baseflag);
	}

	if (get_position() == flagpos) {
		return pop_task(game);
	}

	if (!start_task_movepath(
	       game, flagpos, 0, descr().get_right_walk_anims(does_carry_ware(), this))) {
		molog(game.get_gametime(),
		      "[leavebuilding]: outside of building, but failed to walk back to flag");
		set_location(nullptr);
		return pop_task(game);
	}
}

void Worker::leavebuilding_pop(Game& game, State& state) {
	// As of this writing, this is only really necessary when the task
	// is interrupted by a signal. Putting this in the pop() method is just
	// defensive programming, in case leavebuilding_update() changes
	// in the future.
	//
	//  The if-statement is needed because this is (unfortunately) also called
	//  when the Worker is deallocated when shutting down the simulation. Then
	//  the building might not exist any more.
	if (MapObject* const building = state.objvar1.get(game)) {
		dynamic_cast<Building&>(*building).leave_skip(game, *this);
	}
}

/**
 * Called when the given building allows us to leave it.
 * \return true if we actually woke up due to this.
 */
bool Worker::wakeup_leave_building(Game& game, Building& building) {
	if (State const* const state = get_state()) {
		if (state->task == &taskLeavebuilding) {
			if (state->objvar1.get(game) != &building) {
				throw wexception("MO(%u): [waitleavebuilding]: buildings do not match", serial());
			}
			send_signal(game, "wakeup");
			return true;
		}
	}
	return false;
}

}  // namespace Widelands
