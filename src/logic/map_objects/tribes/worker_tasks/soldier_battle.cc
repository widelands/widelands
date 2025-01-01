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

#include "logic/map_objects/tribes/soldier.h"

#include <memory>

#include "economy/flag.h"
#include "logic/game_controller.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/player.h"

namespace Widelands {

/**
 * We are out in the open and involved in a challenge/battle.
 * Meet with the other soldier and fight.
 */
Bob::Task const Soldier::taskBattle = {"battle", static_cast<Bob::Ptr>(&Soldier::battle_update),
                                       nullptr, static_cast<Bob::Ptr>(&Soldier::battle_pop), true};

void Soldier::start_task_battle(Game& game) {
	assert(battle_);
	combat_walking_ = CD_NONE;

	push_task(game, taskBattle);
}

void Soldier::battle_update(Game& game, State& /* state */) {
	std::string signal = get_signal();
	molog(game.get_gametime(), "[battle] update for player %u's soldier: signal = \"%s\"\n",
	      owner().player_number(), signal.c_str());

	if (!signal.empty()) {
		if (signal == "blocked") {
			signal_handled();
			return start_task_idle(game, descr().get_animation("idle", this), 5000);
		}
		if (signal == "location" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else {
			molog(game.get_gametime(), "[battle] interrupted by unexpected signal '%s'\n",
			      signal.c_str());
			return pop_task(game);
		}
	}

	// The opponent might have died on us
	if ((battle_ == nullptr) || battle_->opponent(*this) == nullptr) {
		if (combat_walking_ == CD_COMBAT_W) {
			return start_task_move_in_battle(game, CD_RETURN_W);
		}
		if (combat_walking_ == CD_COMBAT_E) {
			return start_task_move_in_battle(game, CD_RETURN_E);
		}
		assert(combat_walking_ == CD_NONE);
		molog(game.get_gametime(), "[battle] is over\n");
		send_space_signals(game);
		return pop_task(game);
	}

	const Map& map = game.map();
	Soldier& opponent = *battle_->opponent(*this);
	if (!owner().is_hostile(opponent.owner()) && !opponent.owner().is_hostile(owner())) {
		/* The players agreed on a truce. */
		molog(game.get_gametime(), "[battle] opponent is an ally, cancel battle");
		combat_walking_ = CD_NONE;
		return pop_task(game);
	}
	if (opponent.get_position() != get_position()) {
		const MapObject* mo = map[get_position()].get_immovable();
		if ((mo != nullptr) && mo->descr().type() >= MapObjectType::BUILDING) {
			// Note that this does not use the "leavebuilding" task,
			// because that task is geared towards orderly workers leaving
			// their location, whereas this case can also happen when
			// a player starts a construction site over a waiting soldier.
			molog(game.get_gametime(), "[battle] we are in a building, leave it\n");
			return start_task_move(
			   game, WALK_SE, descr().get_right_walk_anims(does_carry_ware(), this), true);
		}
	}

	if (stay_home()) {
		if (this == battle_->first()) {
			molog(game.get_gametime(), "[battle] stay_home, so reverse roles\n");
			new Battle(game, battle_->second(), battle_->first());
			return skip_act();  //  we will get a signal via set_battle()
		}
		if (combat_walking_ != CD_COMBAT_E) {
			opponent.send_signal(game, "wakeup");
			return start_task_move_in_battle(game, CD_WALK_E);
		}
	} else {
		if (opponent.stay_home() && (this == battle_->second())) {
			// Wait until correct roles are assigned
			new Battle(game, battle_->second(), battle_->first());
			return schedule_act(game, Duration(10));
		}

		if (opponent.get_position() != get_position()) {
			Coords dest = opponent.get_position();

			if (upcast(Building, building, map[dest].get_immovable())) {
				dest = building->base_flag().get_position();
			}

			uint32_t const dist = map.calc_distance(get_position(), dest);

			if (dist >= 2 || this == battle_->first()) {
				// Only make small steps at a time, so we can adjust to the
				// opponent's change of position.
				if (start_task_movepath(game, dest, 0,
				                        descr().get_right_walk_anims(does_carry_ware(), this), false,
				                        (dist + 3) / 4)) {
					molog(game.get_gametime(),
					      "[battle] player %u's soldier started task_movepath to (%i,%i)\n",
					      owner().player_number(), dest.x, dest.y);
					return;
				}
				BaseImmovable const* const immovable_position = get_position().field->get_immovable();
				BaseImmovable const* const immovable_dest = map[dest].get_immovable();

				const std::string messagetext = format(
				   "The game engine has encountered a logic error. The %s "
				   "#%u of player %u could not find a way from (%i, %i) "
				   "(with %s immovable) to the opponent (%s #%u of player "
				   "%u) at (%i, %i) (with %s immovable). The %s will now "
				   "desert (but will not be executed). Strange things may "
				   "happen. No solution for this problem has been "
				   "implemented yet. (bug #536066) (The game has been "
				   "paused.)",
				   descr().descname(), serial(), static_cast<unsigned int>(owner().player_number()),
				   get_position().x, get_position().y,
				   (immovable_position != nullptr ? immovable_position->descr().descname() : ("no")),
				   opponent.descr().descname(), opponent.serial(),
				   static_cast<unsigned int>(opponent.owner().player_number()), dest.x, dest.y,
				   (immovable_dest != nullptr ? immovable_dest->descr().descname() : ("no")),
				   descr().descname());
				get_owner()->add_message(
				   game, std::unique_ptr<Message>(
				            new Message(Message::Type::kGameLogic, game.get_gametime(),
				                        descr().descname(), "images/ui_basic/menu_help.png",
				                        _("Logic error"), messagetext, get_position(), serial_)));
				opponent.get_owner()->add_message(
				   game, std::unique_ptr<Message>(new Message(
				            Message::Type::kGameLogic, game.get_gametime(), descr().descname(),
				            "images/ui_basic/menu_help.png", _("Logic error"), messagetext,
				            opponent.get_position(), serial_)));
				game.game_controller()->set_desired_speed(0);
				return pop_task(game);
			}
		} else {
			assert(opponent.get_position() == get_position());
			assert(battle_ == opponent.get_battle());

			if (opponent.is_walking()) {
				molog(game.get_gametime(), "[battle]: Opponent '%u' is walking, sleeping\n",
				      opponent.serial());
				// We should be woken up by our opponent, but add a timeout anyway for robustness
				return start_task_idle(game, descr().get_animation("idle", this), 5000);
			}

			if (battle_->first()->serial() == serial()) {
				if (combat_walking_ != CD_COMBAT_W) {
					molog(game.get_gametime(), "[battle]: Moving west\n");
					opponent.send_signal(game, "wakeup");
					return start_task_move_in_battle(game, CD_WALK_W);
				}
			} else {
				if (combat_walking_ != CD_COMBAT_E) {
					molog(game.get_gametime(), "[battle]: Moving east\n");
					opponent.send_signal(game, "wakeup");
					return start_task_move_in_battle(game, CD_WALK_E);
				}
			}
		}
	}

	battle_->get_battle_work(game, *this);
}

void Soldier::battle_pop(Game& game, State& /* state */) {
	if (battle_ != nullptr) {
		battle_->cancel(game, *this);
	}
}

}  // namespace Widelands
