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

#include "gamecontroller.h"
#include "log.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/findimmovable.h"
#include "logic/game.h"
#include "logic/immovable.h"
#include "logic/tribe.h"

#include "lua_map.h"
#include "coroutine_impl.h"

#include "lua_root.h"

using namespace Widelands;

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
	METHOD(L_Map, place_immovable),
	METHOD(L_Map, get_field),
	{0, 0},
};
const PropertyType<L_Map> L_Map::Properties[] = {
	PROP_RO(L_Map, width),
	PROP_RO(L_Map, height),
	PROP_RO(L_Map, player_slots),
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

/* RST
	.. attribute:: player_slots

		(RO) This is an :class:`array` that contains :class:`~wl.map.PlayerSlots`
		for each player defined in the map.
*/
int L_Map::get_player_slots(lua_State * L) {
	Map & m = get_egbase(L).map();

	lua_createtable(L, m.get_nrplayers(), 0);
	for(uint32_t i = 0; i < m.get_nrplayers(); i++) {
		lua_pushuint32(L, i+1);
		to_lua<L_PlayerSlot>(L, new L_PlayerSlot(i+1));
		lua_settable(L, -3);
	}

	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
	.. method:: place_immovable(name, field[, from_where = "world"])

		Creates an immovable that is defined by the world (e.g. trees, stones...)
		or a tribe (field) on a given field. If there is already an immovable on
		the field, an error is reported.

		:arg name: The name of the immovable to create
		:type name: :class:`string`
		:arg field: The immovable is created on this field.
		:type field: :class:`wl.map.Field`
		:arg from_where: a tribe name or "world" that defines where the immovable
			is defined
		:type from_where: :class:`string`

		:returns: The created immovable.
*/
int L_Map::place_immovable(lua_State * const L) {
	std::string from_where = "world";

	char const * const objname = luaL_checkstring(L, 2);
	L_Field * c = *get_user_class<L_Field>(L, 3);
	if (lua_gettop(L) > 3 and not lua_isnil(L, 4))
		from_where = luaL_checkstring(L, 4);

	// Check if the map is still free here
	if
	 (BaseImmovable const * const imm = c->fcoords(L).field->get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(L, "Node is no longer free!");

	Editor_Game_Base & egbase = get_egbase(L);

	BaseImmovable * m = 0;
	if (from_where != "world") {
		try {
			Widelands::Tribe_Descr const & tribe =
				egbase.manually_load_tribe(from_where);

			int32_t const imm_idx = tribe.get_immovable_index(objname);
			if (imm_idx < 0)
				return
					report_error
					(L, "Unknown immovable <%s> for tribe <%s>",
					 objname, from_where.c_str());

			m = &egbase.create_immovable(c->coords(), imm_idx, &tribe);
		} catch (game_data_error & gd) {
			return
				report_error
					(L, "Problem loading tribe <%s>. Maybe not existent?",
					 from_where.c_str());
		}
	} else {
		int32_t const imm_idx = egbase.map().world().get_immovable_index(objname);
		if (imm_idx < 0)
			return report_error(L, "Unknown immovable <%s>", objname);

		m = &egbase.create_immovable(c->coords(), imm_idx, 0);
	}

	return upcasted_immovable_to_lua(L, m);
}

/* RST
	.. method:: get_field(x_or_table[, y])

		Returns a :class:`wl.map.Field` object of the given index.
		The function either takes two arguments: the x and y index
		of the field or a :class:`table` as argument. If you pass in a table, it
		will first be checked for the fields :const:`x` and :const:`y`, then for
		the index 1 and 2.

		:returns: :const:`nil`
*/
int L_Map::get_field(lua_State * L) {
	uint32_t x = 4294967295; // 2^32 - 1
	uint32_t y = 4294967295; // 2^32 - 1

	if (lua_gettop(L) == 3) { // x, y arguments
		x = luaL_checkuint32(L, 2);
		y = luaL_checkuint32(L, 3);
	} else {
		luaL_checktype(L, 2, LUA_TTABLE);

		lua_getfield(L, 2, "x");
		if lua_isnil(L, -1) {
			lua_pop(L, 1); // pop the nil
			lua_pushuint32(L, 1);
			lua_gettable(L, 2);
			if lua_isnil(L, -1)
				return report_error(L, "No 'x' or first key in table");
		}
		x = luaL_checkuint32(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 2, "y");
		if lua_isnil(L, -1) {
			lua_pop(L, 1); // pop the nil
			lua_pushuint32(L, 2);
			lua_gettable(L, 2);
			if lua_isnil(L, -1)
				return report_error(L, "No 'y' or second key in table");
		}
		y = luaL_checkuint32(L, -1);
		lua_pop(L, 1);
	}

	Map & m = get_egbase(L).map();
	if (x >= static_cast<uint32_t>(m.get_width()))
		report_error(L, "x coordinate out of range!");
	if (y >= static_cast<uint32_t>(m.get_height()))
		report_error(L, "y coordinate out of range!");

	return to_lua<L_Field>(L, new L_Field(x, y));
}


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

