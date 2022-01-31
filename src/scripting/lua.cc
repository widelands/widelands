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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scripting/lua.h"

void lua_pushstring(lua_State* L, const std::string& s) {
	lua_pushstring(L, s.c_str());
}

lua_State* luaL_checkthread(lua_State* L, int n) {
	luaL_checktype(L, n, LUA_TTHREAD);
	lua_State* thread = lua_tothread(L, n);
	return thread;
}

bool luaL_checkboolean(lua_State* L, int n) {
	if (lua_isboolean(L, n)) {
		return lua_toboolean(L, n);
	}
	return luaL_checkinteger(L, n);
}
