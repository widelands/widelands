/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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

#include <boost/format.hpp>

#include "logic/cmd_luacoroutine.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/world.h"
#include "scripting/globals.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_editor.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"
#include "scripting/lua_table.h"

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
   METHOD(LuaGame, launch_coroutine), METHOD(LuaGame, save), {nullptr, nullptr},
};
const PropertyType<LuaGame> LuaGame::Properties[] = {
   PROP_RO(LuaGame, real_speed),    PROP_RO(LuaGame, time),
   PROP_RW(LuaGame, desired_speed), PROP_RW(LuaGame, allow_autosaving),
   PROP_RW(LuaGame, allow_saving),  {nullptr, nullptr, nullptr},
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
int LuaGame::set_allow_autosaving(lua_State* L) {
	// WAS_DEPRECATED_BEFORE(build18), use allow_saving
	get_game(L).save_handler().set_allow_saving(luaL_checkboolean(L, -1));
	return 0;
}
int LuaGame::get_allow_autosaving(lua_State* L) {
	// WAS_DEPRECATED_BEFORE(build18), use allow_saving
	lua_pushboolean(L, get_game(L).save_handler().get_allow_saving());
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
	if (nargs < 2)
		report_error(L, "Too few arguments!");
	if (nargs == 3) {
		runtime = luaL_checkuint32(L, 3);
		lua_pop(L, 1);
	}

	std::unique_ptr<LuaCoroutine> cr(new LuaCoroutine(luaL_checkthread(L, 2)));
	lua_pop(L, 2);  // Remove coroutine and Game object from stack

	get_game(L).enqueue_command(
	   new Widelands::CmdLuaCoroutine(runtime, std::move(cr)));

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

   This offers access to the objects in a the widelands world and allows to add
   new objects.
*/

const char LuaWorld::className[] = "World";
const MethodType<LuaWorld> LuaWorld::Methods[] = {
   METHOD(LuaWorld, new_critter_type),
   METHOD(LuaWorld, new_editor_immovable_category),
   METHOD(LuaWorld, new_editor_terrain_category),
   METHOD(LuaWorld, new_immovable_type),
   METHOD(LuaWorld, new_resource_type),
   METHOD(LuaWorld, new_terrain_type),
   {0, 0},
};
const PropertyType<LuaWorld> LuaWorld::Properties[] = {
   PROP_RO(LuaWorld, immovable_descriptions), PROP_RO(LuaWorld, terrain_descriptions), {0, 0, 0},
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

/* RST
   .. method:: new_resource_type(table)

      Adds a new resource type that can be in the different maps. Takes a
      single argument, a table with the descriptions for the resource type. See the
      files in data/world/ for usage examples.

      :returns: :const:`nil`
*/
int LuaWorld::new_resource_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_world()->add_resource_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}

	return 0;
}

/* RST
   .. method:: new_terrain_type(table)

      Adds a new terrain type that can be used in maps. Takes a single
      argument, a table with the descriptions for the terrain type. See the files in data/world/
      for usage examples.

      :returns: :const:`nil`
*/
int LuaWorld::new_terrain_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_world()->add_terrain_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}

	return 0;
}

/* RST
   .. method:: new_critter_type(table)

      Adds a new critter type that can be used in maps. Takes a single
      argument, a table with the description. See the files in data/world/ for usage
      examples.

      :returns: :const:`nil`
*/
int LuaWorld::new_critter_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);
		get_egbase(L).mutable_world()->add_critter_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_immovable_type(table)

      Adds a new immovable type that can be used in maps. Takes a single
      argument, a table with the description. See the files in data/world/ for usage
      examples.

      :returns: :const:`nil`
*/
int LuaWorld::new_immovable_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);
		get_egbase(L).mutable_world()->add_immovable_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_editor_terrain_category(table)

      Adds a new editor category that can be used to classify objects in the
      world. This will be used to sort them into sub menus in the editor. See
      usage examples in data/world/.

      :returns: :const:`nil`
*/
int LuaWorld::new_editor_terrain_category(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);
		get_egbase(L).mutable_world()->add_editor_terrain_category(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_editor_immovable_category(table)

      Like :func:`new_editor_terrain_category`, but for immovables.

      :returns: :const:`nil`
*/
int LuaWorld::new_editor_immovable_category(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	try {
		LuaTable table(L);
		get_egbase(L).mutable_world()->add_editor_immovable_category(table);
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
   new objects.
*/

const char LuaTribes::className[] = "Tribes";
const MethodType<LuaTribes> LuaTribes::Methods[] = {
   METHOD(LuaTribes, new_constructionsite_type),
   METHOD(LuaTribes, new_dismantlesite_type),
   METHOD(LuaTribes, new_militarysite_type),
   METHOD(LuaTribes, new_productionsite_type),
   METHOD(LuaTribes, new_trainingsite_type),
   METHOD(LuaTribes, new_warehouse_type),
   METHOD(LuaTribes, new_immovable_type),
   METHOD(LuaTribes, new_ship_type),
   METHOD(LuaTribes, new_ware_type),
   METHOD(LuaTribes, new_carrier_type),
   METHOD(LuaTribes, new_soldier_type),
   METHOD(LuaTribes, new_worker_type),
   METHOD(LuaTribes, new_tribe),
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
   .. method:: new_constructionsite_type(table)

      Adds a new constructionsite building type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_constructionsite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_constructionsite_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_dismantlesite_type(table)

      Adds a new disnamtlesite building type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_dismantlesite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_dismantlesite_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_militarysite_type(table)

      Adds a new militarysite building type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_militarysite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_militarysite_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_productionsite_type(table)

      Adds a new productionsite building type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_productionsite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_productionsite_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_trainingsite_type(table)

      Adds a new trainingsite building type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_trainingsite_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_trainingsite_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_warehouse_type(table)

      Adds a new warehouse building type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_warehouse_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_warehouse_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_immovable_type(table)

      Adds a new immovable type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_immovable_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_tribes()->add_immovable_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_ship_type(table)

      Adds a new ship type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_ship_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_tribes()->add_ship_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_ware_type(table)

      Adds a new ware type. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_ware_type(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		get_egbase(L).mutable_tribes()->add_ware_type(table);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_carrier_type(table)

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
		egbase.mutable_tribes()->add_carrier_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_soldier_type(table)

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
		egbase.mutable_tribes()->add_soldier_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_worker_type(table)

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
		egbase.mutable_tribes()->add_worker_type(table, egbase);
	} catch (std::exception& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. method:: new_tribe(table)

      Adds a new tribe. Takes a single argument, a table with
      the descriptions. See the files in tribes/ for usage examples.

      :returns: :const:`nil`
*/
int LuaTribes::new_tribe(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}

	try {
		LuaTable table(L);  // Will pop the table eventually.
		EditorGameBase& egbase = get_egbase(L);
		egbase.mutable_tribes()->add_tribe(table, egbase);
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
}
