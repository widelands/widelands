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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/legacy.h"

#include "logic/editor_game_base.h"
#include "logic/game_data_error.h"
#include "logic/immovable.h"
#include "logic/tribe.h"

namespace Widelands {

namespace Legacy {

//  These tables are needed for compatibility with requests written in version
//  <= 3.
static char const * const barbarian_ware_types[] = {
	"ax",
	"bread_paddle",
	"battleax",
	"beer",
	"blackwood",
	"broadax",
	"bronzeax",
	"cloth",
	"coal",
	"fire_tongs",
	"fish",
	"fishing_rod",
	"flax",
	"gold",
	"goldore",
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
	"sharpax",
	"shovel",
	"snack",
	"strongbeer",
	"thatchreed",
	"log",
	"warhelm",
	"warriorsax",
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
	"gardener",
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
	"armor",
	"ax",
	"bread_paddle",
	"basket",
	"beer",
	"bread",
	"chain_armor",
	"cloth",
	"coal",
	"fire_tongs",
	"fish",
	"fishing_rod",
	"flour",
	"gold",
	"goldore",
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
	"plate_armor",
	"ration",
	"saw",
	"scythe",
	"shovel",
	"stone",
	"log",
	"war_lance",
	"water",
	"wheat",
	"wine",
	"wood",
	"wood_lance",
	"wool",
};
static char const * const empire_worker_types[] = {
	"armorsmith",
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
	"bread_paddle",
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
	"log",
	"water"
};
static char const * const atlantean_worker_types[] = {
	"armorsmith",
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
	(const Tribe_Descr &       tribe,
	 const std::string &       name,
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
	return tribe.ware_index(type_name);
}

Ware_Index safe_ware_index
	(const Tribe_Descr &       tribe,
	 const std::string &       name,
	 char        const * const relation,
	 uint32_t            const legacy_index)
{
	Ware_Index ware = ware_index(tribe, name, relation, legacy_index);
	if (!ware)
		throw game_data_error
			("Legacy ware %i of tribe %s no longer exists",
			 legacy_index, tribe.name().c_str());
	return ware;
}

Ware_Index worker_index
	(const Tribe_Descr &       tribe,
	 const std::string &       name,
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

	virtual int32_t get_type() const override {return BATTLE;}
	virtual int32_t get_size() const override {return SMALL;}
	virtual bool get_passable() const override {return true;}
	virtual void draw (const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&) override
	{}
	virtual PositionList get_positions (const Editor_Game_Base &) const override
	{
		// This violates what I had in mind for get_positions, but since this is
		// attic code and get_positions was added long after this code was gone
		// I guess it is save to return an empty list here.
		PositionList rv;
		return rv;
	}


	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead & fr, uint8_t const /* version */) {
			BaseImmovable::Loader::load(fr);

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
			} catch (const _wexception & e) {
				throw wexception
					("Error in legacy AttackController: binary/mapobjects:%s",
					 e.what());
			}
		}

		virtual void load_finish() override {
			get_object()->remove(egbase());
		}
	};
};


Map_Object::Loader * loadAttackController
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::unique_ptr<FakeAttackController::Loader> loader
		(new FakeAttackController::Loader);

	try {
		uint8_t const version = fr.Unsigned8();
		if (version != 1)
			throw game_data_error("unknown/unhandled version %u", version);

		loader->init(egbase, mol, *new FakeAttackController);
		loader->load(fr, version);
	} catch (const std::exception & e) {
		throw wexception("Loading legacy AttackController: %s", e.what());
	}

	return loader.release();
}


Map_Object_Descr g_FakeBattle_Descr("battle", "Battle");

struct FakeBattle : public BaseImmovable {
	FakeBattle() : BaseImmovable(g_FakeBattle_Descr) {}

	virtual int32_t get_type() const override {return BATTLE;}
	virtual int32_t get_size() const override {return SMALL;}
	virtual bool get_passable() const override {return true;}
	virtual void draw (const Editor_Game_Base &, RenderTarget &, const FCoords&, const Point&) override
	{}
	virtual PositionList get_positions (const Editor_Game_Base &) const override
	{
		// This violates what I had in mind for get_positions, but since this is
		// attic code and get_positions was added long after this code was gone
		// I guess it is save to return an empty list here.
		PositionList rv;
		return rv;
	}

	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead & fr, uint8_t const /* version */) {
			BaseImmovable::Loader::load(fr);

			fr.Unsigned32();
			fr.Unsigned32();

			fr.Unsigned32();
			fr.Unsigned32();
		}

		virtual void load_finish() override {get_object()->remove(egbase());}
	};
};

Map_Object::Loader * loadBattle
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	std::unique_ptr<FakeBattle::Loader> loader(new FakeBattle::Loader);

	try {
		// Header has been peeled away by caller
		uint8_t const version = fr.Unsigned8();
		if (1 == version) {
			loader->init(egbase, mol, *new FakeBattle);
			loader->load(fr, version);
		} else
			throw game_data_error("unknown/unhandled version %u", version);
	} catch (const std::exception & e) {
		throw wexception("Loading legacy Battle: %s", e.what());
	}

	return loader.release();
}

}

}
