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

#include "lua_root.h"


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
	{0, 0},
};
const PropertyType<L_Game> L_Game::Properties[] = {
	PROP_RO(L_Game, time),
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

	(RO) This returns the absolute time elapsed since the game was started in
	milliseconds.
*/
int L_Game::get_time(lua_State * L) {
	lua_pushint32(L, get_game(L).get_gametime());
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
}

