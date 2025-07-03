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
 * walk=\<where\>
 *
 * Walk to a previously selected destination. where can be one of:
 * object  walk to a previously found and selected object
 * coords  walk to a previously found and selected field/coordinate
 * <dir>   walk one field in a fixed direction
 *
 * iparam1 = walkXXX
 * iparam2 = direction for walkDir
 */
bool Worker::run_walk(Game& game, State& state, const Action& action) {
	BaseImmovable const* const imm = game.map()[get_position()].get_immovable();
	Coords dest(Coords::null());
	bool forceonlast = false;
	int32_t max_steps = -1;

	// First of all, make sure we're outside
	if (upcast(Building, b, get_location(game))) {
		if (b == imm) {
			start_task_leavebuilding(game, false);
			return true;
		}
	}

	if ((action.iparam1 & Action::walkDir) != 0) {
		start_task_move(
		   game, action.iparam2, descr().get_right_walk_anims(does_carry_ware(), this), false);
		++state.ivar1;  // next instruction
		return true;
	}

	// Determine the coords we need to walk towards
	if ((action.iparam1 & Action::walkObject) != 0) {
		MapObject* obj = state.objvar1.get(game);

		if (obj != nullptr) {
			if (upcast(Bob const, bob, obj)) {
				dest = bob->get_position();
			} else if (upcast(Immovable const, immovable, obj)) {
				dest = immovable->get_position();
			} else if (upcast(Flag, f, obj)) {
				// Special handling for flags: Go there by road using a Transfer
				if (f == imm) {
					// already there – call next program step
					++state.ivar1;
					return false;
				}
				Transfer* t = new Transfer(game, *this);
				t->set_destination(*f);
				start_task_transfer(game, t);
				return true;  // do not advance program yet
			} else {
				throw wexception("MO(%u): [actWalk]: bad object type %s", serial(),
				                 to_string(obj->descr().type()).c_str());
			}

			//  Only take one step, then rethink (object may have moved)
			max_steps = 1;

			forceonlast = true;
		}
	}
	if (!dest.valid() && ((action.iparam1 & Action::walkCoords) != 0)) {
		dest = state.coords;
	}
	if (!dest.valid()) {
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	// If we've already reached our destination, that's cool
	if (get_position() == dest) {
		++state.ivar1;
		return false;  // next instruction
	}

	// Walk towards it
	if (!start_task_movepath(game, dest, 10, descr().get_right_walk_anims(does_carry_ware(), this),
	                         forceonlast, max_steps)) {
		molog(game.get_gametime(), "  could not find path\n");
		send_signal(game, "fail");
		pop_task(game);
		return true;
	}

	return true;
}

}  // namespace Widelands
