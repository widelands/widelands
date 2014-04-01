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

#include "logic/battle.h"

#include "log.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

Battle::Descr g_Battle_Descr("battle", "Battle");


Battle::Battle ()
	:
	Map_Object(&g_Battle_Descr),
	m_first(nullptr),
	m_second(nullptr),
	m_creationtime(0),
	m_readyflags(0),
	m_damage(0),
	m_first_strikes(true),
	m_last_attack_hits(false)
{}

Battle::Battle(Game & game, Soldier & First, Soldier & Second) :
	Map_Object     (&g_Battle_Descr),
	m_first        (&First),
	m_second       (&Second),
	m_readyflags   (0),
	m_damage       (0),
	m_first_strikes(true)
{
	assert(First.get_owner() != Second.get_owner());
	{
		StreamWrite & ss = game.syncstream();
		ss.Unsigned32(0x00e111ba); // appears as ba111e00 in a hexdump
		ss.Unsigned32(First .serial());
		ss.Unsigned32(Second.serial());
	}

	// Ensures only live soldiers eganges in a battle
	assert(First.get_current_hitpoints() and Second.get_current_hitpoints());

	init(game);
}


void Battle::init (Editor_Game_Base & egbase)
{
	Map_Object::init(egbase);

	m_creationtime = egbase.get_gametime();

	if (Battle* battle = m_first ->getBattle())
		battle->cancel(ref_cast<Game, Editor_Game_Base>(egbase), *m_first);
	m_first->setBattle(ref_cast<Game, Editor_Game_Base>(egbase), this);
	if (Battle* battle = m_second->getBattle())
		battle->cancel(ref_cast<Game, Editor_Game_Base>(egbase), *m_second);
	m_second->setBattle(ref_cast<Game, Editor_Game_Base>(egbase), this);
}


void Battle::cleanup (Editor_Game_Base & egbase)
{
	if (m_first) {
		m_first ->setBattle(ref_cast<Game, Editor_Game_Base>(egbase), nullptr);
		m_first  = nullptr;
	}
	if (m_second) {
		m_second->setBattle(ref_cast<Game, Editor_Game_Base>(egbase), nullptr);
		m_second = nullptr;
	}

	Map_Object::cleanup(egbase);
}


/**
 * Called by one of the soldiers if it has to cancel the battle immediately.
 */
void Battle::cancel(Game & game, Soldier & soldier)
{
	if (&soldier == m_first)  {
		m_first = nullptr;
		soldier.setBattle(game, nullptr);
	} else if (&soldier == m_second) {
		m_second = nullptr;
		soldier.setBattle(game, nullptr);
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
	assert(m_first == &soldier or m_second == &soldier);
	Soldier* other_soldier = m_first == &soldier ? m_second : m_first;
	return other_soldier;
}

//  FIXME Couldn't this code be simplified tremendously by doing all scheduling
//  FIXME for one soldier and letting the other sleep until the battle is over?
//  Could be, but we need to be able change the animations of the soldiers
//  easily without unneeded hacks, and this code is not so difficult, only it
//  had some translations errors
void Battle::getBattleWork(Game & game, Soldier & soldier)
{
	// Identify what soldier is calling the routine
	uint8_t const this_soldier_is = &soldier == m_first ? 1 : 2;

	assert(m_first->getBattle() == this or m_second->getBattle() == this);

	//  Created this three 'states' of the battle:
	// *First time entered, one enters :
	//    oneReadyToFight, mark m_readyflags as he is ready to fight
	// *Next time, the opponent enters:
	//    bothReadyToFight, mark m_readyflags as 3 (round fighted)
	// *Next time, the first enters again:
	//    roundFighted, reset m_readyflags
	bool const oneReadyToFight  = (m_readyflags == 0);
	bool const roundFighted     = (m_readyflags == 3);
	bool const bothReadyToFight = ((this_soldier_is | m_readyflags) == 3) and
		(!roundFighted);
	std::string what_anim;

	// Apply pending damage
	if (m_damage and oneReadyToFight) {
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

	if (!m_first or !m_second)
		return soldier.skip_act();

	// So both soldiers are alive; are we ready to trade the next blow?
	//
	//  This code choses one of 3 codepaths:
	//  *oneReadyToFight : This soldier is ready, but his opponent isn't, so
	//                     wait until opponent "wakeup"-us
	//  *bothReadyToFight: Opponent is ready to fight, so calculate this round
	//                     and set flags to 'roundFighted', so the opponent
	//                     will know that it need to set proper animation
	//  *roundFighted    : Opponent has calculated this round, so this soldier
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
		// by calculateRound.
		assert
			((m_readyflags == 1 and this_soldier_is == 2) or
			 (m_readyflags == 2 and this_soldier_is == 1));

		// Both are now ready, mark flags, so our opponent can get new animation
		m_readyflags = 3;
		assert(m_readyflags == 3);

		// Resolve combat turn
		calculateRound(game);

		// Wake up opponent, so he could update his animation
		opponent(soldier)->send_signal(game, "wakeup");
	}

	if (roundFighted) {
		//  Both of us were already ready. That means that we already fought and
		//  it is time to wait until both become ready.
		m_readyflags = 0;
	}

	// The function calculateRound inverts value of m_first_strikes, so
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

void Battle::calculateRound(Game & game)
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

#define BATTLE_SAVEGAME_VERSION 2

void Battle::Loader::load(FileRead & fr, uint8_t const version)
{
	Map_Object::Loader::load(fr);

	Battle & battle = get<Battle>();

	battle.m_creationtime  = fr.Signed32();
	battle.m_readyflags    = fr.Unsigned8();
	battle.m_first_strikes = fr.Unsigned8();

	if (version == BATTLE_SAVEGAME_VERSION)
		battle.m_damage     = fr.Unsigned32();

	m_first                = fr.Unsigned32();
	m_second               = fr.Unsigned32();
}

void Battle::Loader::load_pointers()
{
	Battle & battle = get<Battle>();
	try {
		Map_Object::Loader::load_pointers();
		if (m_first)
			try {
				battle.m_first = &mol().get<Soldier>(m_first);
			} catch (const _wexception & e) {
				throw wexception("soldier 1 (%u): %s", m_first, e.what());
			}
		if (m_second)
			try {
				battle.m_second = &mol().get<Soldier>(m_second);
			} catch (const _wexception & e) {
				throw wexception("soldier 2 (%u): %s", m_second, e.what());
			}
	} catch (const _wexception & e) {
		throw wexception("battle: %s", e.what());
	}
}

void Battle::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Battle);
	fw.Unsigned8(BATTLE_SAVEGAME_VERSION);

	Map_Object::save(egbase, mos, fw);

	fw.Signed32(m_creationtime);
	fw.Unsigned8(m_readyflags);
	fw.Unsigned8(m_first_strikes);
	fw.Unsigned32(m_damage);

	// And now, the serials of the soldiers !
	fw.Unsigned32(m_first  ? mos.get_object_file_index(*m_first)  : 0);
	fw.Unsigned32(m_second ? mos.get_object_file_index(*m_second) : 0);
}


Map_Object::Loader * Battle::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// Header has been peeled away by caller

		uint8_t const version = fr.Unsigned8();
		if (version <= BATTLE_SAVEGAME_VERSION) {
			loader->init(egbase, mol, *new Battle);
			loader->load(fr, version);
		} else
			throw game_data_error("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("Loading Battle: %s", e.what());
	}

	return loader.release();
}

}
