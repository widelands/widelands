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
 * Endless loop, in which the worker calls the owning building's
 * get_building_work() function to initiate subtasks.
 * The signal "update" is used to wake the worker up after a sleeping time
 * (initiated by a false return value from get_building_work()).
 *
 * ivar1 - 0: no task has failed; 1: currently in buildingwork;
 *         2: signal failure of buildingwork
 * ivar2 - whether the worker is to be evicted
 */
const Bob::Task Worker::taskBuildingwork = {
   "buildingwork", static_cast<Bob::Ptr>(&Worker::buildingwork_update), nullptr, nullptr, true};

/**
 * Begin work at a building.
 */
void Worker::start_task_buildingwork(Game& game) {
	push_task(game, taskBuildingwork);
	State& state = top_state();
	state.ivar1 = 0;
}

void Worker::buildingwork_update(Game& game, State& state) {
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();

	upcast(Building, building, get_location(game));

	if (state.ivar1 == 1) {
		state.ivar1 = static_cast<int>(signal == "fail") * 2;
	}

	// Return to building, if necessary
	if (building == nullptr) {
		return pop_task(game);
	}

	if (game.map().get_immovable(get_position()) != building) {
		return start_task_return(game, false);  //  do not drop ware
	}

	// Get the new job
	bool const success = state.ivar1 != 2;

	// Set this *before* calling to get_building_work, because the
	// state pointer might become invalid
	state.ivar1 = 1;

	if (!building->get_building_work(game, *this, success)) {
		set_animation(game, 0);
		return skip_act();
	}
}

/**
 * Wake up the buildingwork task if it was sleeping.
 * Otherwise, the buildingwork task will update as soon as the previous task
 * is finished.
 */
void Worker::update_task_buildingwork(Game& game) {
	// After the worker is evicted and 'taskBuildingwork' is popped from the stack but before
	// `taskLeavebuilding` is started, there is a brief window of time where this function can
	// still be called, so we need to take into account that 'state' may be 'nullptr' here.
	const State* const state = get_state();
	if ((state != nullptr) && state->task == &taskBuildingwork) {
		send_signal(game, "update");
	}
}

}  // namespace Widelands
