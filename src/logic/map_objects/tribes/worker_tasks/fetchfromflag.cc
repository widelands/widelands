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
 * ivar1 is set to 0 if we should move to the flag and fetch the ware, and it
 * is set to 1 if we should move into the building.
 */
const Bob::Task Worker::taskFetchfromflag = {
   "fetchfromflag", static_cast<Bob::Ptr>(&Worker::fetchfromflag_update), nullptr, nullptr, true};

/**
 * Walk to the building's flag, fetch an ware from the flag that is destined for
 * the building, and walk back inside.
 */
void Worker::start_task_fetchfromflag(Game& game) {
	push_task(game, taskFetchfromflag);
	top_state().ivar1 = 0;
}

void Worker::fetchfromflag_update(Game& game, State& state) {
	std::string signal = get_signal();
	if (!signal.empty()) {
		if (signal == "location") {
			molog(game.get_gametime(), "[fetchfromflag]: Building disappeared, become fugitive\n");
			return pop_task(game);
		}
	}

	PlayerImmovable& employer = *get_location(game);
	PlayerImmovable* const location =
	   dynamic_cast<PlayerImmovable*>(game.map().get_immovable(get_position()));

	// If we haven't got the ware yet, walk onto the flag
	if ((get_carried_ware(game) == nullptr) && (state.ivar1 == 0)) {
		if ((location != nullptr) && location->descr().type() >= Widelands::MapObjectType::BUILDING) {
			return start_task_leavebuilding(game, false);
		}

		state.ivar1 = 1;  //  force return to building

		if (location == nullptr) {
			// this can happen if the flag (and the building) is destroyed while
			// the worker leaves the building.
			molog(game.get_gametime(), "[fetchfromflag]: flag disappeared - become fugitive");
			return pop_task(game);
		}

		// The ware has decided that it doesn't want to go to us after all
		// In order to return to the warehouse, we're switching to State_DropOff
		if (WareInstance* const ware =
		       dynamic_cast<Flag&>(*location).fetch_pending_ware(game, employer)) {
			set_carried_ware(game, ware);
		}

		set_animation(game, descr().get_animation("idle", this));
		return schedule_act(game, Duration(20));
	}

	// Go back into the building
	if ((location != nullptr) && location->descr().type() == Widelands::MapObjectType::FLAG) {
		molog(game.get_gametime(), "[fetchfromflag]: return to building\n");

		return start_task_move(
		   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
	}

	if ((location == nullptr) || location->descr().type() < Widelands::MapObjectType::BUILDING) {
		// This can happen "naturally" if the building gets destroyed, but the
		// flag is still there and the worker tries to enter from that flag.
		// E.g. the player destroyed the building, it is destroyed, through an
		// enemy player, or it got destroyed through rising water (Atlantean
		// scenario)
		molog(game.get_gametime(),
		      "[fetchfromflag]: building disappeared - searching for alternative\n");
		return pop_task(game);
	}

	assert(location == &employer);

	molog(game.get_gametime(), "[fetchfromflag]: back home\n");

	if (WareInstance* const ware = fetch_carried_ware(game)) {
		if (ware->get_next_move_step(game) == location) {
			ware->enter_building(game, *dynamic_cast<Building*>(location));
		} else {
			// The ware changed its mind and doesn't want to go to this building
			// after all, so carry it back out.
			// This can happen in the following subtle and rare race condition:
			// We start the fetchfromflag task as the worker in an enhanceable building.
			// While we walk back into the building with the ware, the player enhances
			// the building, so that we now belong to the newly created construction site.
			// Obviously the construction site no longer has any use for the ware.
			molog(game.get_gametime(),
			      "[fetchfromflag]: ware no longer wants to go into building, drop off\n");
			pop_task(game);
			start_task_dropoff(game, *ware);
			return;
		}
	}

	// We're back!
	if (location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		schedule_incorporate(game);
		return;
	}

	return pop_task(game);  //  assume that our parent task knows what to do
}

}  // namespace Widelands
