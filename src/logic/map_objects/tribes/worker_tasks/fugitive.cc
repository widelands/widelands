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
#include "economy/flag.h"

namespace Widelands {

/**
 * Run around aimlessly until we find a warehouse.
 */
const Bob::Task Worker::taskFugitive = {
   "fugitive", static_cast<Bob::Ptr>(&Worker::fugitive_update), nullptr, nullptr, true};

void Worker::start_task_fugitive(Game& game) {
	push_task(game, taskFugitive);

	// Fugitives survive for two to four minutes
	top_state().ivar1 = game.get_gametime().get() + 120000 + 200 * (game.logic_rand() % 600);
}

void Worker::fugitive_update(Game& game, State& state) {
	if (!get_signal().empty()) {
		molog(game.get_gametime(), "[fugitive]: interrupted by signal '%s'\n", get_signal().c_str());
		return pop_task(game);
	}

	const Map& map = game.map();
	PlayerImmovable const* location = get_location(game);

	if ((location != nullptr) && location->get_owner() == get_owner()) {
		molog(game.get_gametime(), "[fugitive]: we are on location\n");

		if (location->descr().type() == Widelands::MapObjectType::WAREHOUSE) {
			return schedule_incorporate(game);
		}

		set_location(nullptr);
		location = nullptr;
	}

	// check whether we're on a flag and it's time to return home
	if (upcast(Flag, flag, map[get_position()].get_immovable())) {
		if (flag->get_owner() == get_owner() && !flag->economy(wwWORKER).warehouses().empty()) {
			set_location(flag);
			if (does_carry_ware()) {
				if (flag->has_capacity()) {
					if (WareInstance* const ware = fetch_carried_ware(game)) {
						molog(game.get_gametime(), "[fugitive] is on flag, drop carried ware %s\n",
						      ware->descr().name().c_str());
						flag->add_ware(game, *ware);
					}
				} else {
					molog(game.get_gametime(),
					      "[fugitive] is on flag, which has no capacity for the carried ware!\n");
				}
			}
			return pop_task(game);
		}
	}

	// Try to find a flag connected to a warehouse that we can return to
	//
	// We always have a high probability to see flags within our vision range,
	// but with some luck we see flags that are even further away.
	std::vector<ImmovableFound> flags;
	uint32_t vision = descr().vision_range();
	uint32_t maxdist = 4 * vision;
	if (map.find_immovables(game, Area<FCoords>(map.get_fcoords(get_position()), maxdist), &flags,
	                        FindFlagWithPlayersWarehouse(*get_owner())) != 0u) {
		uint32_t bestdist = 0;
		Flag* best = nullptr;

		molog(game.get_gametime(), "[fugitive]: found a flag connected to warehouse(s)\n");
		for (const ImmovableFound& tmp_flag : flags) {

			Flag& flag = dynamic_cast<Flag&>(*tmp_flag.object);

			uint32_t const dist = map.calc_distance(get_position(), tmp_flag.coords);

			if ((best == nullptr) || bestdist > dist) {
				best = &flag;
				bestdist = dist;
			}
		}

		if ((best != nullptr) && bestdist > vision) {
			uint32_t chance = maxdist - (bestdist - vision);
			if (game.logic_rand() % maxdist >= chance) {
				best = nullptr;
			}
		}

		if (best != nullptr) {
			molog(game.get_gametime(), "[fugitive]: try to move to flag\n");

			// Warehouse could be on a different island, so check for failure
			// Also, move only a few number of steps in the right direction,
			// so that we could theoretically lose the flag again, but also
			// perhaps find a closer flag.
			if (start_task_movepath(game, best->get_position(), 0,
			                        descr().get_right_walk_anims(does_carry_ware(), this), false, 4)) {
				return;
			}
		}
	}

	if ((state.ivar1 < 0) || (Time(state.ivar1) < game.get_gametime())) {  //  time to die?
		molog(game.get_gametime(), "[fugitive]: die\n");
		return schedule_destroy(game);
	}

	molog(game.get_gametime(), "[fugitive]: wander randomly\n");

	if (start_task_movepath(game, game.random_location(get_position(), descr().vision_range()), 4,
	                        descr().get_right_walk_anims(does_carry_ware(), this))) {
		return;
	}

	return start_task_idle(game, descr().get_animation("idle", this), 50);
}

}  // namespace Widelands
