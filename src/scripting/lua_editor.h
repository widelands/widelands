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

#ifndef LUA_EDITOR_H
#define LUA_EDITOR_H

#include <lua.hpp>

#include "lua_bases.h"

class L_EPlayer : public L_PlayerBase {
public:
	// Overwritten from L_PlayerBase, avoid ambiguity when deriving from 
	// L_GameModuleClass and L_PlayerBase
	const char * get_modulename() {return "game";}

	LUNA_CLASS_HEAD(L_EPlayer);

	L_EPlayer() : L_PlayerBase() {}
	// TODO: is this constructor needed?
	L_EPlayer(Widelands::Player_Number n) : L_PlayerBase(n)  {}
	L_EPlayer(lua_State * L);

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


#endif
