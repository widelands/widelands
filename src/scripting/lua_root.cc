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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "logic/game.h"
#include "logic/cmd_luacoroutine.h"
#include "gamecontroller.h"
#include "log.h"

#include "lua_root.h"
#include "coroutine_impl.h"


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

.. class:: Game()

	The root class to access the game internals. You can
	construct as many instances of this class as you like, but
	they all will access the one game currently running.
*/
const char L_Game::className[] = "Game";
const MethodType<L_Game> L_Game::Methods[] = {
	METHOD(L_Game, launch_coroutine),
	{0, 0},
};
const PropertyType<L_Game> L_Game::Properties[] = {
	PROP_RO(L_Game, time),
	PROP_RW(L_Game, desired_speed),
	PROP_RW(L_Game, allow_autosaving),
	{0, 0, 0},
};

L_Game::L_Game(lua_State * L) {
	// Nothing to do.
}

void L_Game::__persist(lua_State * L) {
}
void L_Game::__unpersist(lua_State * L) {
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
	1000 means the game runs at 1x speed. Note that this will not work in
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
	.. attribute:: allow_autosaving

		(RW) Disable or enable auto-saving. When you show off UI features in a
		tutorial or scenario, you have to disallow auto-saving because UI
		elements can not be saved and therefore reloading a game saved in the
		meantime would crash the game.
*/
// UNTESTED
int L_Game::set_allow_autosaving(lua_State * L) {
	get_game(L).save_handler().set_allow_autosaving
		(luaL_checkboolean(L, -1));
	return 0;
}
// UNTESTED
int L_Game::get_allow_autosaving(lua_State * L) {
	lua_pushboolean(L, get_game(L).save_handler().get_allow_autosaving());
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

	LuaCoroutine * cr = new LuaCoroutine_Impl(luaL_checkthread(L, 2));
	lua_pop(L, 2); // Remove coroutine and Game object from stack

	get_game(L).enqueue_command(new Widelands::Cmd_LuaCoroutine(runtime, cr));

	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Map
---

.. class:: Map()

	Access to the map and it's objects. You can construct as many instances of
	this class as you like, but they all will access the same internal object.
*/
const char L_Map::className[] = "Map";
const MethodType<L_Map> L_Map::Methods[] = {
	{0, 0},
};
const PropertyType<L_Map> L_Map::Properties[] = {
	PROP_RO(L_Map, width),
	PROP_RO(L_Map, height),
	{0, 0, 0},
};

L_Map::L_Map(lua_State * L) {
	// Nothing to do.
}

void L_Map::__persist(lua_State * L) {
}
void L_Map::__unpersist(lua_State * L) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: width

		(RO) The width of the map in fields.
*/
int L_Map::get_width(lua_State * L) {
	lua_pushuint32(L, get_egbase(L).map().get_width());
	return 1;
}
/* RST
	.. attribute:: height

		(RO) The height of the map in fields.
*/
int L_Map::get_height(lua_State * L) {
	lua_pushuint32(L, get_egbase(L).map().get_height());
	return 1;
}

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


const static struct luaL_reg wlroot [] = {
	{0, 0}
};

void luaopen_wlroot(lua_State * L) {
	luaL_register(L, "wl", wlroot);
	lua_pop(L, 1); // pop the table

	register_class<L_Game>(L, "");
	register_class<L_Map>(L, "");
}

