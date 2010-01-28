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

#include <lua.hpp>

#include "logic/cmd_luafunction.h"
#include "economy/flag.h"

#include "c_utils.h"
#include "coroutine_impl.h"
#include "lua_map.h"
#include "scripting.h"

#include "lua_game.h"

using namespace Widelands;


/* RST
:mod:`wl.game`
======================

.. module:: wl.game
   :synopsis: Provides access on game internals like Players

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.game
*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */
/* RST
Player
------

.. class:: Player(n)

	This class represents one of the players in the game. You can access
	information about this player or act on his behalf.

	:arg n: player number, range is 1 - :const:`number of players`
	:type n: Integer
*/
const char L_Player::className[] = "Player";
const MethodType<L_Player> L_Player::Methods[] = {
	METHOD(L_Player, place_flag),
	METHOD(L_Player, place_building),
	METHOD(L_Player, send_message),
	METHOD(L_Player, sees_field),
	{0, 0},
};
const PropertyType<L_Player> L_Player::Properties[] = {
	PROP_RO(L_Player, number),
	{0, 0, 0},
};

void L_Player::__persist(lua_State * L) {
	PERS_UINT32("player", m_pl);
}
void L_Player::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", m_pl);
}


/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: number

		(RO) The number of this Player.
*/
int L_Player::get_number(lua_State * L) {
	lua_pushuint32(L, m_pl);
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
	.. function:: place_flag(field[, force])

		Builds a flag at a given position if it is legal to do so. If not,
		reports an error

		:arg field: where the flag should be created
		:type field: :class:`wl.map.Field`
		:arg force: If this is :const:`true` then the map is created with
			pure force:

				* if there is an immovable on this field, it will be
				  removed
				* if there are flags too close by to this field, they will be
				  ripped
				* if the player does not own the territory, it is conquered
				  for him.
		:type force: :class:`boolean`
		:returns: :class:`wl.map.Flag` object created or :const:`nil`.
*/
int L_Player::place_flag(lua_State * L) {
	uint32_t n = lua_gettop(L);
	L_Field * c = *get_user_class<L_Field>(L, 2);
	bool force = false;
	if (n > 2)
		force = luaL_checkboolean(L, 3);

	Flag * f;
	if (not force) {
		f = m_get(get_game(L)).build_flag(c->coords());
		if (!f)
			return report_error(L, "Couldn't build flag!");
	} else {
		f = &m_get(get_game(L)).force_flag(c->coords());
	}

	return to_lua<L_Flag>(L, new L_Flag(*f));
}

/* RST
	.. method:: place_building(name, pos)

		Immediately creates a building at the given position. No construction
		site is created. The building starts out completely empty.  The building
		is forced to be at this position, the same action is taken as for
		:meth:`place_flag` when force is :const:`true`. Additionally, all
		buildings that are too close to the new one are ripped.

		:returns: :class:`wl.map.Building` object created
*/
int L_Player::place_building(lua_State * L) {
	const char * name = luaL_checkstring(L, 2);
	L_Field * c = *get_user_class<L_Field>(L, 3);

	Building_Index i = m_get(get_game(L)).tribe().building_index(name);

	Building & b = m_get(get_game(L)).force_building
		(c->coords(), i, 0, 0, Soldier_Counts());

	return upcasted_immovable_to_lua(L, &b);
}

/* RST
	.. method:: send_message(t, m[, opts])

		Send a message to the player, the message will
		appear in his inbox. Title or Message can be a
		formatted using wideland's rich text.

		:arg t: title of the message
		:type t: :class:`string`

		:arg m: text of the message
		:type m: :class:`string`

		Opts is a table of optional arguments and can be omitted. If it
		exist it must contain string/value pairs of the following type:

		:arg expires_in: if this is given, the message will be removed
			from the players inbox after this many ms. Default:
			message never expires.
		:type expires_in: :class:`integer`

		:arg loc: this field is the location connected to this message. Default:
			no location attached to message
		:type loc: :class:`wl.map.Field`

		:arg status: status to attach to this message. can be 'new', 'read' or
			'archived'. Default: "new"
		:type status: :class:`string`

		:arg sender: sender name of this string. Default: "LuaEngine"
		:type sender: :class:`string`

		:arg popup: should the message window be opened for this message or not.
			Default: :const:`false`
		:type popup: :class:`boolean`

		:returns: :const:`nil`
*/
// TODO: add tests for this function
int L_Player::send_message(lua_State * L) {
	uint32_t n = lua_gettop(L);
	std::string title = luaL_checkstring(L, 2);
	std::string body = luaL_checkstring(L, 3);
	Coords c = Coords::Null();
	Duration d = Forever();
	Message::Status st = Message::New;
	std::string sender = "LuaEngine";
	bool popup = true;

	if (n == 4) {
		// Optional arguments
		lua_getfield(L, 4, "expires_in");
		if (not lua_isnil(L, -1))
			d = luaL_checkuint32(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 4, "loc");
		if (not lua_isnil(L, -1))
			c = (*get_user_class<L_Field>(L, -1))->coords();
		lua_pop(L, 1);

		lua_getfield(L, 4, "status");
		if (not lua_isnil(L, -1)) {
			std::string s = luaL_checkstring(L, -1);
			if (s == "new") st = Message::New;
			else if (s == "read") st = Message::Read;
			else if (s == "archive") st = Message::Archived;
			else report_error(L, "Unknown message status: %s", s.c_str());
		}
		lua_pop(L, 1);

		lua_getfield(L, 4, "sender");
		if (not lua_isnil(L, -1))
			sender = luaL_checkstring(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, 4, "popup");
		if (not lua_isnil(L, -1))
			popup = luaL_checkboolean(L, -1);
		lua_pop(L, 1);
	}

	Game & game = get_game(L);
	Player & plr = m_get(game);

	Message_Id const message =
		plr.add_message
			(game,
			 *new Message
			 	(sender,
			 	 game.get_gametime(),
			 	 d,
			 	 title,
			 	 body,
				 c,
				 st),
			popup);

	return 0;
}

// TODO: document me, test me
int L_Player::sees_field(lua_State * L) {
	FCoords & c = (*get_user_class<L_Field>(L, 2))->coords();
	Game & game = get_game(L);
	Map & map = game.map();
	Player & p = m_get(game);

	Widelands::Map_Index const i = c.field - &map[0];

	lua_pushboolean(L, p.vision(i) > 1);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */



/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */
/*
 * TODO: document me
 */
static int L_get_time(lua_State * L) {
	Game & game = get_game(L);
	lua_pushint32(L, game.get_gametime());
	return 1;
}


/*
 * TODO: document me
 */
static int L_run_coroutine(lua_State * L) {
	int nargs = lua_gettop(L);
	if (nargs < 1)
		report_error(L, "Too little arguments to run_at");

	LuaCoroutine * cr = new LuaCoroutine_Impl(luaL_checkthread(L, -1));
	Game & game = get_game(L);

	lua_pop(L, 1); // Remove coroutine from stack

	game.enqueue_command
		(new Widelands::Cmd_LuaFunction(game.get_gametime(), cr));

	return 0;
}

const static struct luaL_reg wlgame [] = {
	{"run_coroutine", &L_run_coroutine},
	{"get_time", &L_get_time},
	{0, 0}
};

void luaopen_wlgame(lua_State * L) {
	luaL_register(L, "wl.game", wlgame);
	lua_pop(L, 1); // pop the table

	register_class<L_Player>(L, "game");
}

