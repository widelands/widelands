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

#include "logic/map_objects/tribes/worker_program.h"

namespace Widelands {

/**
 * Follow the steps of a configuration-defined program.
 * ivar1 is the next action to be performed.
 * ivar2 is used to store description indices selected by plant
 * objvar1 is used to store objects found by findobject
 * coords is used to store target coordinates found by findspace
 */
const Bob::Task Worker::taskProgram = {"program", static_cast<Bob::Ptr>(&Worker::program_update),
                                       nullptr, static_cast<Bob::Ptr>(&Worker::program_pop), false};

/**
 * Start the given program.
 */
void Worker::start_task_program(Game& game, const std::string& programname) {
	push_task(game, taskProgram);
	State& state = top_state();
	state.program = descr().get_program(programname);
	state.ivar1 = 0;
}

void Worker::program_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		molog(game.get_gametime(), "[program]: Interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	if (state.program == nullptr) {
		// This might happen as fallout of some save game compatibility fix
		molog(game.get_gametime(), "[program]: No program active\n");
		send_signal(game, "fail");
		return pop_task(game);
	}

	for (;;) {
		const WorkerProgram& program = dynamic_cast<const WorkerProgram&>(*state.program);

		if ((state.ivar1 >= 0) && (static_cast<uint32_t>(state.ivar1) >= program.get_size())) {
			return pop_task(game);
		}

		const Action& action = *program.get_action(state.ivar1);

		if ((this->*(action.function))(game, state, action)) {
			return;
		}
	}
}

void Worker::program_pop(Game& game, State& state) {
	set_program_objvar(game, state, nullptr);
}

void Worker::set_program_objvar(Game& game, State& state, MapObject* obj) {
	assert(state.task == &taskProgram);

	if (state.objvar1.get(game) != nullptr) {
		(state.objvar1.get(game))->set_reserved_by_worker(false);
	}

	state.objvar1 = obj;

	if (obj != nullptr) {
		obj->set_reserved_by_worker(true);
	}
}

}  // namespace Widelands
