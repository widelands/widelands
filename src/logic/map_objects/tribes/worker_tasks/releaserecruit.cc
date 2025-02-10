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

/// Give the recruit his diploma and say farewell to him.

const Bob::Task Worker::taskReleaserecruit = {
   "releaserecruit", static_cast<Bob::Ptr>(&Worker::releaserecruit_update), nullptr, nullptr, true};

void Worker::start_task_releaserecruit(Game& game, const Worker& recruit) {
	push_task(game, taskReleaserecruit);
	molog(game.get_gametime(), "Starting to release %s %u...\n", recruit.descr().name().c_str(),
	      recruit.serial());
	return schedule_act(game, Duration(5000));
}

void Worker::releaserecruit_update(Game& game, State& /* state */) {
	molog(game.get_gametime(), "\t...done releasing recruit\n");
	return pop_task(game);
}

}  // namespace Widelands
