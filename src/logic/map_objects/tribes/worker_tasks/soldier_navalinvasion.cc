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

constexpr int kPortSpaceGeneralAreaRadius = 5;

Bob::Task const Soldier::taskNavalInvasion = {
   "naval_invasion", static_cast<Bob::Ptr>(&Soldier::naval_invasion_update), nullptr, nullptr,
   true};

void Soldier::start_task_naval_invasion(Game& game, const Coords& coords) {
	push_task(game, taskNavalInvasion);
	State& invasion_state = top_state();

	invasion_state.coords = coords;
	invasion_state.ivar1 = 0;
	invasion_state.ivar2 =
	   game.descriptions().get_building_descr(owner().tribe().port())->get_conquers();
	invasion_state.ivar3 = game.descriptions().get_largest_workarea();

	molog(game.get_gametime(), "[naval_invasion] Starting at %3dx%3d\n", coords.x, coords.y);
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

	/*
	 * Soldiers in a naval invasion are based around `state.coords` and seek to eliminate
	 * all enemy influence within a radius of `state.ivar2 + state.ivar3` (where ivar2 is the
	 * conquer radius of our planned port and ivar3 the biggest possible enemy conquer radius).
	 * When such influence is found, the target building serial is stored in `state.ivar1`.
	 * Otherwise, they stay at the flag and wait indefinitely until an own port
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

		start_task_attack(game, *enemy, CF_NONE);
		return schedule_act(game, Duration(10));
	}

	// No enemy, look for one
	const Map& map = game.map();
	CheckStepWalkOn checkstep(descr().movecaps(), false);
	const FCoords portspace_fcoords = map.get_fcoords(state.coords);

	std::vector<ImmovableFound> results;
	map.find_reachable_immovables(game, Area<FCoords>(portspace_fcoords, state.ivar2 + state.ivar3),
	                              &results, checkstep, FindImmovableAttackTarget());
	// Consider closest targets first (estimate by air distance to us, not to the port space)
	std::sort(results.begin(), results.end(),
	          [this, &map](const ImmovableFound& a, const ImmovableFound& b) {
		          const uint32_t distance_a = map.calc_distance(get_position(), a.coords);
		          const uint32_t distance_b = map.calc_distance(get_position(), b.coords);
		          if (distance_a != distance_b) {
			          return distance_a < distance_b;
		          }
		          return a.object->serial() < b.object->serial();
	          });

	for (const ImmovableFound& result : results) {
		Building& bld = dynamic_cast<Building&>(*result.object);
		if (!owner().is_hostile(bld.owner()) || bld.attack_target() == nullptr ||
		    !bld.attack_target()->can_be_attacked() ||
		    bld.descr().get_conquers() + state.ivar2 <
		       map.calc_distance(bld.get_position(), state.coords)) {
			continue;
		}

		// Attack in next cycle
		molog(game.get_gametime(), "[naval_invasion] Attack target selected (%s at %3dx%3d)\n",
		      bld.descr().name().c_str(), bld.get_position().x, bld.get_position().y);
		state.ivar1 = bld.serial();
		return schedule_act(game, Duration(10));
	}

	// Check if another enemy wants our port space as well
	if (get_battle() == nullptr) {
		std::vector<Bob*> hostile_soldiers;
		map.find_reachable_bobs(game, Area<FCoords>(portspace_fcoords, state.ivar2),
		                        &hostile_soldiers, checkstep, FindBobEnemySoldier(get_owner()));
		std::sort(hostile_soldiers.begin(), hostile_soldiers.end(), [this, &map](Bob* a, Bob* b) {
			const uint32_t distance_a = map.calc_distance(get_position(), a->get_position());
			const uint32_t distance_b = map.calc_distance(get_position(), b->get_position());
			if (distance_a != distance_b) {
				return distance_a < distance_b;
			}
			return a->serial() < b->serial();
		});
		for (Bob* bob : hostile_soldiers) {
			upcast(Soldier, soldier, bob);
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
		// The target should be unguarded now, conquer the port space.

		bool has_invasion_base = false;
		for (Bob* bob = map[state.coords].get_first_bob(); bob != nullptr;
		     bob = bob->get_next_bob()) {
			if (bob->descr().type() == MapObjectType::NAVAL_INVASION_BASE &&
			    bob->get_owner() == get_owner()) {
				upcast(NavalInvasionBase, invasion, bob);
				assert(invasion != nullptr);
				has_invasion_base = true;
				invasion->add_soldier(game, this);  // add ourself if not present already
				break;
			}
		}

		if (!has_invasion_base) {
			NavalInvasionBase::create(game, *this, state.coords);
			return start_task_idle(game, descr().get_animation("idle", this), 1000);
		}

		// Stationed soldiers heal very slowly.
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
