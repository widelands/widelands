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

#include "ship.h"


namespace Widelands {

Ship_Descr::Ship_Descr
	(const char* name, const char* descname,
	 const std::string& directory, Profile& prof, Section& global_s,
	 const Widelands::Tribe_Descr& tribe)
: Descr(name, descname, directory, prof, global_s, &tribe)
{

}

Bob& Ship_Descr::create_object() const
{
	return *new Ship(*this);
}


Ship::Ship(const Ship_Descr& descr)
:
Bob(descr)
{
}

Bob::Type Ship::get_bob_type() const throw()
{
	return SHIP;
}

void Ship::init_auto_task(Game& game)
{
	start_task_shipidle(game);
}

/**
 * Standard behaviour of ships while idle.
 */
const Bob::Task Ship::taskShipIdle = {
	"shipidle",
	static_cast<Bob::Ptr>(&Ship::shipidle_update),
	0,
	0,
	true // unique task
};

void Ship::start_task_shipidle(Game& game)
{
	push_task(game, taskShipIdle);
}

void Ship::shipidle_update(Game& game, Bob::State& state)
{
	// Sleep
	start_task_idle(game, descr().main_animation(), 60000);
}


} // namespace Widelands
