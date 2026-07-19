/*
 * Copyright (C) 2010-2026 by the Widelands Development Team
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

#include "logic/map_objects/tribes/ship/ship.h"

#include <memory>

#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findnode.h"
#include "logic/map_objects/tribes/naval_invasion_base.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"

namespace Widelands {

constexpr uint8_t kPortUnderAttackDefendersSearchRadius = 10;

struct FindBobDefender : public FindBob {
	explicit FindBobDefender(const Player& p) : player_(p) {
	}
	bool accept(Bob* b) const override {
		if (b->descr().type() != MapObjectType::SHIP) {
			return false;
		}
		const Ship& s = dynamic_cast<const Ship&>(*b);
		return s.get_ship_type() == ShipType::kWarship &&
		       (s.owner().player_number() == player_.player_number() ||
		        (player_.team_number() > 0 && s.owner().team_number() == player_.team_number())) &&
		       !s.has_battle();
	}

private:
	const Player& player_;
};

/** Onboard soldiers add a bonus onto the base attack strength, expressed in percent. */
unsigned Ship::get_sea_attack_soldier_bonus(const EditorGameBase& egbase) const {
	unsigned attack_bonus = 0;
	for (const ShippingItem& si : items_) {
		Worker* worker;
		si.get(egbase, nullptr, &worker);
		if (worker != nullptr && worker->descr().type() == MapObjectType::SOLDIER) {
			attack_bonus += dynamic_cast<Soldier*>(worker)->get_total_level() + 1;
		}
	}
	return attack_bonus;
}

void Ship::start_battle(Game& game, Battle new_battle, bool immediately) {
	if (ship_type_ != ShipType::kWarship || state_is_sinking()) {
		molog(game.get_gametime(), "start_battle: not a warship");
		return;
	}

	Ship* enemy_ship = new_battle.opponent.get(game);
	if (enemy_ship == nullptr) {
		if (!new_battle.attack_coords.valid()) {
			molog(game.get_gametime(), "start_battle: no enemy found");
			return;
		}
	} else {
		if (enemy_ship->get_ship_type() != ShipType::kWarship || enemy_ship->state_is_sinking()) {
			molog(game.get_gametime(), "start_battle: enemy is not a warship");
			return;
		}
	}

	if (immediately) {
		battles_.emplace_back(new_battle);
	} else {
		battles_.emplace_front(new_battle);
	}

	send_signal(game, "wakeup");

	if (!new_battle.is_first) {
		return;
	}

	// Summon someone to the defence
	if (enemy_ship != nullptr) {
		enemy_ship->send_message(game, _("Naval Attack"), _("Enemy Ship Attacking"),
		                         format(_("Your ship ‘%s’ is under attack from an enemy warship."),
		                                enemy_ship->get_shipname()),
		                         "images/wui/ship/ship_attack.png");
		enemy_ship->start_battle(game, Battle(this, Coords::null(), {}, false), true);
	}
}

void Ship::battle_update(Game& game) {
	if (state_is_sinking()) {
		battles_.clear();
		return pop_task(game);
	}

	Battle& current_battle = battles_.back();
	Ship* target_ship = current_battle.opponent.get(game);
	if ((target_ship == nullptr || target_ship->state_is_sinking()) &&
	    !current_battle.attack_coords.valid()) {
		molog(game.get_gametime(), "[battle] Enemy disappeared, cancel");
		battles_.pop_back();
		start_task_idle(game, descr().main_animation(), 100);
		return;
	}

	assert((target_ship != nullptr) ^ current_battle.attack_coords.valid());
	assert(target_ship != nullptr || current_battle.is_first);
	const Map& map = game.map();

	Battle* other_battle = nullptr;
	if (target_ship != nullptr) {
		// Find the other ship's current battle
		const size_t nr_enemy_battles = target_ship->battles_.size();
		for (size_t i = nr_enemy_battles; i > 0; --i) {
			Battle* candidate = &target_ship->battles_.at(i - 1);
			if (candidate->opponent.get(game) == this) {
				if (candidate->is_first != current_battle.is_first &&
				    candidate->phase == current_battle.phase) {
					// Found it
					other_battle = candidate;
					break;
				}

				// Same ship but different battle, can happen in case of multiple attacks.
				// The "correct" battle should be further down the stack.
				continue;
			}

			if (candidate->phase != Battle::Phase::kNotYetStarted) {
				molog(game.get_gametime(), "[battle] Enemy engaged in other battle, wait");
				start_task_idle(game, descr().main_animation(), 1000);
				return;
			}
		}

		if (other_battle == nullptr) {
			throw wexception("Warship %s could not find mirror battle against %s",
			                 get_shipname().c_str(), target_ship->get_shipname().c_str());
		}
	}

	auto set_phase = [&game, &current_battle, other_battle](Battle::Phase new_phase) {
		current_battle.phase = new_phase;
		current_battle.time_of_last_action = game.get_gametime();
		if (other_battle != nullptr) {
			other_battle->phase = new_phase;
			other_battle->time_of_last_action = current_battle.time_of_last_action;
		}
	};
	auto fight = [this, &current_battle, other_battle, &game, target_ship]() {
		if (target_ship == nullptr) {
			molog(game.get_gametime(), "[battle] Attacking a port");
			current_battle.pending_damage = 1;                             // Ports always take 1 point
		} else if (game.logic_rand() % 100 < descr().attack_accuracy_) {  // Hit
			uint32_t attack_strength =
			   (game.logic_rand() % (descr().max_attack_ - descr().min_attack_));
			attack_strength += descr().min_attack_;

			attack_strength += attack_strength * get_sea_attack_soldier_bonus(game) / 100;

			molog(game.get_gametime(), "[battle] Hit with %u points", attack_strength);
			current_battle.pending_damage =
			   attack_strength * (100 - target_ship->descr().defense_) / 100;
		} else {  // Miss
			molog(game.get_gametime(), "[battle] Miss");
			current_battle.pending_damage = 0;
		}

		if (other_battle != nullptr) {
			other_battle->pending_damage = current_battle.pending_damage;
		}
	};
	auto damage = [this, &game, set_phase, &current_battle, other_battle,
	               target_ship](Battle::Phase next) {
		assert(target_ship != nullptr);
		if (target_ship->hitpoints_ > current_battle.pending_damage) {
			molog(game.get_gametime(), "[battle] Subtracting %u hitpoints from enemy",
			      current_battle.pending_damage);
			target_ship->hitpoints_ -= current_battle.pending_damage;
			set_phase(next);
		} else {
			molog(game.get_gametime(), "[battle] Enemy defeated");

			get_owner()->count_naval_victory();
			target_ship->get_owner()->count_naval_loss();
			target_ship->send_message(game, _("Ship Sunk"), _("Ship Destroyed"),
			                          _("An enemy ship has destroyed your warship."),
			                          "images/wui/ship/ship_attack.png");

			target_ship->hitpoints_ = 0;
			target_ship->battles_.clear();
			target_ship->reset_tasks(game);
			target_ship->set_ship_state_and_notify(
			   ShipStates::kSinkRequest, NoteShip::Action::kDestinationChanged);

			battles_.pop_back();
			return true;
		}

		current_battle.pending_damage = 0;
		other_battle->pending_damage = 0;
		return false;
	};

	if (!current_battle.is_first) {
		switch (current_battle.phase) {
		case Battle::Phase::kDefenderAttacking: {
			// Our turn is over, now it's the enemy's turn.
			molog(game.get_gametime(), "[battle] Defender's turn ends");
			bool won = damage(Battle::Phase::kAttackersTurn);
			// Make sure we will idle until the enemy ship is truly gone, so we won't attack again
			start_task_idle(
			   game, descr().main_animation(), won ? (kSinkAnimationDuration + 1000) : 100);
			return;
		}

		case Battle::Phase::kDefendersTurn:
			molog(game.get_gametime(), "[battle] Defender's turn begins");
			fight();
			set_phase(Battle::Phase::kDefenderAttacking);
			start_task_idle(game, descr().main_animation(),
			                kAttackAnimationDuration);  // TODO(Nordfriese): proper animation
			return;

		case Battle::Phase::kMovingToBattlePositions: {
			assert(current_battle.battle_position.valid());

			if (get_position() == current_battle.battle_position) {
				molog(game.get_gametime(),
				      "[battle] Defender at battle position, waiting for the attacker to begin");
				start_task_idle(game, descr().main_animation(), 500);
				return;
			}

			if (start_task_movepath(
			       game, current_battle.battle_position, 3, descr().get_sail_anims(), true)) {
				molog(game.get_gametime(), "[battle] Defender moving towards battle position");
				return;
			}

			throw wexception(
			   "Defending ship %s at %dx%d could not find a path to battle position at %dx%d!",
			   get_shipname().c_str(), get_position().x, get_position().y,
			   current_battle.battle_position.x, current_battle.battle_position.y);
		}

		default:
			break;
		}

		// Idle until the enemy tells us it's our turn now.
		molog(game.get_gametime(), "[battle] Defender waiting for turn");
		return start_task_idle(game, descr().main_animation(), 100);
	}

	switch (current_battle.phase) {
	case Battle::Phase::kDefendersTurn:
	case Battle::Phase::kDefenderAttacking:
		// Idle until the opponent's turn is over.
		molog(game.get_gametime(), "[battle] Attacker waiting for turn");
		return start_task_idle(game, descr().main_animation(), 100);

	case Battle::Phase::kNotYetStarted:
		if (target_ship != nullptr) {
			other_battle->battle_position = target_ship->get_position();
			current_battle.battle_position = map.l_n(map.l_n(other_battle->battle_position));

			// Check if this is possible
			CheckStepDefault cstep(MOVECAPS_SWIM);
			Path path;
			bool success = false;
			for (int i = 0; i <= 2; ++i) {
				if (map.findpath(get_position(), current_battle.battle_position, 3, path, cstep) >= 0 &&
				    map.findpath(target_ship->get_position(), other_battle->battle_position, 3, path,
				                 cstep) >= 0) {
					success = true;
					break;
				}

				current_battle.battle_position = map.r_n(current_battle.battle_position);
				other_battle->battle_position = map.r_n(other_battle->battle_position);
			}

			if (!success) {
				throw wexception("Could not find suitable battle position for fight between %s at "
				                 "%dx%d and %s at %dx%d!",
				                 get_shipname().c_str(), get_position().x, get_position().y,
				                 target_ship->get_shipname().c_str(), target_ship->get_position().x,
				                 target_ship->get_position().y);
			}

		} else {
			current_battle.battle_position = current_battle.attack_coords;
		}

		set_phase(Battle::Phase::kMovingToBattlePositions);
		FALLS_THROUGH;
	case Battle::Phase::kMovingToBattlePositions: {
		assert(current_battle.battle_position.valid());

		if (get_position() == current_battle.battle_position) {
			if (target_ship != nullptr &&
			    target_ship->get_position() != other_battle->battle_position) {
				molog(game.get_gametime(), "[battle] Waiting for defender to reach battle position");
				start_task_idle(game, descr().main_animation(), 500);
				return;
			}
			molog(game.get_gametime(), "[battle] Attacker reached battle position, ready to begin");
			set_phase(target_ship != nullptr ? Battle::Phase::kAttackersTurn :
			                                   Battle::Phase::kAttackerAttacking);
			return start_task_idle(game, descr().main_animation(), 100);
		}

		if (start_task_movepath(
		       game, current_battle.battle_position, 3, descr().get_sail_anims(), true)) {
			molog(game.get_gametime(), "[battle] Attacker moving towards battle position");
			return;
		}

		throw wexception(
		   "Attacking ship %s at %dx%d could not find a path to battle position at %dx%d!",
		   get_shipname().c_str(), get_position().x, get_position().y,
		   current_battle.battle_position.x, current_battle.battle_position.y);
	}

	case Battle::Phase::kAttackerAttacking:
		if (target_ship != nullptr) {
			// Our turn is over, now it's the enemy's turn.
			molog(game.get_gametime(), "[battle] Attacker's turn ends");
			bool won = damage(Battle::Phase::kDefendersTurn);
			if (won) {
				return start_task_idle(game, descr().main_animation(), kSinkAnimationDuration + 1000);
			}
		} else if (current_battle.pending_damage > 0) {
			// The naval assault was successful. Now unload the soldiers.
			// From the ship's perspective, the attack was a success.
			molog(game.get_gametime(), "[battle] Naval invasion commencing");

			Coords portspace = map.find_portspace_for_dockpoint(current_battle.attack_coords);
			assert(portspace != Coords::null());
			Field& portspace_field = map[portspace];

			const PlayerNumber enemy_pn = portspace_field.get_owned_by();
			if (enemy_pn != 0) {
				game.get_player(enemy_pn)->add_message_with_timeout(
				   game,
				   std::unique_ptr<Message>(new Message(
				      Message::Type::kSeafaring, game.get_gametime(), _("Naval Attack"),
				      "images/wui/ship/ship_attack.png", _("Enemy Ship Attacking"),
				      _("Your coast is under attack from an enemy warship."), get_position())),
				   Duration(60 * 1000) /* throttle timeout in milliseconds */, 6 /* throttle radius */);
			}

			// We will scatter the soldiers on walkable fields around the portspace.
			// If that's blocked, we try to find a walkable node as closely nearby as possible.

			Coords representative_location = portspace;

			WalkingDir nextdir = WALK_SE;
			int radius = 1;
			int step = -1;
			constexpr uint16_t kTooFar = 3;
			while (radius < kTooFar &&
			       (map[representative_location].nodecaps() & MOVECAPS_WALK) == 0) {
				representative_location = map.get_neighbour(representative_location, nextdir);
				if (step < 0) {
					// start new circle
					nextdir = WALK_W;
					step = 0;
				} else {
					++step;
					if (step >= radius) {
						step = 0;
						nextdir = get_cw_neighbour(nextdir);
						if (nextdir == WALK_W) {
							// finished the circle, we need a bigger one
							nextdir = WALK_SE;
							++radius;
							step = -1;
						}
					}
				}
			}
			if (radius >= kTooFar) {
				// no suitable location found
				// TODO(tothxa): is this possible? if so, implement cancelling attack and sending
				// message
				NEVER_HERE();
			}

			std::vector<Coords> drop_locations;
			const FindNodeInvasion findnode;
			CheckStepDefault checkstep(MOVECAPS_WALK);
			uint32_t nr_fields = map.find_reachable_fields(
			   game, Area<FCoords>(map.get_fcoords(representative_location), 4), &drop_locations,
			   checkstep, findnode);

			if (nr_fields == 0) {
				// no suitable drop locations found
				// TODO(tothxa): is this possible? if so, implement cancelling attack and sending
				// message
				NEVER_HERE();
			}
			assert(nr_fields == drop_locations.size());

			assert(!battles_.back().attack_soldier_serials.empty());

			NavalInvasionBase* camp = nullptr;

			// Let's see whether this is a reinforcement
			{
				std::vector<Bob*> camps;
				// I don't think it's possible to have 2 portspaces within 5 steps that can not be
				// reached from each other by walking, unless blocked by buildings.
				map.find_bobs(game,
				              Area<FCoords>(map.get_fcoords(portspace), kPortSpaceGeneralAreaRadius),
				              &camps, FindBobByType(MapObjectType::NAVAL_INVASION_BASE));
				for (Bob* bob : camps) {
					if (bob->get_owner() == get_owner()) {
						camp = dynamic_cast<NavalInvasionBase*>(bob);
						assert(camp != nullptr);
						break;
					}
				}
			}

			if (camp == nullptr) {
				camp = NavalInvasionBase::create(game, get_owner(), portspace);
			}

			for (Serial serial : battles_.back().attack_soldier_serials) {
				auto it = std::find_if(items_.begin(), items_.end(), [serial](const ShippingItem& si) {
					return si.get_object_serial() == serial;
				});
				if (it == items_.end()) {
					molog(game.get_gametime(), "[battle] Attack soldier %u not on ship", serial);
					continue;
				}

				Worker* worker;
				it->get(game, nullptr, &worker);
				if (worker == nullptr || worker->descr().type() != MapObjectType::SOLDIER) {
					molog(game.get_gametime(), "[battle] Attack soldier %u not a soldier", serial);
					continue;
				}

				it->set_location(game, nullptr);
				it->end_shipping(game);

				worker->set_position(game, drop_locations.at(game.logic_rand() % nr_fields));
				molog(game.get_gametime(), "[battle] Attack soldier %u landed at %dx%d", serial,
				      worker->get_position().x, worker->get_position().y);
				worker->reset_tasks(game);

				Soldier* soldier = dynamic_cast<Soldier*>(worker);
				camp->add_soldier(soldier);
				soldier->start_task_naval_invasion(game, camp);

				items_.erase(it);
			}

			battles_.pop_back();
		} else {
			// Summon someone to the port's defense, if possible.
			std::vector<Bob*> defenders;
			const PlayerNumber enemy_pn = map[current_battle.attack_coords].get_owned_by();
			Player* enemy = enemy_pn == 0 ? nullptr : game.get_player(enemy_pn);
			if (enemy != nullptr && enemy->is_hostile(owner())) {
				map.find_reachable_bobs(
				   game, Area<FCoords>(get_position(), kPortUnderAttackDefendersSearchRadius),
				   &defenders, CheckStepDefault(MOVECAPS_SWIM), FindBobDefender(*enemy));
			}

			Bob* nearest = nullptr;
			uint32_t distance = 0;
			for (Bob* candidate : defenders) {
				const uint32_t d = map.calc_distance(candidate->get_position(), get_position());
				if (nearest == nullptr || d < distance) {
					nearest = candidate;
					distance = d;
				}
			}

			if (nearest != nullptr) {
				// Let the best candidate launch an attack against us. This
				// suspends the current battle until the new fight is over.
				Ship& nearest_ship = dynamic_cast<Ship&>(*nearest);
				molog(game.get_gametime(), "[battle] Summoning %s to the port's defense",
				      nearest_ship.get_shipname().c_str());
				nearest_ship.start_battle(game, Battle(this, Coords::null(), {}, true), true);
			}

			// Since ports can't defend themselves on their own, start the next round at once.
			molog(game.get_gametime(), "[battle] Port is undefended");
			set_phase(Battle::Phase::kAttackersTurn);
		}
		start_task_idle(game, descr().main_animation(), 100);
		return;

	case Battle::Phase::kAttackersTurn:
		molog(game.get_gametime(), "[battle] Attacker's turn begins");
		fight();
		set_phase(Battle::Phase::kAttackerAttacking);
		start_task_idle(game, descr().main_animation(),
		                kAttackAnimationDuration);  // TODO(Nordfriese): proper animation
		return;

	default:
		NEVER_HERE();
	}

	NEVER_HERE();
}

}  // namespace Widelands
