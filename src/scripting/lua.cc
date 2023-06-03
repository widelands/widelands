/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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

#include "scripting/report_error.h"

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
		return lua_toboolean(L, n) != 0;
	}
	return luaL_checkinteger(L, n) != 0;
}

std::string
get_table_string(lua_State* L, const char* key, bool mandatory, std::string default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		default_value = luaL_checkstring(L, -1);
	} else if (mandatory) {
		report_error(L, "Missing string: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

int32_t get_table_int(lua_State* L, const char* key, bool mandatory, int32_t default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		default_value = luaL_checkint32(L, -1);
	} else if (mandatory) {
		report_error(L, "Missing integer: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}

bool get_table_boolean(lua_State* L, const char* key, bool mandatory, bool default_value) {
	lua_getfield(L, -1, key);
	if (!lua_isnil(L, -1)) {
		default_value = luaL_checkboolean(L, -1);
	} else if (mandatory) {
		report_error(L, "Missing boolean: %s", key);
	}
	lua_pop(L, 1);
	return default_value;
}
