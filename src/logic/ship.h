/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef SHIP_H
#define SHIP_H

#include "bob.h"
#include "graphic/diranimations.h"

namespace Widelands {

struct Economy;

struct Ship_Descr : Bob::Descr {
	Ship_Descr
		(char const * name, char const * descname,
		 std::string const & directory, Profile &, Section & global_s,
		 Tribe_Descr const &);

	virtual uint32_t movecaps() const throw ();
	DirAnimations const & get_sail_anims() const {return m_sail_anims;}

	virtual Bob & create_object() const;

private:
	DirAnimations m_sail_anims;
};

/**
 * Ships belong to a player and to an economy.
 */
struct Ship : Bob {
	MO_DESCR(Ship_Descr);

	Ship(const Ship_Descr & descr);

	virtual Type get_bob_type() const throw ();

	void init_auto_task(Game &);

	void start_task_shipidle(Game &);

private:
	void wakeup_neighbours(Game &);

	static const Task taskShipIdle;

	void shipidle_update(Game &, State &);
	void shipidle_wakeup(Game &);

	// saving and loading
protected:
	struct Loader : Bob::Loader {
		Loader();

		virtual Task const * get_task(std::string const &);
	};

public:
	virtual void save(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &);

	static Map_Object::Loader * load
		(Editor_Game_Base &, Map_Map_Object_Loader &, FileRead &);
};

} // namespace Widelands

#endif // SHIP_H
