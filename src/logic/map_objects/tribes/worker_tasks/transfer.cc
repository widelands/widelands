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
#include "economy/road.h"
#include "logic/map_objects/tribes/warehouse.h"

namespace Widelands {

/**
 * Follow the given transfer.
 *
 * Signal "cancel" to cancel the transfer.
 */
const Bob::Task Worker::taskTransfer = {"transfer", static_cast<Bob::Ptr>(&Worker::transfer_update),
                                        nullptr, static_cast<Bob::Ptr>(&Worker::transfer_pop),
                                        false};

/**
 * Tell the worker to follow the Transfer
 */
void Worker::start_task_transfer(Game& game, Transfer* t) {
	// Hackish override for receiving transfers during gowarehouse,
	// and to correctly handle the stack during loading of games
	// (in that case, the transfer task already exists on the stack
	// when this is called).
	if ((get_state(taskGowarehouse) != nullptr) || (get_state(taskTransfer) != nullptr)) {
		assert(!transfer_);

		transfer_ = t;
		send_signal(game, "transfer");
	} else {  //  just start a normal transfer
		push_task(game, taskTransfer);
		transfer_ = t;
	}
}

void Worker::transfer_pop(Game& /* game */, State& /* state */) {
	if (transfer_ != nullptr) {
		transfer_->has_failed();
		transfer_ = nullptr;
	}
}

void Worker::transfer_update(Game& game, State& /* state */) {
	const Map& map = game.map();
	PlayerImmovable* location = get_location(game);

	// We expect to always have a location at this point,
	// but this assumption may fail when loading a corrupted savegame.
	if (location == nullptr) {
		send_signal(game, "location");
		return pop_task(game);
	}

	// The request is no longer valid, the task has failed
	if (transfer_ == nullptr) {
		molog(game.get_gametime(), "[transfer]: Fail (without transfer)\n");

		send_signal(game, "fail");
		return pop_task(game);
	}

	// Signal handling
	const std::string& signal = get_signal();

	if (!signal.empty()) {
		// The caller requested a route update, or the previously calculated route
		// failed.
		// We will recalculate the route on the next update().
		if (signal == "road" || signal == "fail" || signal == "transfer" || signal == "wakeup") {
			molog(game.get_gametime(), "[transfer]: Got signal '%s' -> recalculate\n", signal.c_str());

			signal_handled();
		} else if (signal == "blocked") {
			molog(game.get_gametime(), "[transfer]: Blocked by a battle\n");

			signal_handled();
			return start_task_idle(game, descr().get_animation("idle", this), 500);
		} else {
			molog(game.get_gametime(), "[transfer]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	// If our location is a building, our position may be somewhere else:
	// We may be on the building's flag for a fetch_from_flag or dropoff task.
	// We may also be somewhere else entirely (e.g. lumberjack, soldier).
	// Similarly for flags.
	if (upcast(Building, building, location)) {
		if (building->get_position() != get_position()) {
			return start_task_leavebuilding(game, true);
		}
	} else if (upcast(Flag, flag, location)) {
		BaseImmovable* const position = map[get_position()].get_immovable();

		if (position != flag) {
			if (position == flag->get_building()) {
				location = flag->get_building();
				set_location(location);
			} else {
				return set_location(nullptr);
			}
		}
	}

	// Figure out where to go
	bool success;
	PlayerImmovable* const nextstep = transfer_->get_next_step(location, success);

	if (nextstep == nullptr) {
		Transfer* const t = transfer_;

		transfer_ = nullptr;

		if (success) {
			pop_task(game);

			t->has_finished();
		} else {
			send_signal(game, "fail");
			pop_task(game);

			t->has_failed();
		}
		return;
	}

	// Initiate the next step
	if (upcast(Building, building, location)) {
		if (&building->base_flag() != nextstep) {
			if (upcast(Warehouse, warehouse, building)) {
				if (warehouse->get_portdock() == nextstep) {
					return start_task_shipping(game, warehouse->get_portdock());
				}
			}

			throw wexception(
			   "MO(%u): [transfer]: in building, nextstep is not building's flag", serial());
		}

		return start_task_leavebuilding(game, true);
	}
	if (upcast(Flag, flag, location)) {
		if (upcast(Building, nextbuild, nextstep)) {  //  Flag to Building
			if (&nextbuild->base_flag() != location) {
				throw wexception(
				   "MO(%u): [transfer]: next step is building, but we are nowhere near", serial());
			}

			return start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
		}
		if (upcast(Flag, nextflag, nextstep)) {  //  Flag to Flag
			Road& road = *flag->get_road(*nextflag);

			Path path(road.get_path());

			if (nextstep != &road.get_flag(RoadBase::FlagEnd)) {
				path.reverse();
			}

			molog(game.get_gametime(),
			      "[transfer]: starting task [movepath] and setting location to road %u\n",
			      road.serial());
			start_task_movepath(game, path, descr().get_right_walk_anims(does_carry_ware(), this));
			set_location(&road);
		} else if (upcast(RoadBase, road, nextstep)) {  //  Flag to Road
			if (&road->get_flag(RoadBase::FlagStart) != location &&
			    &road->get_flag(RoadBase::FlagEnd) != location) {
				throw wexception(
				   "MO(%u): [transfer]: nextstep is road, but we are nowhere near", serial());
			}
			molog(game.get_gametime(), "[transfer]: set location to road %u\n", road->serial());
			set_location(road);
			set_animation(game, descr().get_animation("idle", this));
			schedule_act(game, Duration(10));  //  wait a little
		} else {
			throw wexception("MO(%u): [transfer]: flag to bad nextstep %s %u", serial(),
			                 nextstep->descr().name().c_str(), nextstep->serial());
		}
	} else if (upcast(RoadBase, road, location)) {
		// Road to Flag
		if (nextstep->descr().type() == MapObjectType::FLAG) {
			const Path& path = road->get_path();
			int32_t const index = nextstep == &road->get_flag(RoadBase::FlagStart) ? 0 :
			                      nextstep == &road->get_flag(RoadBase::FlagEnd) ? path.get_nsteps() :
			                                                                       -1;

			if (index >= 0) {
				if (start_task_movepath(
				       game, path, index, descr().get_right_walk_anims(does_carry_ware(), this))) {
					molog(game.get_gametime(), "[transfer]: from road %u to flag %u\n", road->serial(),
					      nextstep->serial());
					return;
				}
			} else if (nextstep != map[get_position()].get_immovable()) {
				throw wexception(
				   "MO(%u): [transfer]: road to flag, but flag is nowhere near", serial());
			}

			set_location(dynamic_cast<Flag*>(nextstep));
			set_animation(game, descr().get_animation("idle", this));
			schedule_act(game, Duration(10));  //  wait a little
		} else {
			throw wexception(
			   "MO(%u): [transfer]: from road to bad nextstep %u", serial(), nextstep->serial());
		}
	} else {
		// Check location to make clang-tidy happy
		throw wexception(
		   "MO(%u): location %u has bad type", serial(), location ? location->serial() : 0);
	}
}

/**
 * Called by transport code when the transfer has been cancelled & destroyed.
 */
void Worker::cancel_task_transfer(Game& game) {
	transfer_ = nullptr;
	send_signal(game, "cancel");
}

}  // namespace Widelands
