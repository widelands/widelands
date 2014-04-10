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

#include "gamecontroller.h"
#include "log.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/immovable.h"
#include "logic/tribe.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_editor.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"

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
const char L_Game::className[] = "Game";
const MethodType<L_Game> L_Game::Methods[] = {
	METHOD(L_Game, launch_coroutine),
	METHOD(L_Game, save),
	{nullptr, nullptr},
};
const PropertyType<L_Game> L_Game::Properties[] = {
	PROP_RO(L_Game, time),
	PROP_RW(L_Game, desired_speed),
	PROP_RW(L_Game, allow_autosaving),
	PROP_RW(L_Game, allow_saving),
	{nullptr, nullptr, nullptr},
};

L_Game::L_Game(lua_State * /* L */) {
	// Nothing to do.
}

void L_Game::__persist(lua_State * /* L */) {
}
void L_Game::__unpersist(lua_State * /* L */) {
}


/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
	.. attribute:: time

	(RO) The absolute time elapsed since the game was started in milliseconds.
*/
int L_Game::get_time(lua_State * L) {
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
int L_Game::set_desired_speed(lua_State * L) {
	get_game(L).gameController()->setDesiredSpeed(luaL_checkuint32(L, -1));
	return 1;
}
// UNTESTED
int L_Game::get_desired_speed(lua_State * L) {
	lua_pushuint32(L, get_game(L).gameController()->desiredSpeed());
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
int L_Game::set_allow_saving(lua_State * L) {
	get_game(L).save_handler().set_allow_saving
		(luaL_checkboolean(L, -1));
	return 0;
}
// UNTESTED
int L_Game::get_allow_saving(lua_State * L) {
	lua_pushboolean(L, get_game(L).save_handler().get_allow_saving());
	return 1;
}
int L_Game::set_allow_autosaving(lua_State * L) {
	// WAS_DEPRECATED_BEFORE(build18), use allow_saving
	get_game(L).save_handler().set_allow_saving
		(luaL_checkboolean(L, -1));
	return 0;
}
int L_Game::get_allow_autosaving(lua_State * L) {
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
int L_Game::launch_coroutine(lua_State * L) {
	int nargs = lua_gettop(L);
	uint32_t runtime = get_game(L).get_gametime();
	if (nargs < 2)
		report_error(L, "Too little arguments!");
	if (nargs == 3) {
		runtime = luaL_checkuint32(L, 3);
		lua_pop(L, 1);
	}

	LuaCoroutine * cr = new LuaCoroutine(luaL_checkthread(L, 2));
	lua_pop(L, 2); // Remove coroutine and Game object from stack

	get_game(L).enqueue_command(new Widelands::Cmd_LuaCoroutine(runtime, cr));

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
int L_Game::save(lua_State * L) {
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

const char L_Editor::className[] = "Editor";
const MethodType<L_Editor> L_Editor::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_Editor> L_Editor::Properties[] = {
	{nullptr, nullptr, nullptr},
};

L_Editor::L_Editor(lua_State * /* L */) {
	// Nothing to do.
}

void L_Editor::__persist(lua_State * /* L */) {
}
void L_Editor::__unpersist(lua_State * /* L */) {
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

const static struct luaL_Reg wlroot [] = {
	{nullptr, nullptr}
};

void luaopen_wlroot(lua_State * L, bool in_editor) {
	lua_getglobal(L, "wl");  // S: wl
	luaL_setfuncs(L, wlroot, 0); // S: wl
	lua_pop(L, 1);  // S:

	if (in_editor) {
		register_class<L_Editor>(L, "", true);
		add_parent<L_Editor, LuaBases::L_EditorGameBase>(L);
		lua_pop(L, 1); // Pop the meta table
	} else {
		register_class<L_Game>(L, "", true);
		add_parent<L_Game, LuaBases::L_EditorGameBase>(L);
		lua_pop(L, 1); // Pop the meta table
	}
}

};
