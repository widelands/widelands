/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LUA_H
#define WL_SCRIPTING_LUA_H

#include <string>

#include "third_party/eris/lua.hpp"

#define luaL_checkint32(L, n) static_cast<int32_t>(luaL_checkinteger(L, (n)))
#define luaL_checkuint32(L, n) static_cast<uint32_t>(luaL_checkinteger(L, (n)))
#define luaL_checkdouble(L, n) static_cast<double>(luaL_checknumber(L, (n)))

#define lua_pushint32(L, n) (lua_pushinteger(L, static_cast<int32_t>(n)))
#define lua_pushuint32(L, n) (lua_pushinteger(L, static_cast<uint32_t>(n)))
#define lua_pushdouble(L, n) (lua_pushnumber(L, static_cast<double>(n)))

void lua_pushstring(lua_State* L, const std::string& s);

lua_State* luaL_checkthread(lua_State* L, int n);

bool luaL_checkboolean(lua_State* L, int n);

#endif  // end of include guard: WL_SCRIPTING_LUA_H
