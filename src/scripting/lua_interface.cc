/*
 * Copyright (C) 2006-2010, 2013 by the Widelands Development Team
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

#include "scripting/lua_interface.h"

#include <memory>
#include <string>

#include <boost/algorithm/string/predicate.hpp>

#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_globals.h"
#include "scripting/lua_path.h"
#include "scripting/lua_table.h"
#include "scripting/run_script.h"

namespace {

// Calls 'method_to_call' with argument 'name'. This expects that this will
// return a table with registered functions in it. If 'register_globally' is
// true, this will also do name = <table> globally.
void open_lua_library
	(lua_State* L, const std::string& name, lua_CFunction method_to_call, bool register_globally) {
	lua_pushcfunction(L, method_to_call);  // S: function
	lua_pushstring(L, name); // S: function name
	lua_call(L, 1, 1); // S: module_table

	if (register_globally) {
		lua_setglobal(L, name.c_str()); // S:
	} else {
		lua_pop(L, 1); // S:
	}
}

}  // namespace


LuaInterface::LuaInterface() {
	m_L = luaL_newstate();

	// Open the Lua libraries
	open_lua_library(m_L, "", luaopen_base, false);
	open_lua_library(m_L, LUA_TABLIBNAME, luaopen_table, true);
	open_lua_library(m_L, LUA_STRLIBNAME, luaopen_string, true);
	open_lua_library(m_L, LUA_MATHLIBNAME, luaopen_math, true);
	open_lua_library(m_L, LUA_DBLIBNAME, luaopen_debug, true);
	open_lua_library(m_L, LUA_COLIBNAME, luaopen_coroutine, true);

	// Push the instance of this class into the registry
	// MSVC2008 requires that stored and retrieved types are
	// same, so use LuaInterface* on both sides.
	lua_pushlightuserdata
		(m_L, reinterpret_cast<void *>(dynamic_cast<LuaInterface *>(this)));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "lua_interface");

	// Now our own
	LuaGlobals::luaopen_globals(m_L);

	// And helper methods.
	LuaPath::luaopen_path(m_L);

	// Also push the "wl" and the "hooks" table.
	lua_newtable(m_L);
	lua_setglobal(m_L, "wl");

	lua_newtable(m_L);
	lua_setglobal(m_L, "hooks");
}

LuaInterface::~LuaInterface() {
	lua_close(m_L);
}

void LuaInterface::interpret_string(const std::string& cmd) {
	int rv = luaL_dostring(m_L, cmd.c_str());
	check_return_value_for_errors(m_L, rv);
}

std::unique_ptr<LuaTable> LuaInterface::run_script(const std::string& path) {
	return ::run_script(m_L, path, g_fs);
}
