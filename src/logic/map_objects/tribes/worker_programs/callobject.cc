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

#include "logic/map_objects/world/critter.h"

namespace Widelands {

/**
 * callobject=\<command\>
 *
 * Cause the currently selected object to execute the given program.
 *
 * sparam1 = object command name
 */
bool Worker::run_callobject(Game& game, State& state, const Action& action) {
	MapObject* const obj = state.objvar1.get(game);

	if (obj == nullptr) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	if (upcast(Immovable, immovable, obj)) {
		immovable->switch_program(game, action.sparam1);
	} else if (upcast(Bob, bob, obj)) {
		if (upcast(Critter, crit, bob)) {
			crit->reset_tasks(game);  //  TODO(unknown): ask the critter more nicely
			crit->start_task_program(game, action.sparam1);
		} else if (upcast(Worker, w, bob)) {
			w->reset_tasks(game);  //  TODO(unknown): ask the worker more nicely
			w->start_task_program(game, action.sparam1);
		} else {
			throw wexception("MO(%u): [actObject]: bad bob type %s", serial(),
			                 to_string(bob->descr().type()).c_str());
		}
	} else {
		throw wexception("MO(%u): [actObject]: bad object type %s", serial(),
		                 to_string(obj->descr().type()).c_str());
	}

	++state.ivar1;
	schedule_act(game, Duration(10));
	return true;
}

}  // namespace Widelands
