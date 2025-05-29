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

#include "economy/waterway.h"
#include "logic/map_objects/checkstep.h"

namespace Widelands {

const Bob::Task Ferry::taskRow = {
   "row", static_cast<Bob::Ptr>(&Ferry::row_update), nullptr, nullptr, true};

void Ferry::start_task_row(Game& game, const Waterway& ww) {
	// Our new destination is the middle of the waterway
	destination_.reset(
	   new Coords(CoordPath(game.map(), ww.get_path()).get_coords()[ww.get_idle_index()]));
	send_signal(game, "row");
}

void Ferry::row_update(Game& game, State& /* state */) {
	if (!destination_) {
		return pop_task(game);
	}

	const Map& map = game.map();

	const std::string& signal = get_signal();
	if (!signal.empty()) {
		if (signal == "road" || signal == "fail" || signal == "row" || signal == "wakeup") {
			molog(game.get_gametime(), "[row]: Got signal '%s' -> recalculate\n", signal.c_str());
			signal_handled();
		} else if (signal == "blocked") {
			molog(game.get_gametime(), "[row]: Blocked by a battle\n");
			signal_handled();
			return start_task_idle(game, descr().get_animation("idle", this), 900);
		} else {
			molog(game.get_gametime(), "[row]: Cancel due to signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	const FCoords& pos = get_position();

	if (pos == *destination_) {
		// Reached destination
		if (upcast(Waterway, ww, map.get_immovable(*destination_))) {
			destination_.reset(nullptr);
			set_location(ww);
			ww->assign_carrier(*this, 0);
			pop_task(game);
			return start_task_road(game);
		}
		// If we get here, the waterway was destroyed and we didn't notice
		molog(game.get_gametime(),
		      "[row]: Reached the destination (%3dx%3d) but it is no longer there\n",
		      get_position().x, get_position().y);
		destination_.reset(nullptr);
		return pop_task(game);
	}

	Path path(pos);
	if (map.findpath(pos, *destination_, 0, path, CheckStepFerry(game)) == 0) {
		molog(game.get_gametime(),
		      "[row]: Can't find a path to the waterway! Ferry at %3dx%3d, Waterway at %3dx%3d\n",
		      get_position().x, get_position().y, destination_->x, destination_->y);
		// try again later
		return schedule_act(game, Duration(50));
	}
	return start_task_movepath(game, path, descr().get_right_walk_anims(does_carry_ware(), this));
}

}  // namespace Widelands
