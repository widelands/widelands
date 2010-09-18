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

#include "logic/player.h"

#include "lua_bases.h"

using namespace Widelands;

namespace LuaBases {

/* RST
:mod:`wl.bases`
==================

.. module:: wl.bases
   :synopsis: Base functions for common features in Game and Editor

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.bases

The :mod:`wl.bases` module contains Base functions on which the classes
of :mod:`wl.game` and :mod:`wl.editor` are based. You will not need
to create any of the functions in this module directly, but you might
use their functionality via the child classes.
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
PlayerBase
----------

.. class:: PlayerBase

	The Base class for the Player objects in Editor and Game
*/

const char L_PlayerBase::className[] = "PlayerBase";
const MethodType<L_PlayerBase> L_PlayerBase::Methods[] = {
	METHOD(L_PlayerBase, __eq),
	{0, 0},
};
const PropertyType<L_PlayerBase> L_PlayerBase::Properties[] = {
	PROP_RO(L_PlayerBase, number),
	PROP_RO(L_PlayerBase, tribe),
	{0, 0, 0},
};

void L_PlayerBase::__persist(lua_State * L) {
	PERS_UINT32("player", m_pl);
}
void L_PlayerBase::__unpersist(lua_State * L) {
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
int L_PlayerBase::get_number(lua_State * L) {
	lua_pushuint32(L, m_pl);
	return 1;
}

/* RST
	.. attribute:: tribe

		(RO) The name of the tribe of this player.
*/
int L_PlayerBase::get_tribe(lua_State *L) {
	lua_pushstring(L, get(L, get_egbase(L)).tribe().name());
	return 1;
}


/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_PlayerBase::__eq(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	const Player & me = get(L, egbase);
	const Player & you =
		(*get_base_user_class<L_PlayerBase>(L, 2))->get(L, egbase);

	lua_pushboolean
		(L, (me.player_number() == you.player_number()));
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Player & L_PlayerBase::get
		(lua_State * L, Widelands::Editor_Game_Base & egbase)
{
	if (m_pl > MAX_PLAYERS)
		report_error(L, "Illegal player number %i",  m_pl);
	Player * rv = egbase.get_player(m_pl);
	if (!rv)
		report_error(L, "Player with the number %i does not exist", m_pl);
	return *rv;
}


/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */


const static struct luaL_reg wlbases [] = {
	{0, 0}
};

void luaopen_wlbases(lua_State * const L) {
	luaL_register(L, "wl.bases", wlbases);
	lua_pop(L, 1); // pop the table from the stack again


	register_class<L_PlayerBase>(L, "bases");
}

};

