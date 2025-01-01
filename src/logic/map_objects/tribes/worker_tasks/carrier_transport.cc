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

#include "logic/map_objects/tribes/carrier.h"

#include "economy/flag.h"
#include "economy/road.h"

namespace Widelands {

/**
 * Fetch a ware from a flag, drop it on the other flag.
 * ivar1 is the flag we fetch from, or -1 when we're in the target building.
 *
 * Signal "update" when the road has been split etc.
 */
Bob::Task const Carrier::taskTransport = {
   "transport", static_cast<Bob::Ptr>(&Carrier::transport_update), nullptr, nullptr, true};

/**
 * Begin the transport task.
 */
void Carrier::start_task_transport(Game& game, int32_t const fromflag) {
	push_task(game, taskTransport);
	top_state().ivar1 = fromflag;
}

void Carrier::transport_update(Game& game, State& state) {
	std::string signal = get_signal();

	if (signal == "road") {
		signal_handled();
	} else if (signal == "blocked") {
		// Blocked by an ongoing battle
		signal_handled();
		set_animation(game, descr().get_animation("idle", this));
		return schedule_act(game, Duration(250));
	} else if (!signal.empty()) {
		molog(game.get_gametime(), "[transport]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	if (state.ivar1 == -1) {
		// If we're "in" the target building, special code applies
		deliver_to_building(game, state);
	} else if (!does_carry_ware()) {
		// If we don't carry something, walk to the flag
		pickup_from_flag(game, state);
	} else {
		RoadBase* road = dynamic_cast<RoadBase*>(get_location(game));
		if (road == nullptr) {
			molog(game.get_gametime(), "[transport]: Road was deleted, cancel");
			send_signal(game, "cancel");
			return pop_task(game);
		}
		// If the ware should go to the building attached to our flag, walk
		// directly into said building
		Flag& flag = road->get_flag(static_cast<RoadBase::FlagId>(state.ivar1 ^ 1));

		WareInstance& ware = *get_carried_ware(game);
		assert(ware.get_location(game) == this);

		// A sanity check is necessary, in case the building has been destroyed
		PlayerImmovable* const next = ware.get_next_move_step(game);

		if ((next != nullptr) && next != &flag && &next->base_flag() == &flag &&
		    road->descr().type() == MapObjectType::ROAD) {
			// Pay some coins before entering the building,
			// to compensate for the time to be spent in its street-segment.
			// Ferries cannot enter buildings, so they lave the ware at the flag
			// for the building's worker to fetch it.
			if (upcast(Road, r, road)) {
				r->pay_for_building();
			}
			enter_building(game, state);
		} else if ((flag.has_capacity() || !swap_or_wait(game, state)) &&
		           !start_task_walktoflag(game, state.ivar1 ^ 1)) {
			// If the flag is overloaded we are allowed to drop wares as
			// long as we can pick another up. Otherwise we have to wait.
			// Drop the ware, possible exchanging it with another one
			drop_ware(game, state);
		}
	}
}

}  // namespace Widelands
