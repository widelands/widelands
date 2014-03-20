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

#ifndef C_UTILS_H
#define C_UTILS_H


#include "logic/game.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "scripting/eris/lua.hpp"
#include "scripting/factory.h"

Factory & get_factory(lua_State *);
Widelands::Game & get_game(lua_State *);
Widelands::Editor_Game_Base & get_egbase(lua_State *);
Widelands::Map_Map_Object_Loader * get_mol(lua_State *);
Widelands::Map_Map_Object_Saver * get_mos(lua_State *);

#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__ ((__format__ (__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif

int report_error(lua_State *, const char *, ...) PRINTF_FORMAT(2, 3);

#define luaL_checkint32(L, n)  static_cast<int32_t>(luaL_checkinteger(L, (n)))
#define luaL_checkuint32(L, n)  static_cast<uint32_t>(luaL_checkinteger(L, (n)))

#define lua_pushint32(L, n) (lua_pushinteger(L, static_cast<int32_t>(n)))
#define lua_pushuint32(L, n) (lua_pushinteger(L, static_cast<uint32_t>(n)))

inline void lua_pushstring (lua_State * L, const std::string & s) {
	lua_pushstring(L, s.c_str());
}

inline lua_State * luaL_checkthread(lua_State * L, int n) {
	luaL_checktype(L, n, LUA_TTHREAD);
	lua_State * thread = lua_tothread(L, n);
	return thread;
}

inline bool luaL_checkboolean(lua_State * L, int n) {
	if (lua_isboolean(L, n))
		return lua_toboolean(L, n);
	return luaL_checkinteger(L, n);
}

#endif
