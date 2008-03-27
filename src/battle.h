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
#ifndef BATTLE_H
#define BATTLE_H


#include "immovable.h"
#include "soldier.h"

namespace Widelands {

/**
 * Manages the battle between two opposing soldiers.
 *
 * A \ref Battle object is created using the \ref create() function
 * as soon as a soldier decides he wants to attack someone else.
 */
struct Battle : public Map_Object {
	typedef Map_Object_Descr Descr;

	Battle();
	~Battle();

	static Battle* create(Game* g, Soldier* first, Soldier* second);

	virtual int32_t get_type() const throw () {return BATTLE;}
	char const * type_name() const throw () {return "battle";}

	void init(Editor_Game_Base*);
	void cleanup(Editor_Game_Base*);
	void cancel(Game*, Soldier*);

	Soldier* first() {return m_first;}
	Soldier* second() {return m_second;}

	/**
	 * \return \c true if the battle should not be interrupted.
	 */
	bool locked(Game* g);

	/**
	 * \param soldier must be one of the soldier involved in this battle
	 * \return the other soldier
	 */
	Soldier* getOpponent(Soldier* soldier);

	/**
	 * Called by the battling soldiers once they've met on a common node
	 * and are idle.
	 */
	void getBattleWork(Game* g, Soldier* soldier);

private:
	void calculateTurn(Game* g);

	Soldier* m_first;
	Soldier* m_second;

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
	 * \c true if the first soldier is the next to strike.
	 */
	bool m_first_strikes;

	// Load/save support
protected:
	struct Loader : public Map_Object::Loader {
		virtual void load(FileRead&);
		virtual void load_pointers();

		Serial m_first;
		Serial m_second;
	};

public:
	// Remove as soon as we fully support the new system
	virtual bool has_new_save_support() {return true;}

	virtual void save(Editor_Game_Base *, Map_Map_Object_Saver *, FileWrite &);
	static Map_Object::Loader * load
		(Editor_Game_Base *, Map_Map_Object_Loader*, FileRead &);
};

};

#endif
