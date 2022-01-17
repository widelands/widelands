/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/map_objects/tribes/battle.h"

#include <memory>

#include "base/wexception.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/player.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

namespace {
BattleDescr g_battle_descr("battle", "Battle");
}  // namespace

const BattleDescr& Battle::descr() const {
	return g_battle_descr;
}

Battle::Battle()
   : MapObject(&g_battle_descr),
     first_(nullptr),
     second_(nullptr),
     creationtime_(0),
     readyflags_(0),
     damage_(0),
     first_strikes_(true),
     last_attack_hits_(false) {
}

Battle::Battle(Game& game, Soldier* first_soldier, Soldier* second_soldier)
   : MapObject(&g_battle_descr),
     first_(first_soldier),
     second_(second_soldier),
     creationtime_(0),
     readyflags_(0),
     damage_(0),
     first_strikes_(true),
     last_attack_hits_(false) {
	assert(first_soldier->get_owner() != second_soldier->get_owner());
	{
		StreamWrite& ss = game.syncstream();
		ss.unsigned_8(SyncEntry::kBattle);
		ss.unsigned_32(first_soldier->serial());
		ss.unsigned_32(second_soldier->serial());
	}

	// Ensures only live soldiers engage in a battle
	assert(first_soldier->get_current_health() && second_soldier->get_current_health());

	init(game);
}

bool Battle::init(EditorGameBase& egbase) {
	MapObject::init(egbase);

	creationtime_ = egbase.get_gametime();

	Game& game = dynamic_cast<Game&>(egbase);

	if (Battle* battle = first_->get_battle()) {
		battle->cancel(game, *first_);
	}
	first_->set_battle(game, this);
	if (Battle* battle = second_->get_battle()) {
		battle->cancel(game, *second_);
	}
	second_->set_battle(game, this);
	return true;
}

void Battle::cleanup(EditorGameBase& egbase) {
	if (first_) {
		first_->set_battle(dynamic_cast<Game&>(egbase), nullptr);
		first_ = nullptr;
	}
	if (second_) {
		second_->set_battle(dynamic_cast<Game&>(egbase), nullptr);
		second_ = nullptr;
	}

	MapObject::cleanup(egbase);
}

/**
 * Called by one of the soldiers if it has to cancel the battle immediately.
 */
void Battle::cancel(Game& game, Soldier& soldier) {
	if (&soldier == first_) {
		first_ = nullptr;
		soldier.set_battle(game, nullptr);
	} else if (&soldier == second_) {
		second_ = nullptr;
		soldier.set_battle(game, nullptr);
	} else {
		return;
	}

	schedule_destroy(game);
}

bool Battle::locked(const Game& game) {
	if (!first_ || !second_) {
		return false;
	}
	if (game.get_gametime() - creationtime_ < Duration(1000)) {
		return true;  // don't change battles around willy-nilly
	}
	return first_->get_position() == second_->get_position();
}

Soldier* Battle::opponent(const Soldier& soldier) const {
	assert(first_ == &soldier || second_ == &soldier);
	Soldier* other_soldier = first_ == &soldier ? second_ : first_;
	return other_soldier;
}

unsigned int Battle::get_pending_damage(const Soldier* for_whom) const {
	if (for_whom == (first_strikes_ ? first_ : second_)) {
		return damage_;
	}
	return 0;
}

//  TODO(unknown): Couldn't this code be simplified tremendously by doing all scheduling
//  for one soldier and letting the other sleep until the battle is over?
//  Could be, but we need to be able change the animations of the soldiers
//  easily without unneeded hacks, and this code is not so difficult, only it
//  had some translations errors
void Battle::get_battle_work(Game& game, Soldier& soldier) {
	// Identify what soldier is calling the routine
	uint8_t const this_soldier_is = &soldier == first_ ? 1 : 2;

	assert(first_->get_battle() == this || second_->get_battle() == this);

	//  Created this four 'states' of the battle:
	// *First time entered, one enters :
	//    oneReadyToFight, mark readyflags_ as he is ready to fight
	// *Next time, the opponent enters:
	//    bothReadyToFight, mark readyflags_ as 3 (round fighted)
	// *Next time, the first enters again:
	//    roundFought, reset readyflags_
	// *Opponent not on field yet, so one enters :
	//    waitingForOpponent, if others are false

	bool const oneReadyToFight = (readyflags_ == 0);
	bool const roundFought = (readyflags_ == 3);
	bool const bothReadyToFight = ((this_soldier_is | readyflags_) == 3) && (!roundFought);
	bool const waitingForOpponent = !(oneReadyToFight || roundFought || bothReadyToFight);
	std::string what_anim;

	// Apply pending damage
	if (damage_ && oneReadyToFight) {
		// Current attacker is last defender, so damage goes to current attacker
		if (first_strikes_) {
			first_->damage(damage_);
		} else {
			second_->damage(damage_);
		}
		damage_ = 0;
	}

	if (soldier.get_current_health() < 1) {
		molog(game.get_gametime(), "[battle] soldier %u lost the battle\n", soldier.serial());
		soldier.get_owner()->count_casualty();
		opponent(soldier)->get_owner()->count_kill();
		soldier.start_task_die(game);
		molog(game.get_gametime(), "[battle] waking up winner %d\n", opponent(soldier)->serial());
		opponent(soldier)->send_signal(game, "wakeup");
		return schedule_destroy(game);
	}

	if (!first_ || !second_) {
		return soldier.skip_act();
	}

	// Here is a timeout to prevent battle freezes
	if (waitingForOpponent && (game.get_gametime() - creationtime_) > Duration(90 * 1000)) {
		molog(
		   game.get_gametime(),
		   "[battle] soldier %u waiting for opponent %u too long (%5d sec), cancelling battle...\n",
		   soldier.serial(), opponent(soldier)->serial(),
		   (game.get_gametime() - creationtime_).get() / 1000);
		cancel(game, soldier);
		return;
	}

	// So both soldiers are alive; are we ready to trade the next blow?
	//
	//  This code choses one of 3 codepaths:
	//  *oneReadyToFight : This soldier is ready, but his opponent isn't, so
	//                     wait until opponent "wakeup"-us
	//  *bothReadyToFight: Opponent is ready to fight, so calculate this round
	//                     and set flags to 'roundFought', so the opponent
	//                     will know that it need to set proper animation
	//  *roundFought     : Opponent has calculated this round, so this soldier
	//                     only need to set his proper animation.
	//
	if (oneReadyToFight) {
		//  My opponent is not ready to battle. Idle until he wakes me up.
		assert(readyflags_ == 0);
		readyflags_ = this_soldier_is;
		assert(readyflags_ == this_soldier_is);

		what_anim = this_soldier_is == 1 ? "evade_success_e" : "evade_success_w";
		return soldier.start_task_idle(
		   game, soldier.descr().get_rand_anim(game, what_anim, &soldier), 10);
	}
	if (bothReadyToFight) {
		//  Our opponent is waiting for us to fight.
		// Time for one of us to hurt the other. Which one is on turn is decided
		// by calculate_round.
		assert((readyflags_ == 1 && this_soldier_is == 2) ||
		       (readyflags_ == 2 && this_soldier_is == 1));

		// Both are now ready, mark flags, so our opponent can get new animation
		readyflags_ = 3;
		assert(readyflags_ == 3);

		// Resolve combat turn
		calculate_round(game);

		// Wake up opponent, so he could update his animation
		opponent(soldier)->send_signal(game, "wakeup");
	}

	if (roundFought) {
		//  Both of us were already ready. That means that we already fought and
		//  it is time to wait until both become ready.
		readyflags_ = 0;
	}

	// The function calculate_round inverts value of first_strikes_, so
	// attacker will be the first_ when first_strikes_ = false and
	// attacker will be second_ when first_strikes_ = true
	molog(game.get_gametime(), "[battle] (%u) vs (%u) is %d, first strikes %d, last hit %d\n",
	      soldier.serial(), opponent(soldier)->serial(), this_soldier_is, first_strikes_,
	      last_attack_hits_);

	bool shorten_animation = false;
	if (this_soldier_is == 1) {
		if (first_strikes_) {
			if (last_attack_hits_) {
				what_anim = "evade_failure_e";
				shorten_animation = true;
			} else {
				what_anim = "evade_success_e";
			}
		} else {
			if (last_attack_hits_) {
				what_anim = "attack_success_e";
			} else {
				what_anim = "attack_failure_e";
			}
		}
	} else {
		if (first_strikes_) {
			if (last_attack_hits_) {
				what_anim = "attack_success_w";
			} else {
				what_anim = "attack_failure_w";
			}
		} else {
			if (last_attack_hits_) {
				what_anim = "evade_failure_w";
				shorten_animation = true;
			} else {
				what_anim = "evade_success_w";
			}
		}
	}
	// If the soldier will die as soon as the animation is complete, don't
	// show it for the full length to prevent overlooping (bug 1817664)
	shorten_animation &= damage_ >= soldier.get_current_health();
	molog(game.get_gametime(), "[battle] Starting animation %s for soldier %d\n", what_anim.c_str(),
	      soldier.serial());
	soldier.start_task_idle(game, soldier.descr().get_rand_anim(game, what_anim, &soldier),
	                        shorten_animation ? 850 : 1000);
}

void Battle::calculate_round(Game& game) {
	assert(!damage_);

	Soldier* attacker;
	Soldier* defender;

	if (first_strikes_) {
		attacker = first_;
		defender = second_;
	} else {
		attacker = second_;
		defender = first_;
	}

	first_strikes_ = !first_strikes_;

	uint32_t const hit = game.logic_rand() % 100;
	if (hit >= defender->get_evade()) {
		// Attacker hits!
		last_attack_hits_ = true;

		assert(attacker->get_min_attack() <= attacker->get_max_attack());
		uint32_t const attack =
		   attacker->get_min_attack() +
		   (game.logic_rand() % (1 + attacker->get_max_attack() - attacker->get_min_attack()));
		damage_ = attack - (attack * defender->get_defense()) / 100;
	} else {
		// Defender evaded
		last_attack_hits_ = false;
	}
}

/*
==============================

Load/Save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 2;

void Battle::Loader::load(FileRead& fr) {
	MapObject::Loader::load(fr);

	Battle& battle = get<Battle>();

	battle.creationtime_ = Time(fr);
	battle.readyflags_ = fr.unsigned_8();
	battle.first_strikes_ = fr.unsigned_8();
	battle.damage_ = fr.unsigned_32();
	first_ = fr.unsigned_32();
	second_ = fr.unsigned_32();
}

void Battle::Loader::load_pointers() {
	Battle& battle = get<Battle>();
	try {
		MapObject::Loader::load_pointers();
		if (first_) {
			try {
				battle.first_ = &mol().get<Soldier>(first_);
			} catch (const WException& e) {
				throw wexception("soldier 1 (%u): %s", first_, e.what());
			}
		}
		if (second_) {
			try {
				battle.second_ = &mol().get<Soldier>(second_);
			} catch (const WException& e) {
				throw wexception("soldier 2 (%u): %s", second_, e.what());
			}
		}
	} catch (const WException& e) {
		throw wexception("battle: %s", e.what());
	}
}

void Battle::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderBattle);
	fw.unsigned_8(kCurrentPacketVersion);

	MapObject::save(egbase, mos, fw);

	creationtime_.save(fw);
	fw.unsigned_8(readyflags_);
	fw.unsigned_8(first_strikes_);
	fw.unsigned_32(damage_);

	// And now, the serials of the soldiers !
	fw.unsigned_32(first_ ? mos.get_object_file_index(*first_) : 0);
	fw.unsigned_32(second_ ? mos.get_object_file_index(*second_) : 0);
}

MapObject::Loader* Battle::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// Header has been peeled away by caller

		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			loader->init(egbase, mol, *new Battle);
			loader->load(fr);
		} else {
			throw UnhandledVersionError("Battle", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("Loading Battle: %s", e.what());
	}

	return loader.release();
}
}  // namespace Widelands
