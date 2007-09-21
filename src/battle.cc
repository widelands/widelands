/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "wexception.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"

#include "log.h"

Battle::Descr g_Battle_Descr;


Battle::Battle (): BaseImmovable(g_Battle_Descr),
                   m_first      (0),
                   m_second     (0),
                   m_last_try   (0),
                   m_next_assault(0)
{}


Battle::~Battle ()
{
	if (m_first) log ("Battle : first wasn't removed yet!\n");
	if (m_second) log ("Battle : second wasn't removed yet!\n");
}


void Battle::init (Editor_Game_Base* eg, Soldier* s1, Soldier* s2)
{
	assert (eg);
	assert (s1);
	assert (s2);

	log ("Battle::init\n");
	Map_Object::init(eg);
	m_first = s1;
	m_second = s2;
	if (Game * const game = dynamic_cast<Game *>(eg))
		m_next_assault = schedule_act(game, 1000); // Every round is 1000 ms
}


void Battle::init (Editor_Game_Base* eg)
{
	assert (eg);

	Map_Object::init(eg);

	if (Game * const game = dynamic_cast<Game *>(eg))
		m_next_assault = schedule_act(game, 1000); // Every round is 1000 ms
}


void Battle::soldiers (Soldier* s1, Soldier* s2)
{
	assert (s1);
	assert (s2);
	log ("Battle::init\n");

	m_first = s1;
	m_second = s2;
}


void Battle::cleanup (Editor_Game_Base* eg)
{
	log ("Battle::cleanup\n");
	m_first = 0;
	m_second = 0;
	Map_Object::cleanup(eg);
}


void Battle::act (Game * g, uint32_t)
{
	log ("Battle::act\n");

	Soldier* attacker;
	Soldier* defender;

	attacker = m_second;
	defender = m_first;

	m_last_try = !m_last_try;
	if (m_last_try) {
		attacker = m_first;
		defender = m_second;

	} else {
		attacker = m_second;
		defender = m_first;
	}

	if (attacker->get_current_hitpoints() < 1) {
		attacker->send_signal(g, "die");
		defender->send_signal(g, "won_battle");

		m_first = 0;
		m_second = 0;
		schedule_destroy (g);
		return;
	}

	if (defender->get_current_hitpoints() < 1)
	{
		defender->send_signal(g, "die");
		attacker->send_signal(g, "won_battle");

		m_first = 0;
		m_second = 0;
		schedule_destroy (g);
		return;
	}

	// Put attack animation
	//attacker->start_animation(g, "attack", 1000);
	uint32_t hit = g->logic_rand() % 100;
	log (" hit=%d ", hit);
	//FIXME: correct implementaion
	if (hit > defender->get_evade()) {
		uint32_t attack = attacker->get_min_attack() +
					  (g->logic_rand() % (attacker->get_max_attack() -
										  attacker->get_min_attack() -1));

		uint32_t defend = defender->get_defense();
		defend = (attack * defend) / 100;

		log (" attack(%d)=%d ", attacker->get_serial(), attack);
		log (" defense(%d)=%d ", defender->get_serial(), defend);
		log (" damage=%d\n", attack-defend);

		defender->damage (attack-defend);
		// defender->start_animation(g, "defend", 1000);

	} else {
		log (" evade(%d)=%d\n", defender->get_serial(), defender->get_evade());
		//defender->start_animation(g, "evade", 1000);
	}
	m_next_assault = schedule_act(g, 1000);
}


/*
==============================

Load/Save support

==============================
*/

#define BATTLE_SAVEGAME_VERSION 1

void Battle::Loader::load(FileRead& fr)
{
	BaseImmovable::Loader::load(fr);

	Battle* b = dynamic_cast<Battle*>(get_object());

	b->m_next_assault = fr.Unsigned32();
	b->m_last_try = fr.Unsigned32();

	m_first = fr.Unsigned32();
	m_second = fr.Unsigned32();
}

void Battle::Loader::load_pointers()
{
	BaseImmovable::Loader::load_pointers();

	Battle* b = dynamic_cast<Battle*>(get_object());

	if (m_first) {
		b->m_first = dynamic_cast<Soldier*>(mol().get_object_by_file_index(m_first));
		if (!b->m_first)
			throw wexception
					("Serial %u of first soldier doesn't point to Soldier object",
					 m_first);
	}

	if (m_second) {
		b->m_second = dynamic_cast<Soldier*>(mol().get_object_by_file_index(m_second));
		if (!b->m_first)
			throw wexception
					("Serial %u of second soldier doesn't point to Soldier object",
					 m_second);
	}
}

void Battle::save(Editor_Game_Base* eg, Widelands_Map_Map_Object_Saver* mos, FileWrite& fw)
{
	fw.Unsigned8(header_Battle);
	fw.Unsigned8(BATTLE_SAVEGAME_VERSION);

	BaseImmovable::save(eg, mos, fw);

	// Write time to next assault
	fw.Unsigned32(m_next_assault);

	// Write the last try
	fw.Unsigned32(m_last_try);

	// And now, the serials of the soldiers !
	if (m_first)
		fw.Unsigned32(mos->get_object_file_index(m_first));
	else
		fw.Unsigned32(0);

	if (m_second)
		fw.Unsigned32(mos->get_object_file_index(m_second));
	else
		fw.Unsigned32(0);
}


Map_Object::Loader* Battle::load
		(Editor_Game_Base* eg,
		 Widelands_Map_Map_Object_Loader* mol,
		 FileRead& fr)
{
	Loader* loader = new Loader;

	try {
		// Header has been peeled away by caller

		Uint8 version = fr.Unsigned8();
		if (version != BATTLE_SAVEGAME_VERSION)
			throw wexception("Unknown version %u", version);

		loader->init(eg, mol, new Battle);
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
