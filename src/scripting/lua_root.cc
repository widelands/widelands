/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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

#include "scripting/lua_root.h"

#include <memory>

#include "logic/cmd_luacoroutine.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/world.h"
#include "scripting/globals.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"
#include "scripting/lua_table.h"
#include "wui/interactive_player.h"

using namespace Widelands;

namespace LuaRoot {

/* RST
:mod:`wl`
======================

.. module:: wl
   :synopsis: Base classes which allow access to all widelands internals.

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl
*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/

/* RST
Game
-----

.. class:: Game

   Child of: :class:`wl.bases.EditorGameBase`

   The root class to access the game internals. You can
   construct as many instances of this class as you like, but
   they all will access the one game currently running.
*/
const char LuaGame::className[] = "Game";
const MethodType<LuaGame> LuaGame::Methods[] = {
   METHOD(LuaGame, launch_coroutine),
   METHOD(LuaGame, save),
   {nullptr, nullptr},
};
const PropertyType<LuaGame> LuaGame::Properties[] = {
   PROP_RO(LuaGame, real_speed),         PROP_RO(LuaGame, time),
   PROP_RW(LuaGame, desired_speed),      PROP_RW(LuaGame, allow_saving),
   PROP_RO(LuaGame, last_save_time),     PROP_RO(LuaGame, type),
   PROP_RO(LuaGame, interactive_player), PROP_RO(LuaGame, scenario_difficulty),
   {nullptr, nullptr, nullptr},
};

LuaGame::LuaGame(lua_State* /* L */) {
	// Nothing to do.
}

void LuaGame::__persist(lua_State* /* L */) {
}
void LuaGame::__unpersist(lua_State* /* L */) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: real_speed

      (RO) The speed that the current game is running at in ms.
           For example, for game speed = 2x, this returns 2000.
*/
int LuaGame::get_real_speed(lua_State* L) {
	lua_pushinteger(L, get_game(L).game_controller()->real_speed());
	return 1;
}

/* RST
   .. attribute:: time

   (RO) The absolute time elapsed since the game was started in milliseconds.
*/
int LuaGame::get_time(lua_State* L) {
	lua_pushint32(L, get_game(L).get_gametime());
	return 1;
}

/* RST
   .. attribute:: desired_speed

   (RW) Sets the desired speed of the game in ms per real second, so a speed of
   2000 means the game runs at 2x speed. Note that this will not work in
   network games as expected.
*/
// UNTESTED
int LuaGame::set_desired_speed(lua_State* L) {
	get_game(L).game_controller()->set_desired_speed(luaL_checkuint32(L, -1));
	return 1;
}
// UNTESTED
int LuaGame::get_desired_speed(lua_State* L) {
	lua_pushuint32(L, get_game(L).game_controller()->desired_speed());
	return 1;
}

/* RST
   .. attribute:: allow_saving

      (RW) Disable or enable saving. When you show off UI features in a
      tutorial or scenario, you have to disallow saving because UI
      elements can not be saved and therefore reloading a game saved in the
      meantime would crash the game.
*/
// UNTESTED
int LuaGame::set_allow_saving(lua_State* L) {
	get_game(L).save_handler().set_allow_saving(luaL_checkboolean(L, -1));
	return 0;
}
// UNTESTED
int LuaGame::get_allow_saving(lua_State* L) {
	lua_pushboolean(L, get_game(L).save_handler().get_allow_saving());
	return 1;
}

/* RST
   .. attribute:: interactive_player

      (RO) The player number of the interactive player, or 0 for spectator
*/
int LuaGame::get_interactive_player(lua_State* L) {
	upcast(const InteractivePlayer, p, get_game(L).get_ibase());
	lua_pushuint32(L, p ? p->player_number() : 0);
	return 1;
}

/* RST
   .. attribute:: last_save_time

      (RO) The gametime at which the game was last saved.
*/
int LuaGame::get_last_save_time(lua_State* L) {
	lua_pushuint32(L, get_game(L).save_handler().last_save_time());
	return 1;
}

/* RST
   .. attribute:: type

      (RO) One string out of 'undefined', 'singleplayer', 'netclient', 'nethost', 'replay',
      describing the type of game that is played.
*/
int LuaGame::get_type(lua_State* L) {
	// enum class GameType : uint8_t { kUndefined = 0, kSingleplayer, kNetClient, kNetHost, kReplay
	// };
	switch (get_game(L).game_controller()->get_game_type()) {
	case GameController::GameType::kSingleplayer:
		lua_pushstring(L, "singleplayer");
		break;
	case GameController::GameType::kNetClient:
		lua_pushstring(L, "netclient");
		break;
	case GameController::GameType::kNetHost:
		lua_pushstring(L, "nethost");
		break;
	case GameController::GameType::kReplay:
		lua_pushstring(L, "replay");
		break;
	default:
		lua_pushstring(L, "undefined");
		break;
	}
	return 1;
}

/* RST
   .. attribute:: scenario_difficulty

      (RO) The difficulty level of the current scenario. Values range from 1 to the number
      of levels specified in the campaign's configuration in campaigns.lua. By convention
      higher values mean more difficult. Throws an error if used outside of a scenario.
*/
int LuaGame::get_scenario_difficulty(lua_State* L) {
	const uint32_t d = get_game(L).get_scenario_difficulty();
	if (d == kScenarioDifficultyNotSet) {
		report_error(L, "Scenario difficulty not set");
	}
	lua_pushuint32(L, d);
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
   .. method:: launch_coroutine(func[, when = now])

      Hands a Lua coroutine object over to widelands for execution. The object
      must have been created via :func:`coroutine.create`. The coroutine is
      expected to :func:`coroutine.yield` at regular intervals with the
      absolute game time on which the function should be awakened again. You
      should also have a look at :mod:`core.cr`.

      :arg func: coroutine object to run
      :type func: :class:`thread`
      :arg when: absolute time when this coroutine should run
      :type when: :class:`integer`

      :returns: :const:`nil`
*/
int LuaGame::launch_coroutine(lua_State* L) {
	int nargs = lua_gettop(L);
	uint32_t runtime = get_game(L).get_gametime();
	if (nargs < 2) {
		report_error(L, "Too few arguments!");
	}
	if (nargs == 3) {
		runtime = luaL_checkuint32(L, 3);
		lua_pop(L, 1);
	}

	std::unique_ptr<LuaCoroutine> cr(new LuaCoroutine(luaL_checkthread(L, 2)));
	lua_pop(L, 2);  // Remove coroutine and Game object from stack

	get_game(L).enqueue_command(new Widelands::CmdLuaCoroutine(runtime, std::move(cr)));

	return 0;
}

/* RST
   .. method:: save(name)

      Requests a savegame. Note that the actual save will be performed
      later, and that you have no control over any error that may happen
      by then currently.

      :arg name: name of save game, as if entered in the save dialog.
         If this game already exists, it will be silently overwritten.
         If empty, the autosave name will be used.
      :type name: :class:`string`
      :returns: :const:`nil`
*/
int LuaGame::save(lua_State* L) {
	const std::string filename = luaL_checkstring(L, -1);
	get_game(L).save_handler().request_save(filename);

	// DO NOT REMOVE THIS OUTPUT. It is used by the regression test suite to
	// figure out which files to load after a save was requested in a test.
	log("Script requests save to: %s\n", filename.c_str());

	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Editor
------

.. class:: Editor

   Child of: :class:`wl.bases.EditorGameBase`

   The Editor object; it is the correspondence of the :class:`wl.Game`
   that is used in a Game.
*/

const char LuaEditor::className[] = "Editor";
const MethodType<LuaEditor> LuaEditor::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaEditor> LuaEditor::Properties[] = {
   {nullptr, nullptr, nullptr},
};

LuaEditor::LuaEditor(lua_State* /* L */) {
	// Nothing to do.
}

void LuaEditor::__persist(lua_State* /* L */) {
}
void LuaEditor::__unpersist(lua_State* /* L */) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
World
-----

.. class:: World

   This offers access to the objects in the Widelands world and allows to add new objects.
   On how to build the world and adding new objects to it, see
   :ref:`toc_lua_world`.
*/

const char LuaWorld::className[] = "World";
const MethodType<LuaWorld> LuaWorld::Methods[] = {
   METHOD(LuaWorld, new_critter_type),
   METHOD(LuaWorld, new_immovable_type),
   METHOD(LuaWorld, new_resource_type),
   METHOD(LuaWorld, new_terrain_type),
   {0, 0},
};
const PropertyType<LuaWorld> LuaWorld::Properties[] = {
   PROP_RO(LuaWorld, immovable_descriptions),
   PROP_RO(LuaWorld, terrain_descriptions),
   {0, 0, 0},
};

LuaWorld::LuaWorld(lua_State* /* L */) {
	// Nothing to do.
}

void LuaWorld::__persist(lua_State*) {
	// Nothing to be done.
}
void LuaWorld::__unpersist(lua_State*) {
	// Nothing to be done.
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. attribute:: immovable_descriptions

      Returns a list of all the immovables that are available in the world.

      (RO) a list of :class:`LuaImmovableDescription` objects
*/
int LuaWorld::get_immovable_descriptions(lua_State* L) {
	const World& world = get_egbase(L).world();
	lua_newtable(L);
	int index = 1;
	for (DescriptionIndex i = 0; i < world.get_nr_immovables(); ++i) {
		lua_pushint32(L, index++);
		to_lua<LuaMaps::LuaImmovableDescription>(
		   L, new LuaMaps::LuaImmovableDescription(world.get_immovable_descr(i)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: terrain_descriptions

      Returns a list of all the terrains that are available in the world.

      (RO) a list of :class:`LuaTerrainDescription` objects
*/
int LuaWorld::get_terrain_descriptions(lua_State* L) {
	const World& world = get_egbase(L).world();
	lua_newtable(L);
	int index = 1;
	for (DescriptionIndex i = 0; i < world.terrains().size(); ++i) {
		lua_pushint32(L, index++);
		to_lua<LuaMaps::LuaTerrainDescription>(
		   L, new LuaMaps::LuaTerrainDescription(&world.terrain_descr(i)));
		lua_settable(L, -3);
	}
	return 1;
}

// Documented in data/world/resources/init.lua.
// See also the World and Tribes section in the Widelands Scripting Reference on the website.
int LuaWorld::new_resource_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_world()->add_world_object_type(table, MapObjectType::RESOURCE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}

	return 0;
}

// Documented in data/world/terrains/init.lua.
// See also the World and Tribes section in the Widelands Scripting Reference on the website.
int LuaWorld::new_terrain_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_world()->add_world_object_type(table, MapObjectType::TERRAIN);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}

	return 0;
}

// Documented in data/world/critters/badger/init.lua.
// See also the World and Tribes section in the Widelands Scripting Reference on the website.
int LuaWorld::new_critter_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);
		get_egbase(L).mutable_world()->add_world_object_type(table, MapObjectType::CRITTER);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

// Documented in data/world/immovables/bush1/init.lua.
// See also the World and Tribes section in the Widelands Scripting Reference on the website.
int LuaWorld::new_immovable_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_world()->add_world_object_type(table, MapObjectType::IMMOVABLE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Tribes
------

.. class:: Tribes

   This offers access to the objects available for the tribes and allows to add
   new objects. See :ref:`lua_tribes` for detailed documentation.
*/

const char LuaTribes::className[] = "Tribes";
const MethodType<LuaTribes> LuaTribes::Methods[] = {
   METHOD(LuaTribes, new_carrier_type),
   METHOD(LuaTribes, new_ferry_type),
   METHOD(LuaTribes, new_constructionsite_type),
   METHOD(LuaTribes, new_dismantlesite_type),
   METHOD(LuaTribes, new_immovable_type),
   METHOD(LuaTribes, new_market_type),
   METHOD(LuaTribes, new_militarysite_type),
   METHOD(LuaTribes, new_productionsite_type),
   METHOD(LuaTribes, new_ship_type),
   METHOD(LuaTribes, new_soldier_type),
   METHOD(LuaTribes, new_trainingsite_type),
   METHOD(LuaTribes, new_tribe),
   METHOD(LuaTribes, new_ware_type),
   METHOD(LuaTribes, new_warehouse_type),
   METHOD(LuaTribes, new_worker_type),
   {0, 0},
};
const PropertyType<LuaTribes> LuaTribes::Properties[] = {
   {0, 0, 0},
};

LuaTribes::LuaTribes(lua_State* /* L */) {
	// Nothing to do.
}

void LuaTribes::__persist(lua_State*) {
	// Nothing to be done.
}

void LuaTribes::__unpersist(lua_State*) {
	// Nothing to be done.
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: new_constructionsite_type{table}

      Adds a new constructionsite building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_constructionsites` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_constructionsite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::CONSTRUCTIONSITE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_dismantlesite_type{table}

      Adds a new disnamtlesite building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_dismantlesites` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_dismantlesite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::DISMANTLESITE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_militarysite_type{table}

      Adds a new militarysite building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_militarysites` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_militarysite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::MILITARYSITE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_productionsite_type{table}

      Adds a new productionsite building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_productionsites` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_productionsite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::PRODUCTIONSITE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_trainingsite_type{table}

      Adds a new trainingsite building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_trainingsites` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_trainingsite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::TRAININGSITE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_warehouse_type{table}

      Adds a new warehouse building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_warehouses` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_warehouse_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::WAREHOUSE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

// TODO(GunChleoc): add RST marker
/*
   .. method:: new_market_type{table}

      Adds a new market building type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_buildings_markets` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_market_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::MARKET);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_immovable_type{table}

      Adds a new immovable type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_immovables` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_immovable_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(
		   table, *egbase.mutable_world(), MapObjectType::IMMOVABLE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_ship_type{table}

      Adds a new ship type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_ships` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_ship_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::SHIP);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_ware_type{table}

      Adds a new ware type. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_wares` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_ware_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::WARE);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_carrier_type{table}

      Adds a new carrier worker type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_carrier_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::CARRIER);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_ferry_type{table}

      Adds a new ferry worker type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_ferry_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::FERRY);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_soldier_type{table}

      Adds a new soldier worker type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_soldier_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::SOLDIER);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_worker_type{table}

      Adds a new worker type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_worker_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe_object_type(table, *egbase.mutable_world(), MapObjectType::WORKER);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_tribe{table}

      Adds a new tribe. Takes a single argument, a table with
      the descriptions. See :ref:`lua_tribes_tribes_units` for detailed
      documentation.

      :returns: :const:`0`
*/
int LuaTribes::new_tribe(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe(table, egbase.world());
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

const static struct luaL_Reg wlroot[] = {{nullptr, nullptr}};

void luaopen_wlroot(lua_State* L, bool in_editor) {
	lua_getglobal(L, "wl");       // S: wl
	luaL_setfuncs(L, wlroot, 0);  // S: wl
	lua_pop(L, 1);                // S:

	if (in_editor) {
		register_class<LuaEditor>(L, "", true);
		add_parent<LuaEditor, LuaBases::LuaEditorGameBase>(L);
		lua_pop(L, 1);  // Pop the meta table
	} else {
		register_class<LuaGame>(L, "", true);
		add_parent<LuaGame, LuaBases::LuaEditorGameBase>(L);
		lua_pop(L, 1);  // Pop the meta table
	}
	register_class<LuaWorld>(L, "", false);
	register_class<LuaTribes>(L, "", false);
}
}  // namespace LuaRoot
