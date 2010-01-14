/*
 * Copyright (C) 2008-2010 by the Widelands Development Team
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
#include "game_data_error.h"
#include "immovable.h"
#include "tribe.h"

namespace Widelands {

namespace Legacy {

//  These tables are needed for compatibility with requests written in version
//  <= 3.
static char const * const barbarian_ware_types[] = {
	"axe",
	"bakingtray",
	"battleaxe",
	"beer",
	"blackwood",
	"broadaxe",
	"bronzeaxe",
	"cloth",
	"coal",
	"fire_tongs",
	"fish",
	"fishing_rod",
	"flax",
	"gold",
	"goldstone",
	"grout",
	"hammer",
	"helm",
	"hunting_spear",
	"iron",
	"ironore",
	"kitchen_tools",
	"mask",
	"meal",
	"meat",
	"pick",
	"pittabread",
	"ration",
	"raw_stone",
	"scythe",
	"sharpaxe",
	"shovel",
	"snack",
	"strongbeer",
	"thatchreed",
	"trunk",
	"warhelmet",
	"warriorsaxe",
	"water",
	"wheat"
};
static char const * const barbarian_worker_types[] = {
	"baker",
	"blacksmith",
	"brewer",
	"builder",
	"burner",
	"carrier",
	"chief-miner",
	"farmer",
	"ferner",
	"fisher",
	"gamekeeper",
	"geologist",
	"helmsmith",
	"hunter",
	"innkeeper",
	"lime-burner",
	"lumberjack",
	"master-blacksmith",
	"master-brewer",
	"master-miner",
	"miner",
	"ranger",
	"smelter",
	"soldier",
	"stonemason",
	"trainer",
	"weaver"
};
static char const * const empire_ware_types[] = {
	"advanced_lance",
	"armour",
	"axe",
	"bakingtray",
	"basket",
	"beer",
	"bread",
	"chain_armour",
	"cloth",
	"coal",
	"fire_tongs",
	"fish",
	"fishing_rod",
	"flour",
	"gold",
	"goldstone",
	"grape",
	"hammer",
	"heavy_lance",
	"helm",
	"hunting_spear",
	"iron",
	"ironore",
	"kitchen_tools",
	"lance",
	"marble",
	"marblecolumn",
	"meal",
	"meat",
	"pick",
	"plate_armour",
	"ration",
	"saw",
	"scythe",
	"shovel",
	"stone",
	"trunk",
	"war_lance",
	"water",
	"wheat",
	"wine",
	"wood",
	"wood_lance",
	"wool",
};
static char const * const empire_worker_types[] = {
	"armoursmith",
	"baker",
	"brewer",
	"builder",
	"burner",
	"carrier",
	"farmer",
	"fisher",
	"forester",
	"geologist",
	"hunter",
	"innkeeper",
	"lumberjack",
	"master-miner",
	"miller",
	"miner",
	"pig-breeder",
	"shepherd",
	"smelter",
	"soldier",
	"stonemason",
	"toolsmith",
	"trainer",
	"weaponsmith",
	"weaver",
	"vinefarmer"
};
static char const * const atlantean_ware_types[] = {
	"advanced_shield",
	"bakingtray",
	"blackroot",
	"blackrootflour",
	"bread",
	"bucket",
	"coal",
	"corn",
	"cornflour",
	"diamond",
	"double_trident",
	"fire_tongs",
	"fish",
	"fishing_net",
	"gold",
	"golden_tabard",
	"goldore",
	"goldyarn",
	"hammer",
	"heavy_double_trident",
	"hook_pole",
	"hunting_bow",
	"iron",
	"ironore",
	"light_trident",
	"long_trident",
	"meat",
	"milking_tongs",
	"pick",
	"planks",
	"quartz",
	"saw",
	"scythe",
	"shovel",
	"smoked_fish",
	"smoked_meat",
	"spidercloth",
	"spideryarn",
	"steel_shield",
	"steel_trident",
	"stone",
	"tabard",
	"trunk",
	"water"
};
static char const * const atlantean_worker_types[] = {
	"armoursmith",
	"baker",
	"blackroot_farmer",
	"builder",
	"burner",
	"carrier",
	"farmer",
	"fish_breeder",
	"fisher",
	"forester",
	"geologist",
	"hunter",
	"miller",
	"miner",
	"sawyer",
	"smelter",
	"smoker",
	"soldier",
	"spiderbreeder",
	"stonecutter",
	"toolsmith",
	"trainer",
	"weaponsmith",
	"weaver",
	"woodcutter"
};

Ware_Index ware_index
	(Tribe_Descr const &       tribe,
	 std::string const &       name,
	 char        const * const relation,
	 uint32_t            const legacy_index)
{
	char const * type_name;
	if        (tribe.name() == "barbarians") {
		if
			(sizeof (barbarian_ware_types) / sizeof (*barbarian_ware_types)
			 <=
			 legacy_index)
			throw wexception
				("could not interpret legacy barbarian ware index %u",
				 legacy_index);
		type_name = barbarian_ware_types[legacy_index];
	} else if (tribe.name() == "empire")     {
		if
			(sizeof (empire_ware_types) / sizeof (*empire_ware_types)
			 <=
			 legacy_index)
			throw wexception
				("could not interpret legacy empire ware index %u",
				 legacy_index);
		type_name = empire_ware_types[legacy_index];
	} else if (tribe.name() == "atlanteans") {
		if
			(sizeof (atlantean_ware_types) / sizeof (*atlantean_ware_types)
			 <=
			 legacy_index)
			throw wexception
				("could not interpret legacy atlantean ware index %u",
				 legacy_index);
		type_name = atlantean_ware_types[legacy_index];
	} else
		throw wexception
			("no backwards compatibility support for tribe %s",
			 tribe.name().c_str());
	log
		("WARNING: Interpreting a legacy ware index: A %s %s %s a ware of type "
		 "%u. This is interpreted as %s.\n",
		 tribe.name().c_str(), name.c_str(), relation, legacy_index, type_name);
	return tribe.safe_ware_index(type_name);
}


Ware_Index worker_index
	(Tribe_Descr const &       tribe,
	 std::string const &       name,
	 char        const * const relation,
	 uint32_t            const legacy_index)
{
	char const * type_name;
	if        (tribe.name() == "barbarians") {
		if
			(sizeof (barbarian_worker_types) / sizeof (*barbarian_worker_types)
			 <=
			 legacy_index)
			throw wexception
				("could not interpret legacy barbarian worker index %u",
				 legacy_index);
		type_name = barbarian_worker_types[legacy_index];
	} else if (tribe.name() == "empire")     {
		if
			(sizeof (empire_worker_types) / sizeof (*empire_worker_types)
			 <=
			 legacy_index)
			throw wexception
				("could not interpret legacy empire worker index %u",
				 legacy_index);
		type_name = empire_worker_types[legacy_index];
	} else if (tribe.name() == "atlanteans") {
		if
			(sizeof (atlantean_worker_types) / sizeof (*atlantean_worker_types)
			 <=
			 legacy_index)
			throw wexception
				("could not interpret legacy atlantean worker index %u",
				 legacy_index);
		type_name = atlantean_worker_types[legacy_index];
	} else
		throw wexception
			("no backwards compatibility support for tribe %s",
			 tribe.name().c_str());
	log
		("WARNING: Interpreting a legacy worker index: A %s %s %s a worker of "
		 "type %u. This is interpreted as %s.\n",
		 tribe.name().c_str(), name.c_str(), relation, legacy_index, type_name);
	return tribe.safe_worker_index(type_name);
}


Map_Object_Descr g_FakeAttackController_Descr
	("attack_controller", "Attack controller");

struct FakeAttackController : public BaseImmovable {
	FakeAttackController() : BaseImmovable(g_FakeAttackController_Descr) {}

	virtual int32_t get_type() const throw () {return BATTLE;}
	virtual int32_t get_size() const throw () {return SMALL;}
	virtual bool get_passable() const throw () {return true;}
	virtual void draw (Editor_Game_Base const &, RenderTarget &, FCoords, Point)
	{}

	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead & fr, uint8_t const version) {
			BaseImmovable::Loader::load(fr, version);

			try {
				fr.Unsigned32();

				fr.Unsigned8();
				fr.Unsigned8();
				fr.Unsigned32();
				fr.Unsigned8();

				uint32_t const numBs = fr.Unsigned32();

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
					("Error in legacy AttackController: binary/mapobjects:%s",
					 e.what());
			}
		}

		virtual void load_finish() {
			get_object()->remove(egbase());
		}
	};
};


Map_Object::Loader * loadAttackController
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<FakeAttackController::Loader> loader
		(new FakeAttackController::Loader);

	try {
		uint8_t const version = fr.Unsigned8();
		if (version != 1)
			throw game_data_error(_("unknown/unhandled version %u"), version);

		loader->init(egbase, mol, *new FakeAttackController);
		loader->load(fr, version);
	} catch (std::exception const & e) {
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
	virtual void draw (Editor_Game_Base const &, RenderTarget &, FCoords, Point)
	{}

	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead & fr, uint8_t const version) {
			BaseImmovable::Loader::load(fr, version);

			fr.Unsigned32();
			fr.Unsigned32();

			fr.Unsigned32();
			fr.Unsigned32();
		}

		virtual void load_finish() {get_object()->remove(egbase());}
	};
};

Map_Object::Loader * loadBattle
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::auto_ptr<FakeBattle::Loader> loader(new FakeBattle::Loader);

	try {
		// Header has been peeled away by caller
		uint8_t const version = fr.Unsigned8();
		if (1 == version) {
			loader->init(egbase, mol, *new FakeBattle);
			loader->load(fr, version);
		} else
			throw game_data_error(_("unknown/unhandled version %u"), version);
	} catch (const std::exception & e) {
		throw wexception("Loading legacy Battle: %s", e.what());
	}

	return loader.release();
}

}

}
