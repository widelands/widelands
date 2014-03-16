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

#include "scripting/lua_editor.h"

#include "log.h"
#include "logic/game.h"
#include "logic/player.h"
#include "scripting/c_utils.h"
#include "wui/interactive_player.h"

namespace LuaEditor {

/* RST
:mod:`wl.editor`
==================

.. module:: wl.editor
   :synopsis: Editor scripting functions

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.editor

The :mod:`wl.editor` module contains functionality that integrates Lua into the
editor. This module is not loaded inside the game, that is if wl.editor is
:const:`nil` the script is run inside a game, else in the editor.
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
Player
------

.. class:: Player

	Child of: :class:`wl.bases.PlayerBase`

	This class represents one of the players in the editor.
	Note that you cannot instantiate a class of this type directly, use the
	:attr:`wl.Editor.players`.

*/
const char L_Player::className[] = "Player";
const MethodType<L_Player> L_Player::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_Player> L_Player::Properties[] = {
	{nullptr, nullptr, nullptr},
};


/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

const static struct luaL_Reg wleditor [] = {
	{nullptr, nullptr}
};

void luaopen_wleditor(lua_State * const L) {
	lua_getglobal(L, "wl");  // S: wl_table
	lua_pushstring(L, "editor"); // S: wl_table "editor"
	luaL_newlib(L, wleditor);  // S: wl_table "editor" wl.editor_table
	lua_settable(L, -3); // S: wl_table
	lua_pop(L, 1); // S:

	register_class<L_Player>(L, "editor", true);
	add_parent<L_Player, LuaBases::L_PlayerBase>(L);
	lua_pop(L, 1); // Pop the meta table
}

};
