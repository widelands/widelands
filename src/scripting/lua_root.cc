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

#include "lua_game.h"
#include "lua_editor.h"
#include "lua_map.h"
#include "coroutine_impl.h"

#include "lua_root.h"

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
	{0, 0},
};
const PropertyType<L_Game> L_Game::Properties[] = {
	PROP_RO(L_Game, time),
	PROP_RW(L_Game, desired_speed),
	PROP_RW(L_Game, allow_autosaving),
	PROP_RO(L_Game, players),
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

/* RST
	.. attribute:: players

		(RO) An :class:`array` with the players currently in the game. Note that
		this might be less then defined in :attr:`wl.map.Map.player_slots` because
		some slots might not be taken. Also note that for the same reason you
		cannot index this array with :attr:`wl.bases.PlayerBase.number`, but the
		players are ordered with increasing number in this array.
*/
int L_Game::get_players(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);

	lua_newtable(L);

	uint32_t idx = 1;
	for(Player_Number i = 1; i <= MAX_PLAYERS; i++) {
		Player * rv = egbase.get_player(i);
		if (not rv)
			continue;

		lua_pushuint32(L, idx++);
		to_lua<LuaGame::L_Player>(L, new LuaGame::L_Player(i));
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

/* RST
	.. method:: save(name)

		Saves the game exactly as if the player had entered the save dialog and
		entered name as an argument. If some error occurred while saving, this
		will throw an Lua error. Note that this currently doesn't work when
		called from inside a Coroutine.

		:arg name: name of save game. If this game already exists, it will be
			silently overwritten
		:type name: :class:`string`
		:returns: :const:`nil`
*/
int L_Game::save(lua_State * const L) {
	Widelands::Game & game = get_game(L);

	std::string const complete_filename =
		game.save_handler().create_file_name
			(SaveHandler::get_base_dir(), luaL_checkstring(L, -1));

	lua_pop(L, 2); // Make stack empty before persistence starts.

	if (g_fs->FileExists(complete_filename))
		g_fs->Unlink(complete_filename);
	std::string error;
	if (!game.save_handler().save_game(game, complete_filename, &error))
		return report_error(L, "save error: %s", error.c_str());

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
	{0, 0},
};
const PropertyType<L_Editor> L_Editor::Properties[] = {
	PROP_RO(L_Editor, players),
	{0, 0, 0},
};

L_Editor::L_Editor(lua_State * L) {
	// Nothing to do.
}

void L_Editor::__persist(lua_State * L) {
}
void L_Editor::__unpersist(lua_State * L) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: players

		(RO) An :class:`array` with the players defined on the current map.
		The editor always creates all players that are defined by the map.
*/
// TODO: this is too similar to L_Game::get_players.
int L_Editor::get_players(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);

	lua_newtable(L);

	uint32_t idx = 1;
	for(Player_Number i = 1; i <= MAX_PLAYERS; i++) {
		Player * rv = egbase.get_player(i);
		if (not rv)
			continue;

		lua_pushuint32(L, idx++);
		to_lua<LuaEditor::L_Player>(L, new LuaEditor::L_Player(i));
		lua_settable(L, -3);
	}
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

void luaopen_wlroot(lua_State * L, bool in_editor) {
	luaL_register(L, "wl", wlroot);
	lua_pop(L, 1); // pop the table

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

