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
#include "logic/map_objects/tribes/market.h"

namespace Widelands {

// The task when a worker is part of the caravan that is trading items.
const Bob::Task Worker::taskCarryTradeItem = {
   "carry_trade_item", static_cast<Bob::Ptr>(&Worker::carry_trade_item_update), nullptr, nullptr,
   true};

void Worker::start_task_carry_trade_item(Game& game,
                                         const int trade_id,
                                         ObjectPointer other_market) {
	push_task(game, taskCarryTradeItem);
	auto& state = top_state();
	state.ivar1 = 0;
	state.ivar2 = trade_id;
	state.objvar1 = other_market;
}

// This is a state machine: leave building, go to the other market, drop off
// wares, and return.
void Worker::carry_trade_item_update(Game& game, State& state) {
	// Reset any signals that are not related to location
	std::string signal = get_signal();
	signal_handled();
	if (!signal.empty()) {
		// TODO(sirver,trading): Remove once signals are correctly handled.
		log_dbg_time(
		   game.get_gametime(), "carry_trade_item_update: signal received: %s\n", signal.c_str());
	}

	// First of all, make sure we're outside
	if (state.ivar1 == 0) {
		start_task_leavebuilding(game, false);
		++state.ivar1;
		return;
	}

	auto* other_market = dynamic_cast<Market*>(state.objvar1.get(game));
	if (state.ivar1 == 1) {
		// Arrived on site. Move to the building and advance our state.
		if (other_market->base_flag().get_position() == get_position()) {
			++state.ivar1;
			return start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
		}

		// Otherwise continue making progress towards the other market.
		if (!start_task_movepath(game, other_market->base_flag().get_position(), 5,
		                         descr().get_right_walk_anims(does_carry_ware(), this))) {
			molog(game.get_gametime(), "carry_trade_item_update: Could not move to other flag.\n");
			// TODO(sirver,trading): something needs to happen here.
		}
		return;
	}

	if (state.ivar1 == 2) {
		WareInstance* const ware = fetch_carried_ware(game);
		other_market->traded_ware_arrived(state.ivar2, ware->descr_index(), &game);
		ware->remove(game);
		++state.ivar1;
		start_task_move(game, WALK_SE, descr().get_right_walk_anims(does_carry_ware(), this), true);
		return;
	}

	if (state.ivar1 == 3) {
		++state.ivar1;
		start_task_return(game, false);
		return;
	}

	if (state.ivar1 == 4) {
		pop_task(game);
		start_task_idle(game, 0, -1);
		dynamic_cast<Market*>(get_location(game))->try_launching_batch(&game);
		return;
	}
	NEVER_HERE();
}

void Worker::update_task_carry_trade_item(Game& game) {
	if (top_state().task == &taskCarryTradeItem) {
		send_signal(game, "update");
	}
}

}  // namespace Widelands
