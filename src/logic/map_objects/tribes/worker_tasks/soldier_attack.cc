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
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"

namespace Widelands {

struct FindNodeOwned {
	explicit FindNodeOwned(PlayerNumber owner) : owner_(owner) {
	}
	[[nodiscard]] bool accept(const EditorGameBase& /* egbase */, const FCoords& coords) const {
		return (coords.field->get_owned_by() == owner_);
	}

private:
	PlayerNumber owner_;
};

/**
 * Leave our home building and single-mindedly try to attack
 * and conquer the given building.
 *
 * The following variables are used:
 * \li objvar1 the \ref Building we're attacking.
 */
Bob::Task const Soldier::taskAttack = {"attack", static_cast<Bob::Ptr>(&Soldier::attack_update),
                                       nullptr, static_cast<Bob::Ptr>(&Soldier::attack_pop), true};

void Soldier::start_task_attack(Game& game, Building& building, CombatFlags flags) {
	push_task(game, taskAttack);
	State& state = top_state();
	state.objvar1 = &building;
	state.coords = building.get_position();
	state.ivar1 = flags;
	state.ivar2 = 0;  // The return state 1=go home 2=go back in known land
	state.ivar3 = 0;  // Counts how often the soldier is blocked in a row

	set_retreat_health(kRetreatWhenHealthDropsBelowThisPercentage * get_max_health() / 100);

	// Injured soldiers are not allowed to attack
	if (get_retreat_health() > get_current_health()) {
		set_retreat_health(get_current_health());
	}
	molog(game.get_gametime(), "[attack] starting, retreat health: %u\n", get_retreat_health());
}

void Soldier::attack_update(Game& game, State& state) {
	std::string signal = get_signal();
	uint32_t defenders = 0;

	if (!signal.empty()) {
		if (signal == "battle" || signal == "wakeup" || signal == "sleep") {
			state.ivar3 = 0;
			signal_handled();
		} else if (signal == "blocked") {
			state.ivar3++;
			signal_handled();
		} else if (signal == "fail") {
			state.ivar3 = 0;
			signal_handled();
			if (state.objvar1.get(game) != nullptr) {
				molog(game.get_gametime(), "[attack] failed to reach enemy\n");
				state.objvar1 = nullptr;
			} else {
				molog(game.get_gametime(), "[attack] unexpected fail\n");
				return pop_task(game);
			}
		} else if (signal == "location") {
			molog(game.get_gametime(), "[attack] Location destroyed\n");
			state.ivar3 = 0;
			signal_handled();
			if (state.ivar2 == 0) {
				state.ivar2 = 1;
			}
		} else {
			molog(
			   game.get_gametime(), "[attack] cancelled by unexpected signal '%s'\n", signal.c_str());
			return pop_task(game);
		}
	} else {
		// no signals means no consecutive block -> we're not stuck anymore
		state.ivar3 = 0;
	}

	//  We are at enemy building flag, and a defender is coming, sleep until he
	// "wake up"s me
	if (signal == "sleep") {
		return start_task_idle(game, descr().get_animation("idle", this), -1);
	}

	upcast(Building, location, get_location(game));
	upcast(Building, enemy, state.objvar1.get(game));

	// Handle returns
	const Map& map = game.map();
	if (state.ivar2 > 0) {
		if (state.ivar2 == 1) {
			// Return home
			if ((location == nullptr) || location->descr().type() != MapObjectType::MILITARYSITE) {
				molog(game.get_gametime(), "[attack] No more site to go back to\n");
				state.ivar2 = 2;
				return schedule_act(game, Duration(10));
			}
			Flag& baseflag = location->base_flag();
			if (get_position() == baseflag.get_position()) {
				// At flag, enter building
				return start_task_move(
				   game, WALK_NW, descr().get_right_walk_anims(does_carry_ware(), this), true);
			}
			if (get_position() == location->get_position()) {
				// At building, check if attack is required
				if (enemy == nullptr) {
					molog(game.get_gametime(), "[attack] returned home\n");
					return pop_task_or_fight(game);
				}
				state.ivar2 = 0;
				return start_task_leavebuilding(game, false);
			}
			// Head to home
			if (state.ivar3 > kBockCountIsStuck) {
				molog(
				   game.get_gametime(), "[attack] soldier is stuck, blocked nodes will be ignored\n");
			}

			if (start_task_movepath(game, baseflag.get_position(),
			                        4,  // use larger persist when returning home
			                        descr().get_right_walk_anims(does_carry_ware(), this), false, -1,
			                        state.ivar3 > kBockCountIsStuck)) {
				return;
			}
			molog(game.get_gametime(), "[attack] failed to return home\n");
			return pop_task(game);
		}
		if (state.ivar2 == 2) {
			// No more home, so return to homeland
			upcast(Flag, flag, map.get_immovable(get_position()));
			if ((flag != nullptr) && flag->get_owner() == get_owner()) {
				// At a flag
				molog(game.get_gametime(), "[attack] Returned to own flag\n");
				return pop_task(game);
			}
			Coords target;
			if (get_location(game) != nullptr) {
				// We still have a location, head for the flag
				target = get_location(game)->base_flag().get_position();
				molog(game.get_gametime(), "[attack] Going back to our flag\n");
			} else {
				// No location
				if (get_position().field->get_owned_by() == get_owner()->player_number()) {
					// We are in our land, become fugitive
					molog(game.get_gametime(), "[attack] Back to our land\n");
					return pop_task(game);
				}
				// Try to find our land
				std::vector<Coords> coords;
				uint32_t maxdist = descr().vision_range() * 2;
				Area<FCoords> area(map.get_fcoords(get_position()), maxdist);
				if (map.find_reachable_fields(game, area, &coords, CheckStepDefault(descr().movecaps()),
				                              FindNodeOwned(get_owner()->player_number())) != 0u) {
					// Found home land
					target = coords.front();
					molog(game.get_gametime(), "[attack] Going back to our land\n");
				} else {
					// Become fugitive
					molog(game.get_gametime(), "[attack] No land in sight\n");
					return pop_task(game);
				}
			}
			if (start_task_movepath(game, target,
			                        4,  // use larger persist when returning home
			                        descr().get_right_walk_anims(does_carry_ware(), this))) {
				return;
			}
			molog(game.get_gametime(), "[attack] failed to return to own land\n");
			return pop_task(game);
		}
	}

	if (location != nullptr && get_position() == location->get_position()) {
		// Still at home, need to go outside first
		return start_task_leavebuilding(game, false);
	}

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

	// Count remaining defenders
	if (enemy != nullptr) {
		if (!owner().is_hostile(enemy->owner()) && !enemy->owner().is_hostile(owner())) {
			/* The players agreed on a truce. */
			molog(game.get_gametime(), "[attack] opponent is an ally, cancel attack");
			combat_walking_ = CD_NONE;
			return pop_task(game);
		}

		if (enemy->soldier_control() != nullptr) {
			defenders = enemy->soldier_control()->present_soldiers().size();
		}
		if (upcast(Warehouse, wh, enemy)) {
			Requirements noreq;
			defenders =
			   wh->count_workers(game, wh->owner().tribe().soldier(), noreq, Warehouse::Match::kExact);
		}
		//  Any enemy soldier at baseflag count as defender.
		std::vector<Bob*> soldiers;
		map.find_bobs(game, Area<FCoords>(map.get_fcoords(enemy->base_flag().get_position()), 0),
		              &soldiers, FindBobEnemySoldier(get_owner()));
		defenders += soldiers.size();
	}

	const bool consider_retreat = (state.ivar1 & CF_RETREAT_WHEN_INJURED) != 0 && defenders > 0 &&
	                              get_retreat_health() > get_current_health();
	if (enemy == nullptr || consider_retreat) {
		// Injured soldiers will try to return to safe site at home.
		if (consider_retreat) {
			molog(game.get_gametime(), " [attack] badly injured (%u), retreating...\n",
			      get_current_health());
			state.coords = Coords::null();
			state.objvar1 = nullptr;
		}
		// The old militarysite gets replaced by a new one, so if "enemy" is not
		// valid anymore, we either "conquered" the new building, or it was
		// destroyed.
		if (state.coords.valid()) {
			BaseImmovable* const newimm = map[state.coords].get_immovable();
			upcast(MilitarySite, newsite, newimm);
			if ((newsite != nullptr) && (&newsite->owner() == &owner())) {
				const SoldierControl* soldier_control = newsite->soldier_control();
				assert(soldier_control != nullptr);  // 'newsite' is a military site
				state.objvar1 = nullptr;
				// We may also have our location destroyed in between
				if (soldier_control->stationed_soldiers().size() <
				       soldier_control->soldier_capacity() &&
				    ((location == nullptr) ||
				     location->base_flag().get_position() != newsite->base_flag().get_position())) {
					molog(game.get_gametime(), "[attack] enemy belongs to us now, move in\n");

					// Remain at the conquered site.
					reset_tasks(game);
					start_task_buildingwork(game);

					set_location(newsite);
					newsite->update_soldier_request();
					return schedule_act(game, Duration(10));
				}
			}
		}
		// Return home
		state.ivar2 = 1;
		return schedule_act(game, Duration(10));
	}

	// At this point, we know that the enemy building still stands,
	// and that we're outside in the plains.
	if (get_position() != enemy->base_flag().get_position()) {
		if (start_task_movepath(game, enemy->base_flag().get_position(), 3,
		                        descr().get_right_walk_anims(does_carry_ware(), this))) {
			return;
		}
		molog(game.get_gametime(), "[attack] failed to move towards building flag, cancel attack "
		                           "and return home!\n");
		state.coords = Coords::null();
		state.objvar1 = nullptr;
		state.ivar2 = 1;
		return schedule_act(game, Duration(10));
	}

	assert(enemy->attack_target() != nullptr);

	molog(game.get_gametime(), "[attack] attacking target building\n");
	//  give the enemy soldier some time to act
	schedule_act(game, Duration(enemy->attack_target()->attack(this) ==
	                                  AttackTarget::AttackResult::DefenderLaunched ?
	                               1000 :
	                               10));
}

void Soldier::attack_pop(Game& game, State& /* state */) {
	if (battle_ != nullptr) {
		battle_->cancel(game, *this);
	}
}

}  // namespace Widelands
