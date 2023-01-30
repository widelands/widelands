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

#include "scripting/lua_interface.h"

#include <memory>

#include "base/multithreading.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/lua_globals.h"
#include "scripting/lua_path.h"
#include "scripting/lua_styles.h"
#include "scripting/lua_table.h"
#include "scripting/lua_ui.h"
#include "scripting/run_script.h"

namespace {

// Calls 'method_to_call' with argument 'name'. This expects that this will
// return a table with registered functions in it. If 'register_globally' is
// true, this will also do name = <table> globally.
void open_lua_library(lua_State* L,
                      const std::string& name,
                      lua_CFunction method_to_call,
                      bool register_globally) {
	lua_pushcfunction(L, method_to_call);  // S: function
	lua_pushstring(L, name);               // S: function name
	lua_call(L, 1, 1);                     // S: module_table

	if (register_globally) {
		lua_setglobal(L, name.c_str());  // S:
	} else {
		lua_pop(L, 1);  // S:
	}
}

}  // namespace

LuaInterface::LuaInterface() {
	lua_state_ = luaL_newstate();

	// Open the Lua libraries
	open_lua_library(lua_state_, "", luaopen_base, false);
	open_lua_library(lua_state_, LUA_TABLIBNAME, luaopen_table, true);
	open_lua_library(lua_state_, LUA_STRLIBNAME, luaopen_string, true);
	open_lua_library(lua_state_, LUA_MATHLIBNAME, luaopen_math, true);
	open_lua_library(lua_state_, LUA_DBLIBNAME, luaopen_debug, true);
	open_lua_library(lua_state_, LUA_COLIBNAME, luaopen_coroutine, true);

	// Push the instance of this class into the registry
	// MSVC2008 requires that stored and retrieved types are
	// same, so use LuaInterface* on both sides.
	lua_pushlightuserdata(lua_state_, reinterpret_cast<void*>(dynamic_cast<LuaInterface*>(this)));
	lua_setfield(lua_state_, LUA_REGISTRYINDEX, "lua_interface");

	// Now our own
	LuaGlobals::luaopen_globals(lua_state_);

	// And helper methods.
	LuaPath::luaopen_path(lua_state_);
	LuaStyles::luaopen_styles(lua_state_);

	// Also push the "wl" and the "hooks" table.
	lua_newtable(lua_state_);
	lua_setglobal(lua_state_, "wl");

	lua_newtable(lua_state_);
	lua_setglobal(lua_state_, "hooks");

	// Game tips need this to access hotkeys.
	LuaUi::luaopen_wlui(lua_state_);
}

LuaInterface::~LuaInterface() {
	lua_close(lua_state_);
}

void LuaInterface::interpret_string(const std::string& cmd) {
	int rv = luaL_dostring(lua_state_, cmd.c_str());
	check_return_value_for_errors(lua_state_, rv);
}

std::unique_ptr<LuaTable> LuaInterface::run_script(const std::string& path) {
	return ::run_script(lua_state_, g_fs->fix_cross_file(path), g_fs);
}

std::unique_ptr<LuaTable> LuaInterface::empty_table() {
	lua_newtable(lua_state_);
	std::unique_ptr<LuaTable> rv(new LuaTable(lua_state_));
	lua_pop(lua_state_, 1);
	return rv;
}
