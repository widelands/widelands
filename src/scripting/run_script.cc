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

#include "scripting/run_script.h"

#include <memory>

#include "io/filesystem/filesystem.h"
#include "scripting/lua_table.h"

namespace {

// Reads the 'filename' from the 'fs' and returns its content.
std::string get_file_content(FileSystem* fs, const std::string& filename) {
	if (!fs || !fs->file_exists(filename)) {
		throw LuaScriptNotExistingError(filename);
	}
	if (fs->is_directory(filename)) {
		throw LuaScriptNotExistingError(format("%s is a directory", filename));
	}
	size_t length;
	void* input_data = fs->load(filename, length);
	const std::string data(static_cast<char*>(input_data));
	// make sure the input_data is freed
	free(input_data);
	return data;
}

// Runs the 'content' as a lua script identified by 'identifier' in 'L'.
std::unique_ptr<LuaTable>
run_string_as_script(lua_State* L, const std::string& identifier, const std::string& content) {
	// Get the current value of __file__
	std::string last_file;
	lua_getglobal(L, "__file__");
	if (!lua_isnil(L, -1)) {
		last_file = luaL_checkstring(L, -1);
	}
	lua_pop(L, 1);

	// Set __file__.
	lua_pushstring(L, identifier);
	lua_setglobal(L, "__file__");

	check_return_value_for_errors(
	   L, luaL_loadbuffer(L, content.c_str(), content.size(), identifier.c_str()) ||
	         lua_pcall(L, 0, 1, 0));

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);    // No return value from script
		lua_newtable(L);  // Push an empty table
	}
	if (!lua_istable(L, -1)) {
		throw LuaError("Script did not return a table!");
	}

	// Restore old value of __file__.
	if (last_file.empty()) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, last_file);
	}
	lua_setglobal(L, "__file__");

	std::unique_ptr<LuaTable> return_value(new LuaTable(L));
	lua_pop(L, 1);
	return return_value;
}

}  // namespace

int check_return_value_for_errors(lua_State* L, int rv) {
	if (rv) {
		const std::string err = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		throw LuaError(err);
	}
	return rv;
}

std::unique_ptr<LuaTable> run_script(lua_State* L, const std::string& path, FileSystem* fs) {
	const std::string content = get_file_content(fs, path);
	return run_string_as_script(L, path, content);
}
