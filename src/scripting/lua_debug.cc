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

#include "lua_debug.h"

#include "c_utils.h"
#include "log.h"
#include "logic/game.h"
#include "logic/player.h"
#include "wui/interactive_player.h"


/* RST
:mod:`wl.debug`
==================

.. module:: wl.debug
   :synopsis: Debugging functionality (only when widelands was build with DEBUG)

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.debug

The :mod:`wl.debug` module contains functionality that can be useful for
developers or scenario creators. It allows to access some internals of
widelands. It is only included in debug builds of widelands (not in release
builds).
*/

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */
/* RST
.. function:: set_see_all(b)

	Toggles the set see all flag for the interactive player (the human
	player). This will either completely hide the map or completely uncover
	it.

	:arg b: either :const:`true` (reveal) or :const:`false` (hide) the map
	:type b: :class:`boolean`
	:returns: :const:`nil`
*/
static int L_set_see_all(lua_State * const l) {
	bool const bval = luaL_checkboolean(l, 1);

	get_game(l).get_ipl()->player().set_see_all(bval);

	return 0;
}

/* RST
.. function:: exit

	Immediately exits the current game, this is equivalent to the user
	clicking the exit button in the in-game main menu. This is especially
	useful for automated testing of features and is for example used in the
	widelands lua test suite.

	:returns: :const:`nil`
*/
static int L_exit(lua_State * const l) {
	get_game(l).get_ipl()->end_modal(0);

	return 0;
}

const static struct luaL_reg wldebug [] = {
	{"set_see_all", &L_set_see_all},
	{"exit", &L_exit},
	{0, 0}
};


void luaopen_wldebug(lua_State * const l) {
	luaL_register(l, "wl.debug", wldebug);

	lua_pop(l, 1); // pop the table from the stack again
}

