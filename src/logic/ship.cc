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

#include "editor_game_base.h"
#include "game_data_error.h"
#include "tribe.h"

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


/*
==============================

Load / Save implementation

==============================
*/

#define SHIP_SAVEGAME_VERSION 1

Ship::Loader::Loader()
{
}

const Bob::Task * Ship::Loader::get_task(const std::string & name)
{
	if (name == "shipidle") return &taskShipIdle;
	return Bob::Loader::get_task(name);
}


Map_Object::Loader * Ship::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const version = fr.Unsigned8();
		if (1 <= version && version <= SHIP_SAVEGAME_VERSION) {
			std::string owner = fr.CString();
			std::string name = fr.CString();
			const Ship_Descr * descr = 0;

			egbase.manually_load_tribe(owner);

			if (const Tribe_Descr * tribe = egbase.get_tribe(owner))
				descr = dynamic_cast<const Ship_Descr *>
					(tribe->get_bob_descr(name));

			if (!descr)
				throw game_data_error
					("undefined ship %s/%s", owner.c_str(), name.c_str());

			loader->init(egbase, mol, descr->create_object());
			loader->load(fr);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (const std::exception & e) {
		throw wexception(_("loading ship: %s"), e.what());
	}

	return loader.release();
}

void Ship::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_Ship);
	fw.Unsigned8(SHIP_SAVEGAME_VERSION);

	fw.CString(descr().get_owner_tribe()->name());
	fw.CString(descr().name());

	Bob::save(egbase, mos, fw);
}

} // namespace Widelands
