/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "battle.h"

#include "game.h"
#include "player.h"
#include "wexception.h"
#include "widelands_fileread.h"
#include "widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"

#include "log.h"

#include "upcast.h"

namespace Widelands {

Battle::Descr g_Battle_Descr("battle", "Battle");


Battle::Battle ()
	:
	Map_Object(&g_Battle_Descr),
	m_first(0),
	m_second(0),
	m_readyflags(0),
	m_first_strikes(true)
{}

Battle::Battle(Game & game, Soldier & First, Soldier & Second) :
	Map_Object     (&g_Battle_Descr),
	m_first        (&First),
	m_second       (&Second),
	m_readyflags   (0),
	m_first_strikes(true)
{
	assert(First.get_owner() != Second.get_owner());
	{
		StreamWrite & ss = game.syncstream();
		ss.Unsigned32(0x00e111ba); // appears as ba111e00 in a hexdump
		ss.Unsigned32(First .serial());
		ss.Unsigned32(Second.serial());
	}

	init(game);
}


void Battle::init (Editor_Game_Base & egbase)
{
	Map_Object::init(egbase);

	m_creationtime = egbase.get_gametime();

	if (upcast(Game, game, &egbase)) {
		if (m_first->getBattle())
			m_first->getBattle()->cancel(*game, *m_first);
		m_first->setBattle(*game, this);
		if (m_second->getBattle())
			m_second->getBattle()->cancel(*game, *m_second);
		m_second->setBattle(*game, this);
	}
}


void Battle::cleanup (Editor_Game_Base & egbase)
{
	if (upcast(Game, game, &egbase)) {
		if (m_first) {
			m_first ->setBattle(*game, 0);
			m_first  = 0;
		}
		if (m_second) {
			m_second->setBattle(*game, 0);
			m_second = 0;
		}
	}

	Map_Object::cleanup(egbase);
}


/**
 * Called by one of the soldiers if it has to cancel the battle immediately.
 */
void Battle::cancel(Game & game, Soldier & soldier)
{
	if        (&soldier == m_first)  {
		m_first = 0;
		soldier.setBattle(game, 0);
	} else if (&soldier == m_second) {
		m_second = 0;
		soldier.setBattle(game, 0);
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

Soldier * Battle::opponent(Soldier & soldier)
{
	assert(m_first == &soldier or m_second == &soldier);
	return m_first == &soldier ? m_second : m_first;
}

//  FIXME Couldn't this code be simplified tremendously by doing all scheduling
//  FIXME for one soldier and letting the other sleep until the battle is over?
void Battle::getBattleWork(Game & game, Soldier & soldier)
{
	if (soldier.get_current_hitpoints() < 1) {
		molog("soldier %u has died\n", soldier.serial());
		soldier          . owner().count_casualty();
		opponent(soldier)->owner().count_kill    ();
		soldier.schedule_destroy(game);
		destroy(game);
		return;
	}

	if (!m_first || !m_second)
		return soldier.skip_act();

	// So both soldiers are alive; are we ready to trade the next blow?
	//
	//  This code choses one of 3 codepaths, depending on 2 variables;
	//  m_readyflags and thisflag. m_readyflags is 1 if only the first soldier
	//  is ready, 2 if only the second soldier is ready and 3 if both soldiers
	//  are ready. But ready to do what? Attack? Defend? Rest?
	//
	//  m_readyflags  :  thisflag  :  m_readyflags | thisflag  : codepath
	//             0  :         1  :                        1  :        a
	//             0  :         2  :                        2  :        a
	//             1  :         1  :                        1  :        a
	//             1  :         2  :                        3  :        b
	//             2  :         1  :                        1  :        a
	//             2  :         2  :                        3  :        b
	//             3  :         1  :                        3  :        c
	//             3  :         2  :                        3  :        c
	//
	//  * If the soldier that this function is called for was not ready before:
	//      * he becomes ready hereby, and
	//      * if the other soldier was ready before, it is time to let one hurt
	//        the other.
	//  * If both soldiers are already ready when this function is called, they
	//    both become non-ready and the soldier that this function was called
	//    for sleeps. When he wakes up, this function is called for him again.
	//    He then becomes ready and wakes up his opponent. Then this function is
	//    called for him, he becomes ready and it is time again to let one hurt
	//    the other.

	uint8_t const thisflag = &soldier == m_first ? 1 : 2;

	if ((m_readyflags | thisflag) != 3) {                         //  codepath a
		//  My opponent is not ready to defend. Idle until he wakes me up.
		assert(m_readyflags == 0 or m_readyflags == thisflag);
		m_readyflags |= thisflag; //  FIXME simplify to a plain assignment
		assert(m_readyflags == thisflag);
		return
			soldier.start_task_idle
				(game, soldier.descr().get_animation("idle"), -1);
	} else if (m_readyflags != 3) {                               //  codepath b
		//  Only one of us was ready before and the other becomes ready now.
		//  Time for one of us to hurt the other. Which one is on turn is decided
		//  by calculateTurn.
		assert
			((m_readyflags == 1 and thisflag == 2) or
			 (m_readyflags == 2 and thisflag == 1));
		m_readyflags |= thisflag; //  FIXME simplify to a plain assignment (3)
		assert(m_readyflags == 3);
		calculateTurn(game);
		opponent(soldier)->send_signal(game, "wakeup");
	} else {                                                      //  codepath c
		//  Both of us were already ready. That means that we already fought and
		//  it is time to rest.
		m_readyflags = 0;
	}

	//  common to codepaths b and c
	soldier.start_task_idle(game, soldier.descr().get_animation("idle"), 1000);
}

void Battle::calculateTurn(Game & game)
{
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
		uint32_t const attack =
			attacker->get_min_attack() +
			(game.logic_rand()
			 %
			 (attacker->get_max_attack() - attacker->get_min_attack() - 1));
		defender->damage(attack - (attack * defender->get_defense()) / 100);
	}
}


/*
==============================

Load/Save support

==============================
*/

#define BATTLE_SAVEGAME_VERSION 1

void Battle::Loader::load(FileRead & fr)
{
	Map_Object::Loader::load(fr);

	Battle & battle = get<Battle>();

	battle.m_creationtime = fr.Signed32();
	battle.m_readyflags = fr.Unsigned8();
	battle.m_first_strikes = fr.Unsigned8();

	m_first = fr.Unsigned32();
	m_second = fr.Unsigned32();
}

void Battle::Loader::load_pointers()
{
	Battle & battle = get<Battle>();
	try {
		Map_Object::Loader::load_pointers();
		if (m_first)
			try {
				battle.m_first = &mol().get<Soldier>(m_first);
			} catch (_wexception const & e) {
				throw wexception("soldier 1 (%u): %s", m_first, e.what());
			}
		if (m_second)
			try {
				battle.m_second = &mol().get<Soldier>(m_second);
			} catch (_wexception const & e) {
				throw wexception("soldier 2 (%u): %s", m_second, e.what());
			}
	} catch (_wexception const & e) {
		throw wexception("battle: %s", e.what());
	}
}

void Battle::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver * mos, FileWrite & fw)
{
	fw.Unsigned8(header_Battle);
	fw.Unsigned8(BATTLE_SAVEGAME_VERSION);

	Map_Object::save(egbase, mos, fw);

	fw.Signed32(m_creationtime);
	fw.Unsigned8(m_readyflags);
	fw.Unsigned8(m_first_strikes);

	// And now, the serials of the soldiers !
	fw.Unsigned32(m_first  ? mos->get_object_file_index(*m_first)  : 0);
	fw.Unsigned32(m_second ? mos->get_object_file_index(*m_second) : 0);
}


Map_Object::Loader * Battle::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader * mol, FileRead & fr)
{
	std::auto_ptr<Loader> loader(new Loader);

	try {
		// Header has been peeled away by caller

		uint8_t const version = fr.Unsigned8();
		if (version != BATTLE_SAVEGAME_VERSION)
			throw wexception("unknown/unhandled version %u", version);

		loader->init(egbase, mol, new Battle);
		loader->load(fr);
	} catch (const std::exception & e) {
		throw wexception("Loading Battle: %s", e.what());
	}

	return loader.release();
}

};
