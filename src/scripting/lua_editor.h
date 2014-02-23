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

#ifndef LUA_EDITOR_H
#define LUA_EDITOR_H

#include "scripting/eris/lua.hpp"
#include "scripting/lua_bases.h"

namespace LuaEditor {

class L_Player : public LuaBases::L_PlayerBase {
public:
	// Overwritten from L_PlayerBase, avoid ambiguity when deriving from
	// L_GameModuleClass and L_PlayerBase
	const char * get_modulename() override {return "game";}

	LUNA_CLASS_HEAD(L_Player);

	L_Player() : LuaBases::L_PlayerBase() {}
	L_Player(Widelands::Player_Number n) : LuaBases::L_PlayerBase(n)  {}
	L_Player(lua_State * L) {
		report_error(L, "Cannot instantiate a 'Player' directly!");
	}

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
private:
};

void luaopen_wleditor(lua_State *);

};


#endif
