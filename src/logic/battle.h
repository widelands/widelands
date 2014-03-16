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
#ifndef BATTLE_H
#define BATTLE_H

#include "logic/instances.h"

namespace Widelands {
class Soldier;

/**
 * Manages the battle between two opposing soldiers.
 *
 * A \ref Battle object is created using the \ref create() function as soon as
 * a soldier decides he wants to attack someone else. A battle always has both
 * Soldiers defined, the battle object must be destroyed as soon as there is no
 * other Soldier to battle anymore.
 */
class Battle : public Map_Object {
public:
	typedef Map_Object_Descr Descr;

	Battle(); //  for loading an existing battle from a savegame
	Battle(Game &, Soldier &, Soldier &); //  to create a new battle in the game

	// Implements Map_Object.
	virtual int32_t get_type() const override {return BATTLE;}
	virtual char const * type_name() const override {return "battle";}
	virtual void init(Editor_Game_Base &) override;
	virtual void cleanup(Editor_Game_Base &) override;
	virtual bool has_new_save_support() override {return true;}
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);

	// Cancel this battle immediately and schedule destruction.
	void cancel(Game &, Soldier &);

	// Returns true if the battle should not be interrupted.
	bool locked(Game &);

	// The two soldiers involved in this fight.
	Soldier * first() {return m_first;}
	Soldier * second() {return m_second;}

	// Returns the other soldier involved in this battle. CHECKs that the given
	// soldier is participating in this battle. Can return nullptr, but I have
	// no idea what that means.
	Soldier * opponent(Soldier &);

	// Called by the battling soldiers once they've met on a common node and are
	// idle.
	void getBattleWork(Game &, Soldier &);

private:
	struct Loader : public Map_Object::Loader {
		virtual void load(FileRead &, uint8_t version);
		virtual void load_pointers() override;

		Serial m_first;
		Serial m_second;
	};

	void calculateRound(Game &);

	Soldier * m_first;
	Soldier * m_second;

	/**
	 * Gametime when the battle was created.
	 */
	int32_t m_creationtime;

	/**
	 * 1 if only the first soldier is ready, 2 if only the second soldier
	 * is ready, 3 if both are ready.
	 */
	uint8_t m_readyflags;

	/**
	 * Damage pending to apply. Damage is applied at end of round so animations
	 * can show current action.
	 */
	uint32_t m_damage;

	/**
	 * \c true if the first soldier is the next to strike.
	 */
	bool m_first_strikes;

	/**
	 * \c true if the last turn attacker damaged his opponent
	 */
	bool m_last_attack_hits;
};

}

#endif
