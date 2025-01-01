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

#include <memory>

#include "economy/flag.h"
#include "logic/player.h"

namespace Widelands {

/**
 * Return to our owning building.
 * If dropware (ivar1) is true, we'll drop our carried ware (if any) on the
 * building's flag, if possible.
 * Blocks all signals except for "location".
 */
const Bob::Task Worker::taskReturn = {
   "return", static_cast<Bob::Ptr>(&Worker::return_update), nullptr, nullptr, true};

/**
 * Return to our owning building.
 */
void Worker::start_task_return(Game& game, bool const dropware) {
	PlayerImmovable* const location = get_location(game);

	if ((location == nullptr) || (location->descr().type() < MapObjectType::BUILDING &&
	                              location->descr().type() != MapObjectType::FLAG)) {
		throw wexception("MO(%u): start_task_return(): not owned by building or flag", serial());
	}

	push_task(game, taskReturn);
	top_state().ivar1 = dropware ? 1 : 0;
}

void Worker::return_update(Game& game, State& state) {
	std::string signal = get_signal();

	if (signal == "location") {
		molog(game.get_gametime(), "[return]: Interrupted by signal '%s'\n", signal.c_str());
		return pop_task(game);
	}

	signal_handled();

	PlayerImmovable* location = get_location(game);

	if (location == nullptr) {
		// Usually, this should be caught via the "location" signal above.
		// However, in certain cases, e.g. for a soldier during battle,
		// the location may be overwritten by a different signal while
		// walking home.
		molog(game.get_gametime(), "[return]: Our location disappeared from under us\n");
		return pop_task(game);
	}

	if (BaseImmovable* const pos = game.map().get_immovable(get_position())) {
		if (pos == location) {
			set_animation(game, 0);
			return pop_task(game);
		}

		if (upcast(Flag, flag, pos)) {
			// Is this "our" flag?
			if (flag->get_building() == location) {
				if ((state.ivar1 != 0) && flag->has_capacity()) {
					if (WareInstance* const ware = fetch_carried_ware(game)) {
						flag->add_ware(game, *ware);
						set_animation(game, descr().get_animation("idle", this));
						return schedule_act(game, Duration(20));  //  rest a while
					}
				}

				// Don't try to enter building if it is a dismantle site
				// It is no problem for builders since they won't return before
				// dismantling is complete.
				if ((location != nullptr) && location->descr().type() == MapObjectType::DISMANTLESITE) {
					set_location(nullptr);
					return pop_task(game);
				}
				return start_task_move(
				   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
			}
			if (location == flag) {
				return pop_task(game);
			}
		}
	}

	// Determine the building's flag and move to it

	Flag& target_flag = location->descr().type() == MapObjectType::FLAG ?
	                       dynamic_cast<Flag&>(*location) :
	                       dynamic_cast<Building&>(*location).base_flag();
	if (!start_task_movepath(game, target_flag.get_position(), 15,
	                         descr().get_right_walk_anims(does_carry_ware(), this))) {
		molog(game.get_gametime(), "[return]: Failed to return\n");
		const std::string message = format(
		   _("Your %s can’t find a way home and will likely die."), descr().descname().c_str());

		get_owner()->add_message(
		   game,
		   std::unique_ptr<Message>(new Message(Message::Type::kGameLogic, game.get_gametime(),
		                                        _("Worker"), "images/ui_basic/menu_help.png",
		                                        _("Worker got lost!"), message, get_position())),
		   serial_ != 0u);
		set_location(nullptr);
		return pop_task(game);
	}
}

}  // namespace Widelands
