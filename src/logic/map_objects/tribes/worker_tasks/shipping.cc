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

#include "logic/map_objects/tribes/warehouse.h"

namespace Widelands {

/**
 * Sleep while the shipping code in @ref PortDock and @ref Ship handles us.
 */
const Bob::Task Worker::taskShipping = {"shipping", static_cast<Bob::Ptr>(&Worker::shipping_update),
                                        nullptr, static_cast<Bob::Ptr>(&Worker::shipping_pop),
                                        true};

/**
 * Start the shipping task. If pd != nullptr, add us as a shipping item. We
 * could be an expedition worker though, so we will not be a shipping item
 * though.
 *
 * ivar1 = end shipping?
 */
void Worker::start_task_shipping(Game& game, PortDock* pd) {
	push_task(game, taskShipping);
	top_state().ivar1 = 0;
	if (pd != nullptr) {
		pd->add_shippingitem(game, *this);
	}
}

/**
 * Trigger the end of the shipping task.
 *
 * @note the worker must be in a @ref Warehouse location
 */
void Worker::end_shipping(Game& game) {
	set_ship_serial(0);
	if (State* state = get_state(taskShipping); state != nullptr) {
		state->ivar1 = 1;
		send_signal(game, "endshipping");
	}
}

/**
 * Whether we are currently being handled by the shipping code.
 */
bool Worker::is_shipping() const {
	return get_state(taskShipping) != nullptr;
}

void Worker::shipping_pop(Game& game, State& /* state */) {
	// Defense against unorderly cleanup via reset_tasks
	if (get_location(game) == nullptr) {
		set_economy(nullptr, wwWARE);
		set_economy(nullptr, wwWORKER);
	}
}

void Worker::shipping_update(Game& game, State& state) {
	PlayerImmovable* location = get_location(game);

	// Signal handling
	const std::string& signal = get_signal();

	if (!signal.empty()) {
		if (signal == "endshipping") {
			signal_handled();
			if (dynamic_cast<Warehouse*>(location) == nullptr) {
				molog(game.get_gametime(),
				      "shipping_update: received signal 'endshipping' while not in warehouse!\n");
				pop_task(game);
				return;
			}
		}
		if (signal == "transfer" || signal == "wakeup") {
			signal_handled();
		}
	}

	if ((location != nullptr) || (state.ivar1 != 0)) {
		if (upcast(PortDock, pd, location)) {
			pd->update_shippingitem(game, *this);
		} else {
			return pop_task(game);
		}
	}

	start_task_idle(game, 0, -1);
}

}  // namespace Widelands
