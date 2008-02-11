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

struct Battle : public BaseImmovable {
	friend struct Map_Battle_Data_Packet;

	typedef Map_Object_Descr Descr;

	Battle();
	~Battle();
	virtual int32_t  get_type    () const throw () {return BATTLE;}
	virtual int32_t  get_size    () const throw () {return SMALL;}
	virtual bool get_passable() const throw () {return false;}

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point)
	{}

	void init (Editor_Game_Base*);
	void init (Editor_Game_Base*, Soldier*, Soldier*);
	void soldiers (Soldier*, Soldier*);
	virtual void cleanup (Editor_Game_Base*);
	virtual void act (Game*, uint32_t);

private:
	Soldier* m_first;
	Soldier* m_second;
	int32_t      m_last_try;
	int32_t      m_next_assault;

	// Load/save support
protected:
	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead&);
		virtual void load_pointers();

		uint32_t m_first;
		uint32_t m_second;
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
