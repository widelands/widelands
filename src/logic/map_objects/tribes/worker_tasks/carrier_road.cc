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
 * Signal "road" on road split.
 * Signal "ware" when a ware has arrived.
 */
Bob::Task const Carrier::taskRoad = {"road", static_cast<Bob::Ptr>(&Carrier::road_update), nullptr,
                                     static_cast<Bob::Ptr>(&Carrier::road_pop), true};

/**
 * Called by road code when the carrier has arrived successfully.
 * Work on the given road, assume the location is correct.
 */
void Carrier::start_task_road(Game& game) {
	push_task(game, taskRoad);

	top_state().ivar1 = 0;

	promised_pickup_to_ = NOONE;
}

/**
 * Called by Road code when the road is split.
 */
void Carrier::update_task_road(Game& game) {
	send_signal(game, "road");
}

void Carrier::road_update(Game& game, State& state) {
	std::string signal = get_signal();

	if (signal == "road" || signal == "ware") {
		// The road changed under us or we're supposed to pick up some ware
		signal_handled();
	} else if (signal == "blocked") {
		// Blocked by an ongoing battle
		signal_handled();
		set_animation(game, descr().get_animation("idle", this));
		return schedule_act(game, Duration(250));
	} else if (!signal.empty()) {
		// Something else happened (probably a location signal)
		molog(game.get_gametime(), "[road]: Terminated by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	// Check for pending wares
	if (promised_pickup_to_ == NOONE) {
		find_pending_ware(game);
	}

	if (promised_pickup_to_ != NOONE) {
		if (state.ivar1 != 0) {
			state.ivar1 = 0;
			return start_task_transport(game, promised_pickup_to_);
		}
		// Short delay before we move to pick up
		state.ivar1 = 1;

		set_animation(game, descr().get_animation("idle", this));
		return schedule_act(game, Duration(50));
	}

	RoadBase& road = dynamic_cast<RoadBase&>(*get_location(game));

	// Move into idle position if necessary
	if (start_task_movepath(game, road.get_path(), road.get_idle_index(),
	                        descr().get_right_walk_anims(does_carry_ware(), this))) {
		return;
	}

	// Be bored. There's nothing good on TV, either.
	// TODO(unknown): idle animations
	set_animation(game, descr().get_animation("idle", this));
	state.ivar1 = 1;  //  we are available immediately after an idle phase
	// subtract maintenance cost and check for road demotion
	if (Road::is_road_descr(&road.descr())) {
		Road& r = dynamic_cast<Road&>(road);
		r.charge_wallet(game);
		// if road still promoted then schedule demotion, otherwise go fully idle waiting until signal
		return r.is_busy() ? schedule_act(game, Duration((r.wallet() + 2) * 500)) : skip_act();
	}
	skip_act();
}

/**
 * We are released, most likely because the road is no longer busy and we are
 * a second carrier (ox or something). If we promised a flag that we would pick up
 * a ware there, we have to make sure that they do not count on us anymore.
 */
void Carrier::road_pop(Game& game, State& /* state */) {
	if (promised_pickup_to_ != NOONE && (get_location(game) != nullptr)) {
		RoadBase& road = dynamic_cast<RoadBase&>(*get_location(game));
		Flag& flag = road.get_flag(static_cast<RoadBase::FlagId>(promised_pickup_to_));
		Flag& otherflag = road.get_flag(static_cast<RoadBase::FlagId>(promised_pickup_to_ ^ 1));

		flag.cancel_pickup(game, otherflag);
	}
}

}  // namespace Widelands
