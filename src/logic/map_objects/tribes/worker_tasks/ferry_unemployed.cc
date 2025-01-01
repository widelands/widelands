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

#include "logic/map_objects/tribes/ferry.h"

#include "economy/flag.h"
#include "logic/map_objects/checkstep.h"

namespace Widelands {

const Bob::Task Ferry::taskUnemployed = {
   "unemployed", static_cast<Bob::Ptr>(&Ferry::unemployed_update), nullptr, nullptr, true};

void Ferry::start_task_unemployed(Game& game) {
	assert(unemployed_since_.get() == 0);
	push_task(game, taskUnemployed);
	unemployed_since_ = game.get_gametime();
}

constexpr Duration kUnemployedLifetime(1000 * 60 * 10);  // 10 minutes

void Ferry::unemployed_update(Game& game, State& /* state */) {
	if (!get_signal().empty()) {
		molog(
		   game.get_gametime(), "[unemployed]: interrupted by signal '%s'\n", get_signal().c_str());
		if (get_signal() == "row") {
			assert(destination_);
			signal_handled();
			unemployed_since_ = Time(0);
			pop_task(game);
			push_task(game, taskRow);
			return schedule_act(game, Duration(10));
		}
	}
	if (destination_) {
		// Sometimes (e.g. when reassigned directly from waterway servicing),
		// the 'row' signal is consumed before we can receive it
		unemployed_since_ = Time(0);
		pop_task(game);
		push_task(game, taskRow);
		return schedule_act(game, Duration(10));
	}

	assert(game.get_gametime() >= unemployed_since_);
	if (game.get_gametime() - unemployed_since_ > kUnemployedLifetime) {
		return schedule_destroy(game);
	}

	const Map& map = game.map();
	const FCoords& pos = get_position();

	if (does_carry_ware()) {
		if (upcast(Flag, flag, pos.field->get_immovable())) {
			// We are on a flag
			if (flag->has_capacity()) {
				molog(game.get_gametime(), "[unemployed]: dropping ware here\n");
				flag->add_ware(game, *fetch_carried_ware(game));
				return start_task_idle(game, descr().get_animation("idle", this), 50);
			}
		}
		molog(game.get_gametime(), "[unemployed]: trying to find a flag\n");
		std::vector<ImmovableFound> flags;
		if (map.find_reachable_immovables(game, Area<FCoords>(pos, 4), &flags, CheckStepFerry(game),
		                                  FindImmovableType(MapObjectType::FLAG)) == 0u) {
			molog(game.get_gametime(), "[unemployed]: no flag found at all\n");
			// Fall through to the selection of a random nearby location
		} else {
			for (ImmovableFound& imm : flags) {
				if (upcast(Flag, flag, imm.object)) {
					if (flag->get_owner() == get_owner()) {
						if (flag->has_capacity()) {
							Path path(pos);
							if (map.findpath(pos, flag->get_position(), 0, path, CheckStepFerry(game)) !=
							    0) {
								molog(game.get_gametime(), "[unemployed]: moving to nearby flag\n");
								return start_task_movepath(
								   game, path, descr().get_right_walk_anims(true, this));
							}
							molog(game.get_gametime(), "[unemployed]: unable to row to reachable flag!\n");
							return start_task_idle(game, descr().get_animation("idle", this), 50);
						}
					}
				}
			}
			molog(game.get_gametime(), "[unemployed]: no nearby flag has capacity\n");
			// If no flag with capacity is nearby, fall through to the selection of a random nearby
			// location
		}
	}

	bool move = true;
	if ((pos.field->nodecaps() & MOVECAPS_SWIM) == 0) {
		molog(game.get_gametime(), "[unemployed]: we are on shore\n");
	} else if (pos.field->get_immovable() != nullptr) {
		molog(game.get_gametime(), "[unemployed]: we are on location\n");
	} else if (pos.field->get_first_bob()->get_next_bob() != nullptr) {
		molog(game.get_gametime(), "[unemployed]: we are on another bob\n");
	} else {
		move = false;
	}

	if (move) {
		// 2 and 5 are arbitrary values that define how far away we'll
		// row at most and how hard we'll try to find a nice new location.
		Path path(pos);
		for (uint8_t i = 0; i < 5; i++) {
			if (map.findpath(pos, game.random_location(pos, 2), 0, path, CheckStepFerry(game)) != 0) {
				return start_task_movepath(
				   game, path, descr().get_right_walk_anims(does_carry_ware(), this));
			}
		}
		molog(game.get_gametime(), "[unemployed]: no suitable locations to row to found\n");
		return start_task_idle(game, descr().get_animation("idle", this), 50);
	}

	return start_task_idle(game, descr().get_animation("idle", this), 500);
}

bool Ferry::unemployed() {
	return (get_state(taskUnemployed) != nullptr) && (destination_ == nullptr);
}

}  // namespace Widelands
