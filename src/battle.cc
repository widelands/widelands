/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "log.h"

#include "upcast.h"

namespace Widelands {

Battle::Descr g_Battle_Descr;


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
	{
		StreamWrite & ss = game.syncstream();
		ss.Unsigned32(0x00e111ba); // appears as ba111e00 in a hexdump
		ss.Unsigned32(First .get_serial());
		ss.Unsigned32(Second.get_serial());
	}

	init(&game);
}


void Battle::init (Editor_Game_Base* eg)
{
	Map_Object::init(eg);

	m_creationtime = eg->get_gametime();

	if (upcast(Game, g, eg)) {
		if (m_first->getBattle())
			m_first->getBattle()->cancel(g, m_first);
		m_first->setBattle(g, this);
		if (m_second->getBattle())
			m_second->getBattle()->cancel(g, m_second);
		m_second->setBattle(g, this);
	}
}


void Battle::cleanup (Editor_Game_Base* eg)
{
	if (upcast(Game, g, eg)) {
		if (m_first)
			m_first->setBattle(g, 0);
		if (m_second)
			m_second->setBattle(g, 0);
		m_first = m_second = 0;
	}

	Map_Object::cleanup(eg);
}


/**
 * Called by one of the soldiers if it has to cancel the battle immediately.
 */
void Battle::cancel(Game* g, Soldier* soldier)
{
	if (soldier != m_first && soldier != m_second)
		return;

	if (soldier == m_first) {
		m_first = 0;
		soldier->setBattle(g, 0);
	} else if (soldier == m_second) {
		m_second = 0;
		soldier->setBattle(g, 0);
	}

	schedule_destroy(g);
}


bool Battle::locked(Game* g)
{
	if (!m_first || !m_second)
		return false;
	if (g->get_gametime() - m_creationtime < 1000)
		return true; // don't change battles around willy-nilly
	return m_first->get_position() == m_second->get_position();
}

Soldier * Battle::opponent(Soldier & soldier)
{
	assert(m_first == &soldier or m_second == &soldier);
	return m_first == &soldier ? m_second : m_first;
}

void Battle::getBattleWork(Game & game, Soldier & soldier)
{
	if (soldier.get_current_hitpoints() < 1) {
		molog("soldier %u has died\n", soldier.get_serial());
		soldier          . get_owner()->count_casualty();
		opponent(soldier)->get_owner()->count_kill    ();
		soldier.schedule_destroy(&game);
		destroy(&game);
		return;
	}

	if (!m_first || !m_second)
		return soldier.skip_act();

	// So both soldiers are alive; are we ready to trade the next blow?
	uint8_t const thisflag = &soldier == m_first ? 1 : 2;

	if ((m_readyflags | thisflag) != 3) {
		soldier.start_task_idle(&game, soldier.descr().get_animation("idle"), -1);
		m_readyflags |= thisflag;
		return;
	}
	if (m_readyflags != 3) {
		m_readyflags |= thisflag;
		calculateTurn(game);
		opponent(soldier)->send_signal(&game, "wakeup");
	} else {
		m_readyflags = 0;
	}

	soldier.start_task_idle(&game, soldier.descr().get_animation("idle"), 1000);
}

void Battle::calculateTurn(Game & game)
{
	Soldier* attacker;
	Soldier* defender;

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
		uint32_t attack =
			attacker->get_min_attack() +
			(game.logic_rand()
			 %
			 (attacker->get_max_attack() - attacker->get_min_attack() - 1));

		uint32_t defend = defender->get_defense();
		defend = (attack * defend) / 100;

		defender->damage(attack-defend);
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
	(Editor_Game_Base * egbase, Map_Map_Object_Saver * mos, FileWrite & fw)
{
	fw.Unsigned8(header_Battle);
	fw.Unsigned8(BATTLE_SAVEGAME_VERSION);

	Map_Object::save(egbase, mos, fw);

	fw.Signed32(m_creationtime);
	fw.Unsigned8(m_readyflags);
	fw.Unsigned8(m_first_strikes);

	// And now, the serials of the soldiers !
	fw.Unsigned32(m_first ? mos->get_object_file_index(m_first) : 0);
	fw.Unsigned32(m_second ? mos->get_object_file_index(m_second) : 0);
}


Map_Object::Loader* Battle::load
	(Editor_Game_Base * egbase, Map_Map_Object_Loader * mol, FileRead & fr)
{
	Loader* loader = new Loader;

	try {
		// Header has been peeled away by caller

		uint8_t const version = fr.Unsigned8();
		if (version != BATTLE_SAVEGAME_VERSION)
			throw wexception("unknown/unhandled version %u", version);

		loader->init(egbase, mol, new Battle);
		loader->load(fr);
	} catch (const std::exception & e) {
		delete loader;
		throw wexception("Loading Battle: %s", e.what());
	} catch (...) {
		delete loader;
		throw;
	}

	return loader;
}

};
