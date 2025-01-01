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

#include "economy/flag.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/player.h"

namespace Widelands {

/**
 * Accept Bob when is a Soldier alive that is attacking the Player.
 */
struct FindBobSoldierAttackingPlayer : public FindBob {
	FindBobSoldierAttackingPlayer(Game& g, Player& p) : player(p), game(g) {
	}

	bool accept(Bob* const bob) const override {
		if (upcast(Soldier, soldier, bob)) {
			return (soldier->get_current_health() != 0u) &&
			       soldier->is_attacking_player(game, player) && soldier->owner().is_hostile(player);
		}
		return false;
	}

	Player& player;
	Game& game;
};

/**
 * Soldiers with this task go out of his buildings. They will
 * try to find an enemy in his lands and go to hunt them down (signaling
 * "battle"). If no enemy was found inside our lands, but an enemy is found
 * outside our lands, then wait until the enemy goes inside or dissapear.
 * If no enemy is found, then return home.
 *
 * Variables used:
 * \li ivar1 used to store \c CombatFlags
 * \li ivar2 when CF_DEFEND_STAYHOME, 1 if it has reached the flag
//           when CF_RETREAT_WHEN_INJURED, the lesser health before fleeing
 */
Bob::Task const Soldier::taskDefense = {"defense", static_cast<Bob::Ptr>(&Soldier::defense_update),
                                        nullptr, static_cast<Bob::Ptr>(&Soldier::defense_pop),
                                        true};

void Soldier::start_task_defense(Game& game, bool stayhome) {
	molog(game.get_gametime(), "[defense] starting\n");
	push_task(game, taskDefense);
	State& state = top_state();

	state.ivar1 = 0;
	state.ivar2 = 0;

	// Here goes 'configuration'
	if (stayhome) {
		state.ivar1 |= CF_DEFEND_STAYHOME;
		set_retreat_health(0);
	} else {
		/* Flag defenders are not allowed to flee, to avoid abuses */
		state.ivar1 |= CF_RETREAT_WHEN_INJURED;
		set_retreat_health(get_max_health() * kRetreatWhenHealthDropsBelowThisPercentage / 100);

		// Soldier must defend even if he starts injured
		// (current health is below retreat treshold)
		if (get_retreat_health() > get_current_health()) {
			set_retreat_health(get_current_health());
		}
	}
	molog(game.get_gametime(), "[defense] retreat health set: %u\n", get_retreat_health());
}

struct SoldierDistance {
	Soldier* s;
	int dist;

	SoldierDistance(Soldier* a, int d) : dist(d) {
		s = a;
	}

	struct Greater {
		bool operator()(const SoldierDistance& a, const SoldierDistance& b) {
			return (a.dist > b.dist);
		}
	};
};

void Soldier::defense_update(Game& game, State& state) {
	std::string signal = get_signal();

	if (!signal.empty()) {
		if (signal == "blocked" || signal == "battle" || signal == "wakeup") {
			signal_handled();
		} else {
			molog(game.get_gametime(), "[defense] cancelled by signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	}

	PlayerImmovable* const location = get_location(game);
	BaseImmovable* const position = game.map()[get_position()].get_immovable();

	/**
	 * Attempt to fix a crash when player bulldozes a building being defended
	 * by soldiers.
	 */
	if (location == nullptr) {
		return pop_task(game);
	}

	Flag& baseflag = location->base_flag();

	if (battle_ != nullptr) {
		return start_task_battle(game);
	}

	if (signal == "blocked") {
		// Wait before we try again. Note that this must come *after*
		// we check for a battle
		// Note that we *should* be woken via send_space_signals,
		// so the timeout is just an additional safety net.
		return start_task_idle(game, descr().get_animation("idle", this), 5000);
	}

	// If we only are defending our home ...
	if ((state.ivar1 & CF_DEFEND_STAYHOME) != 0) {
		if (position == location && state.ivar2 == 1) {
			molog(game.get_gametime(), "[defense] stayhome: returned home\n");
			return pop_task_or_fight(game);
		}

		if (position == &baseflag) {
			state.ivar2 = 1;
			assert(state.ivar2 == 1);

			if (battle_ != nullptr) {
				return start_task_battle(game);
			}

			// Check if any attacker is waiting us to fight
			std::vector<Bob*> soldiers;
			game.map().find_bobs(
			   game, Area<FCoords>(get_position(), 0), &soldiers, FindBobEnemySoldier(get_owner()));

			for (Bob* temp_bob : soldiers) {
				if (upcast(Soldier, temp_soldier, temp_bob)) {
					if (temp_soldier->can_be_challenged()) {
						assert(temp_soldier != nullptr);
						new Battle(game, this, temp_soldier);
						return start_task_battle(game);
					}
				}
			}

			if (state.ivar2 == 1) {
				molog(game.get_gametime(), "[defense] stayhome: return home\n");
				return start_task_return(game, false);
			}
		}

		molog(game.get_gametime(), "[defense] stayhome: leavebuilding\n");
		return start_task_leavebuilding(game, false);
	}

	// We are outside our building, get list of enemy soldiers attacking us
	std::vector<Bob*> soldiers;
	game.map().find_bobs(game, Area<FCoords>(get_position(), 10), &soldiers,
	                     FindBobSoldierAttackingPlayer(game, *get_owner()));

	if (soldiers.empty() || (get_current_health() < get_retreat_health())) {
		if (get_current_health() < get_retreat_health()) {
			assert(state.ivar1 & CF_RETREAT_WHEN_INJURED);
			molog(game.get_gametime(), "[defense] I am heavily injured (%u)!\n", get_current_health());
		} else {
			molog(game.get_gametime(), "[defense] no enemy soldiers found, ending task\n");
		}

		// If no enemy was found, return home
		if (location == nullptr) {
			molog(game.get_gametime(), "[defense] location disappeared during battle\n");
			return pop_task(game);
		}

		// Soldier is inside of building
		if (position == location) {
			molog(game.get_gametime(), "[defense] returned home\n");
			return pop_task_or_fight(game);
		}

		// Soldier is on base flag
		if (position == &baseflag) {
			return start_task_move(
			   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
		}
		molog(game.get_gametime(), "[defense] return home\n");
		if (start_task_movepath(game, baseflag.get_position(),
		                        4,  // use larger persist when returning home
		                        descr().get_right_walk_anims(does_carry_ware(), this))) {
			return;
		}

		molog(game.get_gametime(), "[defense] could not find way home\n");
		return pop_task(game);
	}

	// Go through soldiers
	std::vector<SoldierDistance> targets;
	for (Bob* temp_bob : soldiers) {

		// If enemy is in our land, then go after it!
		if (upcast(Soldier, soldier, temp_bob)) {
			assert(soldier != this);
			const Field& f = game.map().operator[](soldier->get_position());

			//  Check soldier, be sure that we can fight against soldier.
			// Soldiers can not go over enemy land when defending.
			if ((soldier->can_be_challenged()) && (f.get_owned_by() == get_owner()->player_number())) {
				uint32_t thisDist = game.map().calc_distance(get_position(), soldier->get_position());
				targets.emplace_back(soldier, thisDist);
			}
		}
	}

	std::stable_sort(targets.begin(), targets.end(), SoldierDistance::Greater());

	while (!targets.empty()) {
		const SoldierDistance& target = targets.back();

		if (position == location) {
			return start_task_leavebuilding(game, false);
		}

		if (target.dist <= 1) {
			molog(game.get_gametime(), "[defense] starting battle with %u!\n", target.s->serial());
			assert(target.s != nullptr);
			new Battle(game, this, target.s);
			return start_task_battle(game);
		}

		// Move towards soldier
		if (start_task_movepath(game, target.s->get_position(), 3,
		                        descr().get_right_walk_anims(does_carry_ware(), this), false, 1)) {
			molog(game.get_gametime(), "[defense] move towards soldier %u\n", target.s->serial());
			return;
		}
		molog(game.get_gametime(), "[defense] failed to move towards attacking soldier %u\n",
		      target.s->serial());
		targets.pop_back();
	}
	// If the enemy is not in our land, wait
	return start_task_idle(game, descr().get_animation("idle", this), 250);
}

void Soldier::defense_pop(Game& game, State& /* state */) {
	if (battle_ != nullptr) {
		battle_->cancel(game, *this);
	}
}

}  // namespace Widelands
