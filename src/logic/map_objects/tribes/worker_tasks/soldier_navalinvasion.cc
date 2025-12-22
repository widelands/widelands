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

#include <algorithm>

#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findbob.h"
#include "logic/map_objects/tribes/battle.h"
#include "logic/map_objects/tribes/naval_invasion_base.h"
#include "logic/player.h"

namespace Widelands {

Bob::Task const Soldier::taskNavalInvasion = {
   "naval_invasion", static_cast<Bob::Ptr>(&Soldier::naval_invasion_update), nullptr, nullptr,
   true};

void Soldier::start_task_naval_invasion(Game& game, NavalInvasionBase* camp) {
	push_task(game, taskNavalInvasion);
	State& invasion_state = top_state();

	invasion_state.objvar1 = camp;
	invasion_state.coords = camp->get_position();
	invasion_state.ivar1 = 0;

	// TODO(tothxa): Compatibility with v1.3:
	//    invasion_state.ivar2 and invasion_state.ivar3 were used before v1.4.
	//    Be careful if you want to reuse them before v1.5.
	//    Remove this reminder in v1.5.

	molog(game.get_gametime(), "[naval_invasion] Starting at %3dx%3d\n", invasion_state.coords.x,
	      invasion_state.coords.y);
}

void Soldier::naval_invasion_update(Game& game, State& state) {
	std::string signal = get_signal();
	if (!signal.empty()) {
		signal_handled();
	}

	if (battle_ != nullptr) {
		// We are being attacked
		return start_task_battle(game);
	}

	NavalInvasionBase* camp = dynamic_cast<NavalInvasionBase*>(state.objvar1.get(game));

	// TODO(tothxa): Saveloading compatibility with v1.3.
	//    The invasion base is stored in state.objvar1 only since v1.4, so it may be empty
	//    when an old savegame is loaded. Maybe this can be removed in v1.5...
	if (camp == nullptr) {
		for (Bob* bob = game.map()[state.coords].get_first_bob(); bob != nullptr;
		     bob = bob->get_next_bob()) {
			if (bob->descr().type() == MapObjectType::NAVAL_INVASION_BASE &&
			    bob->get_owner() == get_owner()) {
				camp = dynamic_cast<NavalInvasionBase*>(bob);
				assert(camp != nullptr);
				break;
			}
		}
	}

	if (camp == nullptr) {
		throw(wexception("No naval invasion base for soldier %u at %3dx%3d", serial(), state.coords.x,
		                 state.coords.y));
	}
	assert(camp->owner() == owner());

	/*
	 * Soldiers in a naval invasion are based around `state.objvar1` and seek to eliminate
	 * all enemy influence within the conquer radius of our planned port and the biggest
	 * possible enemy conquer radius. Enemy buildings with influence within this area are
	 * tracked by the invasion base. (this is a performance optimisation, it allows fewer
	 * expensive searches)
	 * When such influence exists, the target building serial is stored in `state.ivar1`.
	 * Otherwise, the soldiers stay at the flag and wait indefinitely until an own port
	 * has been built at their location.
	 */

	if (state.ivar1 != 0) {
		// Heading to enemy building
		Building* enemy = dynamic_cast<Building*>(game.objects().get_object(state.ivar1));
		state.ivar1 = 0;

		if (enemy == nullptr || !owner().is_hostile(enemy->owner()) ||
		    enemy->attack_target() == nullptr || !enemy->attack_target()->can_be_attacked()) {
			molog(game.get_gametime(), "[naval_invasion] Attack target vanished or not hostile\n");
			return schedule_act(game, Duration(10));
		}

		// Normal pathfinding in task attack gives up too early when the enemy building
		// is on the other side of a bay, creating an endless loop of starting attack -
		// cancelling attack. Since the invasion base only keeps track of reachable enemy
		// buildings, we should increase the pathfinding limit for attacks started from
		// invasions.
		start_task_attack(game, *enemy, CF_ALLOW_LONG_WALK);
		return schedule_act(game, Duration(10));
	}

	// No enemy, look for one
	const Map& map = game.map();
	CheckStepWalkOn checkstep(descr().movecaps(), false);
	const FCoords portspace_fcoords = map.get_fcoords(state.coords);

	std::vector<std::pair<Serial, Coords>> enemy_buildings = camp->get_enemy_buildings(game);

	// Consider closest targets first (estimate by air distance to us, not to the port space)
	std::sort(enemy_buildings.begin(), enemy_buildings.end(),
	          [this, &map](const std::pair<Serial, Coords>& a, const std::pair<Serial, Coords>& b) {
		          const uint32_t distance_a = map.calc_distance(get_position(), a.second);
		          const uint32_t distance_b = map.calc_distance(get_position(), b.second);
		          if (distance_a != distance_b) {
			          return distance_a < distance_b;
		          }
		          return a.first < b.first;
	          });

	for (const auto& enemy : enemy_buildings) {
		Building* bld = dynamic_cast<Building*>(game.objects().get_object(enemy.first));
		if (bld == nullptr || !owner().is_hostile(bld->owner()) || bld->attack_target() == nullptr ||
		    !bld->attack_target()->can_be_attacked()) {
			continue;
		}

		// Attack in next cycle
		molog(game.get_gametime(), "[naval_invasion] Attack target selected (%s at %3dx%3d)\n",
		      bld->descr().name().c_str(), enemy.second.x, enemy.second.y);
		state.ivar1 = enemy.first;
		return schedule_act(game, Duration(10));
	}

	// Check if another enemy wants our port space as well
	if (get_battle() == nullptr) {
		std::vector<OPtr<Soldier>> hostile_soldiers = camp->get_enemy_soldiers();
		std::sort(hostile_soldiers.begin(), hostile_soldiers.end(),
		          [this, &game, &map](OPtr<Soldier> a, OPtr<Soldier> b) {
			          const uint32_t distance_a =
			             map.calc_distance(get_position(), a.get(game)->get_position());
			          const uint32_t distance_b =
			             map.calc_distance(get_position(), b.get(game)->get_position());
			          if (distance_a != distance_b) {
				          return distance_a < distance_b;
			          }
			          return a.serial() < b.serial();
		          });
		for (const OPtr<Soldier>& soldier_optr : hostile_soldiers) {
			Soldier* soldier = soldier_optr.get(game);
			if (soldier->can_be_challenged() && soldier->get_battle() == nullptr &&
			    soldier->get_state(taskNavalInvasion) != nullptr) {
				molog(game.get_gametime(),
				      "[naval_invasion] Hostile soldier selected (%s at %3dx%3d)\n",
				      soldier->descr().name().c_str(), soldier->get_position().x,
				      soldier->get_position().y);
				new Battle(game, this, soldier);
				return start_task_battle(game);
			}
		}
	}

	// No targets found, return to our temporary camp and wait there.

	if (portspace_fcoords.field->get_immovable() != nullptr &&
	    portspace_fcoords.field->get_immovable()->descr().type() == MapObjectType::WAREHOUSE &&
	    portspace_fcoords.field->get_immovable()->get_owner() == get_owner()) {
		Warehouse& wh = dynamic_cast<Warehouse&>(*portspace_fcoords.field->get_immovable());
		if (wh.descr().get_isport()) {
			// A port has been built. Our work here is done.
			pop_task(game);
			set_location(&wh);

			if (get_position().field->get_immovable() == &wh) {
				molog(game.get_gametime(), "[naval_invasion] Entering port\n");
				wh.incorporate_worker(game, this);
				return;
			}

			molog(game.get_gametime(), "[naval_invasion] Heading to port\n");
			return start_task_return(game, false);
		}
	}

	// The player doesn't have to build the port exactly where we envision it.
	if (map.find_reachable_immovables(
	       game, Area<FCoords>(get_position(), kPortSpaceGeneralAreaRadius), nullptr, checkstep,
	       FindFlagWithPlayersWarehouse(*get_owner())) > 0) {
		molog(game.get_gametime(), "[naval_invasion] Nearby economy found, join it\n");
		return pop_task(game);
	}

	if (map.calc_distance(get_position(), state.coords) <= kPortSpaceGeneralAreaRadius) {
		// Invasion soldiers heal very slowly.
		if (current_health_ < get_max_health()) {
			constexpr unsigned kStationaryHealPerSecond = 10;
			heal(kStationaryHealPerSecond);
		}

		return start_task_idle(game, descr().get_animation("idle", this), 1000);
	}

	if (start_task_movepath(game, state.coords, 0,
	                        descr().get_right_walk_anims(does_carry_ware(), this), false,
	                        kPortSpaceRadius)) {
		molog(game.get_gametime(), "[naval_invasion] Return to camp\n");
		return;
	}

	// Camp is blocked, try to get as close as we can.
	for (int i = 10; i > 0; --i) {
		Coords coords = game.random_location(state.coords, kPortSpaceGeneralAreaRadius);
		if (start_task_movepath(game, coords, 0,
		                        descr().get_right_walk_anims(does_carry_ware(), this), false,
		                        kPortSpaceRadius)) {
			molog(game.get_gametime(), "[naval_invasion] Approach camp\n");
			return;
		}
	}

	molog(game.get_gametime(), "[naval_invasion] Could not find a way to camp!\n");
	return pop_task(game);  // Become fugitive
}

}  // namespace Widelands
