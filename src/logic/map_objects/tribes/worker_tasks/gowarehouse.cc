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

#include "economy/economy.h"

namespace Widelands {

const Bob::Task Worker::taskGowarehouse = {
   "gowarehouse", static_cast<Bob::Ptr>(&Worker::gowarehouse_update),
   static_cast<Bob::PtrSignal>(&Worker::gowarehouse_signalimmediate),
   static_cast<Bob::Ptr>(&Worker::gowarehouse_pop), true};

/**
 * Get the worker to move to the nearest warehouse.
 * The worker is added to the list of usable wares, so he may be reassigned to
 * a new task immediately.
 */
void Worker::start_task_gowarehouse(Game& game) {
	assert(!supply_);

	push_task(game, taskGowarehouse);
}

void Worker::gowarehouse_update(Game& game, State& /* state */) {
	PlayerImmovable* const location = get_location(game);

	if (location == nullptr) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// Signal handling
	std::string signal = get_signal();

	if (!signal.empty()) {
		// if routing has failed, try a different warehouse/route on next update()
		if (signal == "fail" || signal == "cancel") {
			molog(game.get_gametime(), "[gowarehouse]: caught '%s'\n", signal.c_str());
			signal_handled();
		} else if (signal == "transfer") {
			signal_handled();
		} else {
			molog(game.get_gametime(), "[gowarehouse]: cancel for signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	if (location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		delete supply_;
		supply_ = nullptr;

		schedule_incorporate(game);
		return;
	}

	// If we got a transfer, use it
	if (transfer_ != nullptr) {
		Transfer* const t = transfer_;
		transfer_ = nullptr;

		molog(game.get_gametime(), "[gowarehouse]: Got transfer\n");

		pop_task(game);
		return start_task_transfer(game, t);
	}

	// Always leave buildings in an orderly manner,
	// even when no warehouses are left to return to
	if (location->descr().type() >= MapObjectType::BUILDING) {
		return start_task_leavebuilding(game, true);
	}

	if (get_economy(wwWORKER)->warehouses().empty()) {
		molog(game.get_gametime(), "[gowarehouse]: No warehouse left in WorkerEconomy\n");
		return pop_task(game);
	}

	// Idle until we are assigned a transfer.
	// The delay length is rather arbitrary, but we need some kind of
	// check against disappearing warehouses, or the worker will just
	// idle on a flag until the end of days (actually, until either the
	// flag is removed or a warehouse connects to the Economy).
	if (supply_ == nullptr) {
		supply_ = new IdleWorkerSupply(*this);
	}

	return start_task_idle(game, descr().get_animation("idle", this), 1000);
}

void Worker::gowarehouse_signalimmediate(Game& /* game */,
                                         State& /* state */,
                                         const std::string& signal) {
	if (signal == "transfer") {
		// We are assigned a transfer, make sure our supply disappears immediately
		// Otherwise, we might receive two transfers in a row.
		delete supply_;
		supply_ = nullptr;
	}
}

void Worker::gowarehouse_pop(Game& /* game */, State& /* state */) {
	delete supply_;
	supply_ = nullptr;

	if (transfer_ != nullptr) {
		transfer_->has_failed();
		transfer_ = nullptr;
	}
}

}  // namespace Widelands
