/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "base/log.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/production_program.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker_program.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "scripting/globals.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"
#include "scripting/lua_table.h"
#include "wui/interactive_player.h"

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

   The root class to access the game internals. Use ``wl.Game()`` to access the properties, e.g.:

   .. code-block:: lua

      current_speed = wl.Game().real_Speed

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
   PROP_RO(LuaGame, win_condition),      {nullptr, nullptr, nullptr},
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
	lua_pushint32(L, get_game(L).get_gametime().get());
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

      (RO) One string out of :const:`"undefined"`, :const:`"singleplayer"`, :const:`"netclient"`,
      :const:`"nethost"`, :const:`"replay"`, describing the type of game that is played.
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
   .. attribute:: win_condition

      (RO) The (unlocalized) name of the game's win condition, e.g. :const:`"Endless Game"`.
      For scenarios this is :const:`"Scenario"`.
*/
int LuaGame::get_win_condition(lua_State* L) {
	lua_pushstring(L, get_game(L).get_win_condition_displayname().c_str());
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
	if (d == Widelands::kScenarioDifficultyNotSet) {
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
	Time runtime = get_game(L).get_gametime();
	if (nargs < 2) {
		report_error(L, "Too few arguments!");
	}
	if (nargs == 3) {
		runtime = Time(luaL_checkuint32(L, 3));
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
	log_info_time(get_game(L).get_gametime(), "Script requests save to: %s\n", filename.c_str());

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

   The Editor object; it is the correspondence of the :class:`wl.Game`
   that is used in a Game. Use ``ẁl.Editor()`` to access the properties.
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
World and Tribe Descriptions
----------------------------

.. class:: Descriptions

   This offers access to the objects in the Widelands world and allows to add new objects.
   On how to build the world and adding new objects to it, see
   :ref:`toc_lua_world` and :ref:`lua_tribes`.
*/

const char LuaDescriptions::className[] = "Descriptions";
const MethodType<LuaDescriptions> LuaDescriptions::Methods[] = {
   METHOD(LuaDescriptions, new_critter_type),
   METHOD(LuaDescriptions, new_immovable_type),
   METHOD(LuaDescriptions, new_resource_type),
   METHOD(LuaDescriptions, new_terrain_type),
   METHOD(LuaDescriptions, new_carrier_type),
   METHOD(LuaDescriptions, new_ferry_type),
   METHOD(LuaDescriptions, new_constructionsite_type),
   METHOD(LuaDescriptions, new_dismantlesite_type),
   METHOD(LuaDescriptions, new_market_type),
   METHOD(LuaDescriptions, new_militarysite_type),
   METHOD(LuaDescriptions, new_productionsite_type),
   METHOD(LuaDescriptions, new_ship_type),
   METHOD(LuaDescriptions, new_soldier_type),
   METHOD(LuaDescriptions, new_trainingsite_type),
   METHOD(LuaDescriptions, new_tribe),
   METHOD(LuaDescriptions, new_ware_type),
   METHOD(LuaDescriptions, new_warehouse_type),
   METHOD(LuaDescriptions, new_worker_type),
   METHOD(LuaDescriptions, modify_unit),
   {nullptr, nullptr},
};
const PropertyType<LuaDescriptions> LuaDescriptions::Properties[] = {
   PROP_RO(LuaDescriptions, immovable_descriptions),
   PROP_RO(LuaDescriptions, terrain_descriptions),
   PROP_RO(LuaDescriptions, worker_descriptions),
   {nullptr, nullptr, nullptr},
};

LuaDescriptions::LuaDescriptions(lua_State* /* L */) {
	// Nothing to do.
}

void LuaDescriptions::__persist(lua_State*) {
	// Nothing to be done.
}
void LuaDescriptions::__unpersist(lua_State*) {
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

      Returns a list of all the immovables that are available.

      (RO) a list of :class:`~wl.map.ImmovableDescription` objects
*/
int LuaDescriptions::get_immovable_descriptions(lua_State* L) {
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	lua_newtable(L);
	int index = 1;
	for (Widelands::DescriptionIndex i = 0; i < descriptions.nr_immovables(); ++i) {
		lua_pushint32(L, index++);
		to_lua<LuaMaps::LuaImmovableDescription>(
		   L, new LuaMaps::LuaImmovableDescription(descriptions.get_immovable_descr(i)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: terrain_descriptions

      Returns a list of all the terrains that are available.

      (RO) a list of :class:`~wl.map.TerrainDescription` objects
*/
int LuaDescriptions::get_terrain_descriptions(lua_State* L) {
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	lua_newtable(L);
	int index = 1;
	for (Widelands::DescriptionIndex i = 0; i < descriptions.terrains().size(); ++i) {
		lua_pushint32(L, index++);
		to_lua<LuaMaps::LuaTerrainDescription>(
		   L, new LuaMaps::LuaTerrainDescription(descriptions.get_terrain_descr(i)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: worker_descriptions

      Returns a list of all the workers that are available.

      (RO) a list of :class:`~wl.map.WorkerDescription` objects
*/
int LuaDescriptions::get_worker_descriptions(lua_State* L) {
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	lua_newtable(L);
	int index = 1;
	for (Widelands::DescriptionIndex i = 0; i < descriptions.workers().size(); ++i) {
		lua_pushint32(L, index++);
		to_lua<LuaMaps::LuaWorkerDescription>(
		   L, new LuaMaps::LuaWorkerDescription(descriptions.get_worker_descr(i)));
		lua_settable(L, -3);
	}
	return 1;
}

namespace {
// Loads instantiation for a subclass of Widelands::MapObjectDescr into the engine.
int new_map_object_description(lua_State* L, Widelands::MapObjectType type) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_descriptions()->add_object_description(table, type);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}
}  // namespace

// Documented in data/world/resources/init.lua.
// See also the World and Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_resource_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::RESOURCE);
}

// Documented in data/world/terrains/desert/beach/init.lua.
// See also the World section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_terrain_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::TERRAIN);
}

// Documented in data/world/critters/badger/init.lua.
// See also the World section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_critter_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::CRITTER);
}

// Documented in data/world/immovables/artifacts/artifact00/init.lua and
// data/tribes/immovables/ashes/init.lua
// See also the World and Tribes sections in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_immovable_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::IMMOVABLE);
}

// Documented in data/tribes/buildings/partially_finished/constructionsite/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_constructionsite_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::CONSTRUCTIONSITE);
}

// Documented in data/tribes/buildings/partially_finished/dismantlesite/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_dismantlesite_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::DISMANTLESITE);
}

// Documented in data/tribes/buildings/militarysites/atlanteans/castle/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_militarysite_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::MILITARYSITE);
}

// Documented in data/tribes/buildings/productionsites/atlanteans/armorsmithy/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_productionsite_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::PRODUCTIONSITE);
}

// Documented in data/tribes/buildings/trainingsites/atlanteans/dungeon/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_trainingsite_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::TRAININGSITE);
}

// Documented in data/tribes/buildings/warehouses/atlanteans/headquarters/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_warehouse_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::WAREHOUSE);
}

// Only used in the test suite for now
int LuaDescriptions::new_market_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::MARKET);
}

// Documented in data/tribes/ships/atlanteans/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_ship_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::SHIP);
}

// Documented in data/tribes/wares/armor/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_ware_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::WARE);
}

// Documented in data/tribes/workers/atlanteans/carrier/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_carrier_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::CARRIER);
}

// Documented in data/tribes/workers/atlanteans/ferry/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_ferry_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::FERRY);
}

// Documented in data/tribes/workers/atlanteans/soldier/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_soldier_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::SOLDIER);
}

// Documented in data/tribes/workers/atlanteans/armorsmith/init.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_worker_type(lua_State* L) {
	return new_map_object_description(L, Widelands::MapObjectType::WORKER);
}

// Documented in data/tribes/initialization/atlanteans/units.lua.
// See also the Tribes section in the Widelands Scripting Reference on the website.
int LuaDescriptions::new_tribe(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_descriptions()->add_tribe(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

// clang-format off
/* RST
   .. method:: modify_unit(type, name, property … , value …)

      This powerful function can be used to modify almost
      every property of every object description.

      Currently, support is implemented only for a limited number of properties.
      Contact the Widelands Development Team if you need access to a property
      that can not be modified by this function yet.

      This function is meant to be used **only** by add-ons of the ``tribes``
      and ``world`` categories from their ``postload.lua``. **Do not** use it
      in scenario scripts, add-ons of other types, or the debug console.

      :arg string type: The object type to modify. See below for a list of valid values.
      :arg string name: The name of the item to modify.
      :arg string property…: The property to modify. A property is specified as a
                             sequence of one or more strings. See below for a list
                             of all supported property descriptors.
      :arg value…: The values for the property. The number and types of the expected
                   values depend on the property being modified (see below).

      Supported types and properties are:

      - Resource_
      - Terrain_
      - Worker_
      - Building_
      - Productionsite_
      - Trainingsite_
      - Militarysite_
      - Warehouse_
      - Tribe_

      .. table:: ``"resource"``
         :name: resource
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"max_amount"`                         **amount**         (*int*)               1.0
         ============================================  =======================================  =============

      .. table:: ``"terrain"``
         :name: terrain
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"enhancement"`                        **terrain_name**     (*string*)          1.0
         :const:`"textures"`                           **textures_and_fps** (*table*)           1.1
         ============================================  =======================================  =============

      .. table:: ``"worker"``
         :name: worker
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"experience"`                         **experience**     (*int*)               1.0
         :const:`"becomes"`                            **worker_name**    (*string*)            1.0
         :const:`"programs"`, :const:`"set"`           **program_name**   (*string*),           1.0
                                                       **actions_table**  (*table*)
         ============================================  =======================================  =============

      .. table:: ``"building"``
         :name: building
         :width: 100%
         :widths: 40,50,10
         :align: left

         ==============================================================  =======================================  =============
         Property descriptor                                             Values                                   Since version
         ==============================================================  =======================================  =============
         :const:`"enhancement"`                                          **building_name**  (*string*)            1.1
         :const:`"buildcost"`, :const:`"remove"`                         **ware_name**      (*string*)            1.1
         :const:`"buildcost"`, :const:`"set"`                            **ware_name**      (*string*),           1.1
                                                                         **amount**         (*int*)
         :const:`"return_on_dismantle"`, :const:`"remove"`               **ware_name**      (*string*)            1.1
         :const:`"return_on_dismantle"`, :const:`"set"`                  **ware_name**      (*string*),           1.1
                                                                         **amount**         (*int*)
         :const:`"enhancement_cost"`, :const:`"remove"`                  **ware_name**      (*string*)            1.1
         :const:`"enhancement_cost"`, :const:`"set"`                     **ware_name**      (*string*),           1.1
                                                                         **amount**         (*int*)
         :const:`"enhancement_return_on_dismantle"`, :const:`"remove"`   **ware_name**      (*string*)            1.1
         :const:`"enhancement_return_on_dismantle"`, :const:`"set"`      **ware_name**      (*string*),           1.1
                                                                         **amount**         (*int*)
         ==============================================================  =======================================  =============

      .. table:: ``"productionsite"``
         :name: productionsite
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"input"`, :const:`"add_ware"`         **ware_name**      (*string*),           1.0
                                                       **amount**         (*int*)
         :const:`"input"`, :const:`"modify_ware"`      **ware_name**      (*string*),           1.0
                                                       **new_amount**     (*int*)
         :const:`"input"`, :const:`"remove_ware"`      **ware_name**      (*string*)            1.0
         :const:`"input"`, :const:`"add_worker"`       **worker_name**    (*string*),           1.0
                                                       **amount**         (*int*)
         :const:`"input"`, :const:`"modify_worker"`    **worker_name**    (*string*),           1.0
                                                       **new_amount**     (*int*)
         :const:`"input"`, :const:`"remove_worker"`    **worker_name**    (*string*)            1.0
         :const:`"programs"`, :const:`"set"`           **program_name**   (*string*),           1.0
                                                       **program_table**  (*table*)
         ============================================  =======================================  =============

      .. table:: ``"trainingsite"``
         :name: trainingsite
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"soldier_capacity"`                   **capacity**       (*int*)               1.1
         :const:`"trainer_patience"`                   **patience**       (*int*)               1.1
         ============================================  =======================================  =============

      .. table:: ``"militarysite"``
         :name: militarysite
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"conquers"`                           **radius**         (*int*)               1.1
         :const:`"heal_per_second"`                    **amount**         (*int*)               1.1
         :const:`"max_soldiers"`                       **amount**         (*int*)               1.1
         ============================================  =======================================  =============

      .. table:: ``"warehouse"``
         :name: warehouse
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"heal_per_second"`                    **amount**         (*int*)               1.1
         :const:`"conquers"`                           **radius**         (*int*)               1.1
         ============================================  =======================================  =============

      .. table:: ``"tribe"``
         :name: tribe
         :width: 100%
         :widths: 40,50,10
         :align: left

         ============================================  =======================================  =============
         Property descriptor                           Values                                   Since version
         ============================================  =======================================  =============
         :const:`"add_ware"`                           **ware_name**       (*string*),          1.0
                                                       **menu_column**     (*int*),
                                                       **target_quantity** (*int* or *nil*),
                                                       **preciousness**    (*int*),
                                                       **helptexts**       (*table*)
         :const:`"add_worker"`                         **worker_name**     (*string*),          1.0
                                                       **menu_column**     (*int*),
                                                       **target_quantity** (*int* or *nil*),
                                                       **preciousness**    (*int* or *nil*),
                                                       **helptexts**       (*table*)
         :const:`"add_building"`                       **building_name**   (*string*),          1.0
                                                       **helptexts**       (*table*)
         :const:`"add_immovable"`                      **immovable_name**  (*string*),          1.0
                                                       **helptexts**       (*table*)
         ============================================  =======================================  =============

      Example to add a new worker to an existing tribe; the worker will be appended to the 2nd
      column in the workers displays (stock menu, warehouse window, economy options), and have
      no target quantity or preciousness:

      .. code-block:: lua

         descriptions:modify_unit("tribe", "frisians", "add_worker", "frisians_salter",
               2, nil, nil, { helptexts = { purpose =
                  _("The salter washes salt from the shores of the sea.")
               }})

      Example to add a new input ware to a building and ensure that the programs use it:

      .. code-block:: lua

         -- Add the input
         descriptions:modify_unit("productionsite", "frisians_smokery",
               "input", "add_ware", "salt", 6)

         -- Overwrite the two predefined programs with new ones
         descriptions:modify_unit("productionsite", "frisians_smokery", "programs", "set",
               "smoke_fish", { descname = _("smoking fish"), actions = {
                     "return=skipped unless economy needs smoked_fish",
                     "consume=fish:2 salt log",
                     "sleep=duration:16s",
                     "animate=working duration:30s",
                     "produce=smoked_fish:2"
               }})
         descriptions:modify_unit("productionsite", "frisians_smokery", "programs", "set",
               "smoke_meat", { descname = _("smoking meat"), actions = {
                     "return=skipped when site has fish:2 and economy needs smoked_fish",
                     "return=skipped unless economy needs smoked_meat",
                     "consume=meat:2 salt log",
                     "sleep=duration:16s",
                     "animate=working duration:30s",
                     "produce=smoked_meat:2"
               }})

         -- The main program needs to be overwritten as well – otherwise
         -- the new program definitions will not not applied!
         descriptions:modify_unit("productionsite", "frisians_smokery", "programs", "set",
               "main", { descname = _("working"), actions = {
                     "call=smoke_fish",
                     "call=smoke_meat"
               }})
*/
// clang-format on
int LuaDescriptions::modify_unit(lua_State* L) {
	const std::string type = luaL_checkstring(L, 2);
	const std::string unit = luaL_checkstring(L, 3);
	const std::string property = luaL_checkstring(L, 4);

	// Load the unit to modify if it was not loaded yet
	Notifications::publish(Widelands::NoteMapObjectDescription(
	   unit, Widelands::NoteMapObjectDescription::LoadType::kObject));

	if (type == "resource") {
		do_modify_resource(L, unit, property);
	} else if (type == "tribe") {
		do_modify_tribe(L, unit, property);
	} else if (type == "ship") {
		do_modify_ship(L, unit, property);
	} else if (type == "critter") {
		do_modify_critter(L, unit, property);
	} else if (type == "terrain") {
		do_modify_terrain(L, unit, property);
	} else if (type == "immovable") {
		do_modify_immovable(L, unit, property);
	} else if (type == "worker") {
		do_modify_worker(L, unit, property);
	} else if (type == "ware") {
		do_modify_ware(L, unit, property);
	} else if (type == "building") {
		do_modify_building(L, unit, property);
	} else if (type == "trainingsite") {
		do_modify_trainingsite(L, unit, property);
	} else if (type == "productionsite") {
		do_modify_productionsite(L, unit, property);
	} else if (type == "militarysite") {
		do_modify_militarysite(L, unit, property);
	} else if (type == "warehouse") {
		do_modify_warehouse(L, unit, property);
	} else {
		report_error(L, "modify_unit: invalid type '%s'", type.c_str());
	}

	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

void LuaDescriptions::do_modify_resource(lua_State* L,
                                         const std::string& unit_name,
                                         const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	Widelands::ResourceDescription& resource_descr =
	   *descrs.get_mutable_resource_descr(descrs.safe_resource_index(unit_name));

	if (property == "max_amount") {
		resource_descr.set_max_amount(luaL_checkuint32(L, 5));
	} else {
		report_error(L, "modify_unit: invalid resource property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_tribe(lua_State* L,
                                      const std::string& unit_name,
                                      const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	Widelands::TribeDescr& tribe_descr =
	   *descrs.get_mutable_tribe_descr(descrs.safe_tribe_index(unit_name));

	if (property == "add_ware") {
		const std::string warename = luaL_checkstring(L, 5);
		Notifications::publish(Widelands::NoteMapObjectDescription(
		   warename, Widelands::NoteMapObjectDescription::LoadType::kObject));

		const size_t column = luaL_checkuint32(L, 6);
		const Widelands::DescriptionIndex di = descrs.safe_ware_index(warename);
		tribe_descr.mutable_wares().insert(di);
		tribe_descr.mutable_wares_order()[column].push_back(di);

		Widelands::WareDescr& wdescr = *descrs.get_mutable_ware_descr(di);
		if (!lua_isnil(L, 7)) {
			wdescr.set_default_target_quantity(unit_name, luaL_checkuint32(L, 7));
		}
		if (!lua_isinteger(L, 8)) {
			report_error(L, "Preciousness is mandatory for wares (item: %s)", warename.c_str());
		}
		wdescr.set_preciousness(unit_name, luaL_checkuint32(L, 8));

		LuaTable t(L);
		tribe_descr.load_helptexts(descrs.get_mutable_ware_descr(di), t);

		// Update the player's ware statistics
		iterate_players_existing(p, egbase.map().get_nrplayers(), egbase, player) {
			if (&player->tribe() == &tribe_descr) {
				player->init_statistics();
			}
		}
	} else if (property == "add_worker") {
		const std::string workername = luaL_checkstring(L, 5);
		Notifications::publish(Widelands::NoteMapObjectDescription(
		   workername, Widelands::NoteMapObjectDescription::LoadType::kObject));

		const size_t column = luaL_checkuint32(L, 6);
		const Widelands::DescriptionIndex di = descrs.safe_worker_index(workername);
		tribe_descr.mutable_workers().insert(di);
		tribe_descr.mutable_workers_order()[column].push_back(di);

		Widelands::WorkerDescr& wdescr = *descrs.get_mutable_worker_descr(di);
		if (!lua_isnil(L, 7)) {
			wdescr.set_default_target_quantity(luaL_checkuint32(L, 7));
			if (lua_isnil(L, 8)) {
				report_error(L, "Worker '%s' with target quantity must define a preciousness",
				             workername.c_str());
			}
		}
		if (!lua_isnil(L, 8)) {
			wdescr.set_preciousness(unit_name, luaL_checkuint32(L, 8));
		}

		LuaTable t(L);
		tribe_descr.load_helptexts(descrs.get_mutable_worker_descr(di), t);

		// Update the player's worker statistics
		iterate_players_existing(p, egbase.map().get_nrplayers(), egbase, player) {
			if (&player->tribe() == &tribe_descr) {
				player->init_statistics();
			}
		}
	} else if (property == "add_building") {
		const std::string buildingname = luaL_checkstring(L, 5);
		Notifications::publish(Widelands::NoteMapObjectDescription(
		   buildingname, Widelands::NoteMapObjectDescription::LoadType::kObject));

		const Widelands::DescriptionIndex di = descrs.safe_building_index(buildingname);
		tribe_descr.mutable_buildings().insert(di);

		LuaTable t(L);
		tribe_descr.load_helptexts(descrs.get_mutable_building_descr(di), t);

		// Ensure the new building can be built
		iterate_players_existing(p, egbase.map().get_nrplayers(), egbase, player) {
			if (&player->tribe() == &tribe_descr) {
				player->allow_building_type(di, true);
			}
		}
	} else if (property == "add_immovable") {
		const std::string immo_name = luaL_checkstring(L, 5);
		Notifications::publish(Widelands::NoteMapObjectDescription(
		   immo_name, Widelands::NoteMapObjectDescription::LoadType::kObject));
		tribe_descr.mutable_immovables().insert(descrs.safe_immovable_index(immo_name));

		LuaTable t(L);
		tribe_descr.load_helptexts(
		   descrs.get_mutable_immovable_descr(descrs.safe_immovable_index(immo_name)), t);
	} else {
		report_error(L, "modify_unit: invalid tribe property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_worker(lua_State* L,
                                       const std::string& unit_name,
                                       const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	Widelands::WorkerDescr& worker_descr =
	   *descrs.get_mutable_worker_descr(descrs.safe_worker_index(unit_name));

	if (property == "experience") {
		worker_descr.set_needed_experience(luaL_checkuint32(L, 5));
	} else if (property == "becomes") {
		worker_descr.set_becomes(descrs, luaL_checkstring(L, 5));
	} else if (property == "programs") {
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string prog_name = luaL_checkstring(L, 6);
		if (cmd == "set") {
			LuaTable t(L);
			worker_descr.mutable_programs()[prog_name] = std::unique_ptr<Widelands::WorkerProgram>(
			   new Widelands::WorkerProgram(prog_name, t, worker_descr, descrs));
		} else {
			report_error(L, "modify_unit - worker - programs: invalid command '%s'", cmd.c_str());
		}
	} else {
		report_error(L, "modify_unit: invalid worker property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_building(lua_State* L,
                                         const std::string& unit_name,
                                         const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	const Widelands::DescriptionIndex bldindex = descrs.safe_building_index(unit_name);
	Widelands::BuildingDescr& bld = *descrs.get_mutable_building_descr(bldindex);

	if (property == "enhancement") {
		std::unique_ptr<LuaTable> tbl(new LuaTable(L));
		bld.set_enhancement(descrs, *tbl);
		descrs.get_mutable_building_descr(bld.enhancement())->set_enhanced_from(bldindex);
	} else if (property == "vision_range") {
		bld.set_vision_range(luaL_checkuint32(L, 5));
	} else if (property == "buildcost") {
		Widelands::Buildcost& bc = bld.mutable_buildcost();
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string warename = luaL_checkstring(L, 6);
		const Widelands::DescriptionIndex wi = descrs.safe_ware_index(warename);
		if (cmd == "remove") {
			bc.erase(wi);
		} else if (cmd == "set") {
			bc[wi] = luaL_checkuint32(L, 7);
		} else {
			report_error(L, "modify_unit - building - buildcost: invalid command '%s'", cmd.c_str());
		}
	} else if (property == "return_on_dismantle") {
		Widelands::Buildcost& bc = bld.mutable_returns_on_dismantle();
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string warename = luaL_checkstring(L, 6);
		const Widelands::DescriptionIndex wi = descrs.safe_ware_index(warename);
		if (cmd == "remove") {
			bc.erase(wi);
		} else if (cmd == "set") {
			bc[wi] = luaL_checkuint32(L, 7);
		} else {
			report_error(
			   L, "modify_unit - building - return_on_dismantle: invalid command '%s'", cmd.c_str());
		}
	} else if (property == "enhancement_cost") {
		Widelands::Buildcost& bc = bld.mutable_enhancement_cost();
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string warename = luaL_checkstring(L, 6);
		const Widelands::DescriptionIndex wi = descrs.safe_ware_index(warename);
		if (cmd == "remove") {
			bc.erase(wi);
		} else if (cmd == "set") {
			bc[wi] = luaL_checkuint32(L, 7);
		} else {
			report_error(
			   L, "modify_unit - building - enhancement_cost: invalid command '%s'", cmd.c_str());
		}
	} else if (property == "enhancement_return_on_dismantle") {
		Widelands::Buildcost& bc = bld.mutable_enhancement_returns_on_dismantle();
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string warename = luaL_checkstring(L, 6);
		const Widelands::DescriptionIndex wi = descrs.safe_ware_index(warename);
		if (cmd == "remove") {
			bc.erase(wi);
		} else if (cmd == "set") {
			bc[wi] = luaL_checkuint32(L, 7);
		} else {
			report_error(
			   L, "modify_unit - building - enhancement_return_on_dismantle: invalid command '%s'",
			   cmd.c_str());
		}
	} else {
		report_error(L, "modify_unit: invalid building property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_productionsite(lua_State* L,
                                               const std::string& unit_name,
                                               const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	const Widelands::DescriptionIndex psindex = descrs.safe_building_index(unit_name);
	Widelands::ProductionSiteDescr& psdescr =
	   dynamic_cast<Widelands::ProductionSiteDescr&>(*descrs.get_mutable_building_descr(psindex));

	if (property == "input") {
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string input_name = luaL_checkstring(L, 6);
		Notifications::publish(Widelands::NoteMapObjectDescription(
		   input_name, Widelands::NoteMapObjectDescription::LoadType::kObject));

		if (cmd == "add_ware") {
			const Widelands::DescriptionIndex di = descrs.safe_ware_index(input_name);
			const Widelands::Quantity amount = luaL_checkuint32(L, 7);
			assert(amount);
			psdescr.mutable_input_wares().push_back(Widelands::WareAmount(di, amount));
		} else if (cmd == "modify_ware") {
			const Widelands::DescriptionIndex di = descrs.safe_ware_index(input_name);
			const Widelands::Quantity amount = luaL_checkuint32(L, 7);
			assert(amount);
			for (Widelands::WareAmount& w : psdescr.mutable_input_wares()) {
				if (w.first == di) {
					w.second = amount;
					return;
				}
			}
			report_error(L, "modify_unit - productionsite - input - modify_ware: %s not found",
			             input_name.c_str());
		} else if (cmd == "remove_ware") {
			const Widelands::DescriptionIndex di = descrs.safe_ware_index(input_name);
			for (auto it = psdescr.mutable_input_wares().begin();
			     it != psdescr.mutable_input_wares().end(); ++it) {
				if (it->first == di) {
					psdescr.mutable_input_wares().erase(it);
					return;
				}
			}
			report_error(L, "modify_unit - productionsite - input - remove_ware: %s not found",
			             input_name.c_str());
		} else if (cmd == "add_worker") {
			const Widelands::DescriptionIndex di = descrs.safe_worker_index(input_name);
			const Widelands::Quantity amount = luaL_checkuint32(L, 7);
			assert(amount);
			psdescr.mutable_input_workers().push_back(Widelands::WareAmount(di, amount));
		} else if (cmd == "modify_worker") {
			const Widelands::DescriptionIndex di = descrs.safe_worker_index(input_name);
			const Widelands::Quantity amount = luaL_checkuint32(L, 7);
			assert(amount);
			for (Widelands::WareAmount& w : psdescr.mutable_input_workers()) {
				if (w.first == di) {
					w.second = amount;
					return;
				}
			}
			report_error(L, "modify_unit - productionsite - input - modify_worker: %s not found",
			             input_name.c_str());
		} else if (cmd == "remove_worker") {
			const Widelands::DescriptionIndex di = descrs.safe_worker_index(input_name);
			for (auto it = psdescr.mutable_input_workers().begin();
			     it != psdescr.mutable_input_workers().end(); ++it) {
				if (it->first == di) {
					psdescr.mutable_input_workers().erase(it);
					return;
				}
			}
			report_error(L, "modify_unit - productionsite - input - remove_worker: %s not found",
			             input_name.c_str());
		} else {
			report_error(L, "modify_unit - productionsite - input: invalid command '%s'", cmd.c_str());
		}
	} else if (property == "programs") {
		const std::string cmd = luaL_checkstring(L, 5);
		const std::string prog_name = luaL_checkstring(L, 6);
		if (cmd == "set") {
			std::unique_ptr<LuaTable> tbl(new LuaTable(L));
			psdescr.mutable_programs()[prog_name] = std::unique_ptr<Widelands::ProductionProgram>(
			   new Widelands::ProductionProgram(prog_name, *tbl, descrs, &psdescr));
		} else {
			report_error(
			   L, "modify_unit - productionsite - programs: invalid command '%s'", cmd.c_str());
		}
	} else if (property == "enhancement") {
		// Needs to stay around for backward compatibility
		return do_modify_building(L, unit_name, property);
	} else {
		report_error(L, "modify_unit: invalid productionsite property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_ship(lua_State* L, const std::string&, const std::string&) {
	report_error(L, "modify_unit for ships not yet supported");
}

void LuaDescriptions::do_modify_critter(lua_State* L, const std::string&, const std::string&) {
	report_error(L, "modify_unit for critters not yet supported");
}

void LuaDescriptions::do_modify_terrain(lua_State* L,
                                        const std::string& unit_name,
                                        const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	Widelands::TerrainDescription& terrain =
	   *descrs.get_mutable_terrain_descr(descrs.safe_terrain_index(unit_name));

	if (property == "enhancement") {
		terrain.set_enhancement(luaL_checkstring(L, 5), luaL_checkstring(L, 6));
	} else if (property == "textures") {
		LuaTable t(L);
		terrain.replace_textures(t);
	} else {
		report_error(L, "modify_unit: invalid terrain property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_immovable(lua_State* L, const std::string&, const std::string&) {
	report_error(L, "modify_unit for immovables not yet supported");
}

void LuaDescriptions::do_modify_ware(lua_State* L, const std::string&, const std::string&) {
	report_error(L, "modify_unit for wares not yet supported");
}

void LuaDescriptions::do_modify_trainingsite(lua_State* L,
                                             const std::string& unit_name,
                                             const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	const Widelands::DescriptionIndex tsindex = descrs.safe_building_index(unit_name);
	Widelands::TrainingSiteDescr& tsdescr =
	   dynamic_cast<Widelands::TrainingSiteDescr&>(*descrs.get_mutable_building_descr(tsindex));

	if (property == "soldier_capacity") {
		tsdescr.set_max_number_of_soldiers(luaL_checkuint32(L, 5));
	} else if (property == "trainer_patience") {
		tsdescr.set_max_stall(luaL_checkuint32(L, 5));
	} else {
		report_error(L, "modify_unit: invalid trainingsite property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_militarysite(lua_State* L,
                                             const std::string& unit_name,
                                             const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	const Widelands::DescriptionIndex msindex = descrs.safe_building_index(unit_name);
	Widelands::MilitarySiteDescr& msdescr =
	   dynamic_cast<Widelands::MilitarySiteDescr&>(*descrs.get_mutable_building_descr(msindex));

	if (property == "conquers") {
		msdescr.set_conquers(luaL_checkuint32(L, 5));
	} else if (property == "heal_per_second") {
		msdescr.set_heal_per_second(luaL_checkuint32(L, 5));
	} else if (property == "max_soldiers") {
		msdescr.set_max_number_of_soldiers(luaL_checkuint32(L, 5));
	} else {
		report_error(L, "modify_unit: invalid militarysite property '%s'", property.c_str());
	}
}

void LuaDescriptions::do_modify_warehouse(lua_State* L,
                                          const std::string& unit_name,
                                          const std::string& property) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Descriptions& descrs = *egbase.mutable_descriptions();
	const Widelands::DescriptionIndex whindex = descrs.safe_building_index(unit_name);
	Widelands::WarehouseDescr& whdescr =
	   dynamic_cast<Widelands::WarehouseDescr&>(*descrs.get_mutable_building_descr(whindex));

	if (property == "conquers") {
		whdescr.set_conquers(luaL_checkuint32(L, 5));
	} else if (property == "heal_per_second") {
		whdescr.set_heal_per_second(luaL_checkuint32(L, 5));
	} else {
		report_error(L, "modify_unit: invalid warehouse property '%s'", property.c_str());
	}
}

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

		// TODO(GunChleoc): These 2 classes are only here for savegame compatibility
		register_class<LuaWorld>(L, "", false);
		register_class<LuaTribes>(L, "", false);
	}
	register_class<LuaDescriptions>(L, "", false);
}

const char LuaWorld::className[] = "World";
const MethodType<LuaWorld> LuaWorld::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaWorld> LuaWorld::Properties[] = {
   {nullptr, nullptr, nullptr},
};
const char LuaTribes::className[] = "Tribes";
const MethodType<LuaTribes> LuaTribes::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTribes> LuaTribes::Properties[] = {
   {nullptr, nullptr, nullptr},
};

}  // namespace LuaRoot
