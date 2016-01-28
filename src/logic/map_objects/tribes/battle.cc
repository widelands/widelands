/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "base/log.h"
#include "base/macros.h"
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
}

const BattleDescr& Battle::descr() const {
	return g_battle_descr;
}

Battle::Battle ()
	:
	MapObject(&g_battle_descr),
	m_first(nullptr),
	m_second(nullptr),
	m_creationtime(0),
	m_readyflags(0),
	m_damage(0),
	m_first_strikes(true),
	m_last_attack_hits(false)
{}

Battle::Battle(Game & game, Soldier & First, Soldier & Second) :
	MapObject     (&g_battle_descr),
	m_first        (&First),
	m_second       (&Second),
	m_readyflags   (0),
	m_damage       (0),
	m_first_strikes(true)
{
	assert(First.get_owner() != Second.get_owner());
	{
		StreamWrite & ss = game.syncstream();
		ss.unsigned_32(0x00e111ba); // appears as ba111e00 in a hexdump
		ss.unsigned_32(First .serial());
		ss.unsigned_32(Second.serial());
	}

	// Ensures only live soldiers eganges in a battle
	assert(First.get_current_hitpoints() && Second.get_current_hitpoints());

	init(game);
}


void Battle::init (EditorGameBase & egbase)
{
	MapObject::init(egbase);

	m_creationtime = egbase.get_gametime();

	Game& game = dynamic_cast<Game&>(egbase);

	if (Battle* battle = m_first ->get_battle()) {
		battle->cancel(game, *m_first);
	}
	m_first->set_battle(game, this);
	if (Battle* battle = m_second->get_battle()) {
		battle->cancel(game, *m_second);
	}
	m_second->set_battle(game, this);
}


void Battle::cleanup (EditorGameBase & egbase)
{
	if (m_first) {
		m_first ->set_battle(dynamic_cast<Game&>(egbase), nullptr);
		m_first  = nullptr;
	}
	if (m_second) {
		m_second->set_battle(dynamic_cast<Game&>(egbase), nullptr);
		m_second = nullptr;
	}

	MapObject::cleanup(egbase);
}


/**
 * Called by one of the soldiers if it has to cancel the battle immediately.
 */
void Battle::cancel(Game & game, Soldier & soldier)
{
	if (&soldier == m_first)  {
		m_first = nullptr;
		soldier.set_battle(game, nullptr);
	} else if (&soldier == m_second) {
		m_second = nullptr;
		soldier.set_battle(game, nullptr);
	} else
		return;

	schedule_destroy(game);
}


bool Battle::locked(Game & game)
{
	if (!m_first || !m_second)
		return false;
	if (game.get_gametime() - m_creationtime < 1000)
		return true; // don't change battles around willy-nilly
	return m_first->get_position() == m_second->get_position();
}

Soldier * Battle::opponent(Soldier& soldier)
{
	assert(m_first == &soldier || m_second == &soldier);
	Soldier* other_soldier = m_first == &soldier ? m_second : m_first;
	return other_soldier;
}

//  TODO(unknown): Couldn't this code be simplified tremendously by doing all scheduling
//  for one soldier and letting the other sleep until the battle is over?
//  Could be, but we need to be able change the animations of the soldiers
//  easily without unneeded hacks, and this code is not so difficult, only it
//  had some translations errors
void Battle::get_battle_work(Game & game, Soldier & soldier)
{
	// Identify what soldier is calling the routine
	uint8_t const this_soldier_is = &soldier == m_first ? 1 : 2;

	assert(m_first->get_battle() == this || m_second->get_battle() == this);

	//  Created this four 'states' of the battle:
	// *First time entered, one enters :
	//    oneReadyToFight, mark m_readyflags as he is ready to fight
	// *Next time, the opponent enters:
	//    bothReadyToFight, mark m_readyflags as 3 (round fighted)
	// *Next time, the first enters again:
	//    roundFought, reset m_readyflags
	// *Opponent not on field yet, so one enters :
	//    waitingForOpponent, if others are false

	bool const oneReadyToFight  = (m_readyflags == 0);
	bool const roundFought      = (m_readyflags == 3);
	bool const bothReadyToFight = ((this_soldier_is | m_readyflags) == 3) &&
		(!roundFought);
	bool const waitingForOpponent = !(oneReadyToFight || roundFought || bothReadyToFight);
	std::string what_anim;

	// Apply pending damage
	if (m_damage && oneReadyToFight) {
		// Current attacker is last defender, so damage goes to current attacker
		if (m_first_strikes)
			m_first ->damage(m_damage);
		else
			m_second->damage(m_damage);
		m_damage = 0;
	}

	if (soldier.get_current_hitpoints() < 1) {
		molog("[battle] soldier %u lost the battle\n", soldier.serial());
		soldier          . owner().count_casualty();
		opponent(soldier)->owner().count_kill    ();
		soldier.start_task_die(game);
		molog("[battle] waking up winner %d\n", opponent(soldier)->serial());
		opponent(soldier)->send_signal(game, "wakeup");
		return schedule_destroy(game);
	}

	if (!m_first || !m_second)
		return soldier.skip_act();

	// Here is a timeout to prevent battle freezes
	if (waitingForOpponent && (game.get_gametime() - m_creationtime) > 90 * 1000) {
		molog("[battle] soldier %u waiting for opponent %u too long (%5d sec), cancelling battle...\n",
			soldier.serial(),
			opponent(soldier)->serial(),
			(game.get_gametime() - m_creationtime) / 1000);
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
		assert(m_readyflags == 0);
		m_readyflags = this_soldier_is;
		assert(m_readyflags == this_soldier_is);

		what_anim = this_soldier_is == 1 ?
			"evade_success_e" :
			"evade_success_w";
		return
			soldier.start_task_idle
				(game, soldier.descr().get_rand_anim(game, what_anim.c_str()), 10);
	}
	if (bothReadyToFight) {
		//  Our opponent is waiting for us to fight.
		// Time for one of us to hurt the other. Which one is on turn is decided
		// by calculate_round.
		assert
			((m_readyflags == 1 && this_soldier_is == 2) ||
			 (m_readyflags == 2 && this_soldier_is == 1));

		// Both are now ready, mark flags, so our opponent can get new animation
		m_readyflags = 3;
		assert(m_readyflags == 3);

		// Resolve combat turn
		calculate_round(game);

		// Wake up opponent, so he could update his animation
		opponent(soldier)->send_signal(game, "wakeup");
	}

	if (roundFought) {
		//  Both of us were already ready. That means that we already fought and
		//  it is time to wait until both become ready.
		m_readyflags = 0;
	}

	// The function calculate_round inverts value of m_first_strikes, so
	// attacker will be the m_first when m_first_strikes = false and
	// attacker will be m_second when m_first_strikes = true
	molog
		("[battle] (%u) vs (%u) is %d, first strikes %d, last hit %d\n",
		 soldier.serial(),
		 opponent(soldier)->serial(),
		 this_soldier_is,
		 m_first_strikes,
		 m_last_attack_hits);

	if (this_soldier_is == 1) {
		if (m_first_strikes) {
			if (m_last_attack_hits) {
				what_anim = "evade_failure_e";
			}
			else {
				what_anim = "evade_success_e";
			}
		}
		else {
			if (m_last_attack_hits) {
				what_anim = "attack_success_e";
			}
			else {
				what_anim = "attack_failure_e";
			}
		}
	} else {
		if (m_first_strikes) {
			if (m_last_attack_hits) {
				what_anim = "attack_success_w";
			}
			else {
				what_anim = "attack_failure_w";
			}
		}
		else {
			if (m_last_attack_hits) {
				what_anim = "evade_failure_w";
			}
			else {
				what_anim = "evade_success_w";
			}
		}
	}
	molog
		("[battle] Starting animation %s for soldier %d\n",
		 what_anim.c_str(),
		 soldier.serial());
	soldier.start_task_idle
		(game, soldier.descr().get_rand_anim(game, what_anim.c_str()), 1000);
}

void Battle::calculate_round(Game & game)
{
	assert(!m_damage);

	Soldier * attacker;
	Soldier * defender;

	if (m_first_strikes) {
		attacker = m_first;
		defender = m_second;
	} else {
		attacker = m_second;
		defender = m_first;
	}

	m_first_strikes = !m_first_strikes;

	uint32_t const hit = game.logic_rand() % 100;
	if (hit > defender->get_evade()) {
		// Attacker hits!
		m_last_attack_hits = true;

		assert(attacker->get_min_attack() <= attacker->get_max_attack());
		uint32_t const attack =
			attacker->get_min_attack() +
			(game.logic_rand()
			 %
			 (1 + attacker->get_max_attack() - attacker->get_min_attack()));
		m_damage = attack - (attack * defender->get_defense()) / 100;
	}
	else {
		// Defender evaded
		m_last_attack_hits = false;
	}
}


/*
==============================

Load/Save support

==============================
*/

constexpr uint8_t kCurrentPacketVersion = 2;

void Battle::Loader::load(FileRead & fr)
{
	MapObject::Loader::load(fr);

	Battle & battle = get<Battle>();

	battle.m_creationtime  = fr.signed_32();
	battle.m_readyflags    = fr.unsigned_8();
	battle.m_first_strikes = fr.unsigned_8();
	battle.m_damage     = fr.unsigned_32();
	m_first                = fr.unsigned_32();
	m_second               = fr.unsigned_32();
}

void Battle::Loader::load_pointers()
{
	Battle & battle = get<Battle>();
	try {
		MapObject::Loader::load_pointers();
		if (m_first)
			try {
				battle.m_first = &mol().get<Soldier>(m_first);
			} catch (const WException & e) {
				throw wexception("soldier 1 (%u): %s", m_first, e.what());
			}
		if (m_second)
			try {
				battle.m_second = &mol().get<Soldier>(m_second);
			} catch (const WException & e) {
				throw wexception("soldier 2 (%u): %s", m_second, e.what());
			}
	} catch (const WException & e) {
		throw wexception("battle: %s", e.what());
	}
}

void Battle::save
	(EditorGameBase & egbase, MapObjectSaver & mos, FileWrite & fw)
{
	fw.unsigned_8(HeaderBattle);
	fw.unsigned_8(kCurrentPacketVersion);

	MapObject::save(egbase, mos, fw);

	fw.signed_32(m_creationtime);
	fw.unsigned_8(m_readyflags);
	fw.unsigned_8(m_first_strikes);
	fw.unsigned_32(m_damage);

	// And now, the serials of the soldiers !
	fw.unsigned_32(m_first  ? mos.get_object_file_index(*m_first)  : 0);
	fw.unsigned_32(m_second ? mos.get_object_file_index(*m_second) : 0);
}


MapObject::Loader * Battle::load
	(EditorGameBase & egbase, MapObjectLoader & mol, FileRead & fr)
{
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
	} catch (const std::exception & e) {
		throw wexception("Loading Battle: %s", e.what());
	}

	return loader.release();
}

}
