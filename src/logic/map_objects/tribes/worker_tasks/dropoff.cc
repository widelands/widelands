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

const Bob::Task Worker::taskDropoff = {
   "dropoff", static_cast<Bob::Ptr>(&Worker::dropoff_update), nullptr, nullptr, true};

/**
 * Walk to the building's flag, drop the given ware, and walk back inside.
 */
void Worker::start_task_dropoff(Game& game, WareInstance& ware) {
	set_carried_ware(game, &ware);
	push_task(game, taskDropoff);
}

void Worker::dropoff_update(Game& game, State& /* state */) {
	std::string signal = get_signal();

	if (!signal.empty()) {
		molog(game.get_gametime(), "[dropoff]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	WareInstance* ware = get_carried_ware(game);
	BaseImmovable* const location = game.map()[get_position()].get_immovable();

	// If the building just got destroyed, pop the task
	PlayerImmovable* current_location = get_location(game);
	if (current_location == nullptr) {
		molog(game.get_gametime(),
		      "%s: Unable to dropoff ware in building at (%d,%d) - there is no building there\n",
		      descr().name().c_str(), get_position().x, get_position().y);
		return pop_task(game);
	}

#ifndef NDEBUG
	Building* ploc = dynamic_cast<Building*>(current_location);
	assert(ploc == location || &ploc->base_flag() == location);
#endif

	// Deliver the ware
	if (ware != nullptr) {
		// We're in the building, walk onto the flag
		if (upcast(Building, building, location)) {
			if (start_task_waitforcapacity(game, building->base_flag())) {
				return;
			}

			return start_task_leavebuilding(game, false);  //  exit throttle
		}

		// We're on the flag, drop the ware and pause a little
		if (upcast(Flag, flag, location)) {
			if (flag->has_capacity()) {
				flag->add_ware(game, *fetch_carried_ware(game));

				set_animation(game, descr().get_animation("idle", this));
				return schedule_act(game, Duration(50));
			}

			molog(game.get_gametime(), "[dropoff]: flag is overloaded\n");
			start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
			return;
		}

		throw wexception("MO(%u): [dropoff]: not on building or on flag - fishy", serial());
	}

	// We don't have the ware any more, return home
	if (location->descr().type() == MapObjectType::FLAG) {
		return start_task_move(
		   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
	}

	if (location->descr().type() < MapObjectType::BUILDING) {
		throw wexception("MO(%u): [dropoff]: not on building on return", serial());
	}

	if ((location != nullptr) && location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		schedule_incorporate(game);
		return;
	}

	// Our parent task should know what to do
	return pop_task(game);
}

}  // namespace Widelands
