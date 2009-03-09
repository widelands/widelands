/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "legacy.h"

#include "editor_game_base.h"
#include "immovable.h"

namespace Widelands {

namespace Legacy {

Map_Object_Descr g_FakeAttackController_Descr
	("attack_controller", "Attack controller");

struct FakeAttackController : public BaseImmovable {
	FakeAttackController() : BaseImmovable(g_FakeAttackController_Descr) {}

	virtual int32_t get_type() const throw () {return BATTLE;}
	virtual int32_t get_size() const throw () {return SMALL;}
	virtual bool get_passable() const throw () {return true;}
	virtual void draw (const Editor_Game_Base &, RenderTarget &, const FCoords, const Point) {}

	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead& fr) {
			BaseImmovable::Loader::load(fr);

			try {
				fr.Unsigned32();

				fr.Unsigned8();
				fr.Unsigned8();
				fr.Unsigned32();
				fr.Unsigned8();

				uint32_t numBs = fr.Unsigned32();

				for (uint32_t j = 0; j < numBs; ++j) {
					fr.Unsigned32();
					fr.Unsigned32();
					fr.Coords32(egbase().map().extent());

					fr.Unsigned8();
					fr.Unsigned8();
					fr.Unsigned8();
				}

				uint32_t numInMs = fr.Unsigned32();
				for (uint32_t j = 0; j < numInMs; ++j)
					fr.Unsigned32();
			} catch (_wexception const & e) {
				throw wexception
					("Error in legacy AttackController: binary/mapobjects:%s", e.what());
			}
		}

		virtual void load_finish() {
			get_object()->remove(&egbase());
		}
	};
};


Map_Object::Loader* loadAttackController(Editor_Game_Base *egbase, Map_Map_Object_Loader *mol, FileRead & fr)
{
	std::auto_ptr<FakeAttackController::Loader> loader
		(new FakeAttackController::Loader);

	try {
		uint8_t const version = fr.Unsigned8();
		if (version != 1)
			throw wexception("unknown/unhandled version %u", version);

		loader->init(egbase, mol, new FakeAttackController);
		loader->load(fr);
	} catch (const std::exception& e) {
		throw wexception("Loading legacy AttackController: %s", e.what());
	}

	return loader.release();
}


Map_Object_Descr g_FakeBattle_Descr("battle", "Battle");

struct FakeBattle : public BaseImmovable {
	FakeBattle() : BaseImmovable(g_FakeBattle_Descr) {}

	virtual int32_t get_type() const throw () {return BATTLE;}
	virtual int32_t get_size() const throw () {return SMALL;}
	virtual bool get_passable() const throw () {return true;}
	virtual void draw (const Editor_Game_Base &, RenderTarget &, const FCoords, const Point) {}

	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead& fr) {
			BaseImmovable::Loader::load(fr);

			fr.Unsigned32();
			fr.Unsigned32();

			fr.Unsigned32();
			fr.Unsigned32();
		}

		virtual void load_finish() {
			get_object()->remove(&egbase());
		}
	};
};

Map_Object::Loader* loadBattle(Editor_Game_Base * egbase, Map_Map_Object_Loader * mol, FileRead & fr)
{
	std::auto_ptr<FakeBattle::Loader> loader(new FakeBattle::Loader);

	try {
		// Header has been peeled away by caller
		uint8_t const version = fr.Unsigned8();
		if (version != 1)
			throw wexception("unknown/unhandled version %u", version);

		loader->init(egbase, mol, new FakeBattle);
		loader->load(fr);
	} catch (const std::exception & e) {
		throw wexception("Loading legacy Battle: %s", e.what());
	}

	return loader.release();
}

}

}
