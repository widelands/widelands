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

#include "scripting/scripting.h"

#include <stdexcept>
#include <string>

#include <boost/algorithm/string/predicate.hpp>
#ifdef _MSC_VER
#include <ctype.h> // for tolower
#endif
#include <stdint.h>

#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "scripting/c_utils.h"
#include "scripting/factory.h"
#include "scripting/lua_bases.h"
#include "scripting/lua_coroutine.h"
#include "scripting/lua_editor.h"
#include "scripting/lua_game.h"
#include "scripting/lua_globals.h"
#include "scripting/lua_map.h"
#include "scripting/lua_root.h"
#include "scripting/lua_table.h"
#include "scripting/lua_ui.h"
#include "scripting/persistence.h"

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

// Checks the return value of a function all for nonzero state and throws the
// string that the function hopefully pushed as an Error. Returns 'rv' if there
// is no error.
int check_return_value_for_errors(lua_State* L, int rv) {
	if (rv) {
		const std::string err = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		throw LuaError(err);
	}
	return rv;
}

// Setup the basic Widelands functions and pushes egbase into the Lua registry
// so that it is available for all the other Lua functions.
void setup_for_editor_and_game(lua_State* L, Widelands::Editor_Game_Base * g) {
	LuaBases::luaopen_wlbases(L);
	LuaMap::luaopen_wlmap(L);
	LuaUi::luaopen_wlui(L);

	// Push the editor game base
	lua_pushlightuserdata(L, static_cast<void *>(g));
	lua_setfield(L, LUA_REGISTRYINDEX, "egbase");
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
	   L,
	   luaL_loadbuffer(L, content.c_str(), content.size(), identifier.c_str()) ||
	      lua_pcall(L, 0, 1, 0));

	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);    // No return value from script
		lua_newtable(L);  // Push an empty table
	}
	if (not lua_istable(L, -1))
		throw LuaError("Script did not return a table!");

	// Restore old value of __file__.
	if (last_file.empty()) {
		lua_pushnil(L);
	} else {
		lua_pushstring(L, last_file);
	}
	lua_setglobal(L, "__file__");

	return std::unique_ptr<LuaTable>(new LuaTable(L));
}

// Reads the 'filename' from the 'fs' and returns its content.
std::string get_file_content(FileSystem* fs, const std::string& filename) {
	if (!fs || !fs->FileExists(filename)) {
		throw LuaScriptNotExistingError(filename);
	}
	size_t length;
	void* input_data = fs->Load(filename, length);
	const std::string data(static_cast<char*>(input_data));
	// make sure the input_data is freed
	free(input_data);
	return data;
}

}  // namespace


/*
============================================
       Lua Interface
============================================
*/
LuaInterface::LuaInterface() {
	m_L = luaL_newstate();

	// Open the Lua libraries
	open_lua_library(m_L, "", luaopen_base, false);
	open_lua_library(m_L, LUA_TABLIBNAME, luaopen_table, true);
	open_lua_library(m_L, LUA_STRLIBNAME, luaopen_string, true);
	open_lua_library(m_L, LUA_MATHLIBNAME, luaopen_math, true);
	open_lua_library(m_L, LUA_DBLIBNAME, luaopen_debug, true);
	open_lua_library(m_L, LUA_COLIBNAME, luaopen_coroutine, true);

#ifndef NDEBUG
	open_lua_library(m_L, LUA_LOADLIBNAME, luaopen_package, true);
	open_lua_library(m_L, LUA_IOLIBNAME, luaopen_io, true);
	open_lua_library(m_L, LUA_OSLIBNAME, luaopen_os, true);
#endif

	// Push the instance of this class into the registry
	// MSVC2008 requires that stored and retrieved types are
	// same, so use LuaInterface* on both sides.
	lua_pushlightuserdata
		(m_L, reinterpret_cast<void *>(dynamic_cast<LuaInterface *>(this)));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "lua_interface");

	// Now our own
	LuaGlobals::luaopen_globals(m_L);

	// Also push the "wl" table.
	lua_newtable(m_L);
	lua_setglobal(m_L, "wl");
}

LuaInterface::~LuaInterface() {
	lua_close(m_L);
}

void LuaInterface::interpret_string(const std::string& cmd) {
	int rv = luaL_dostring(m_L, cmd.c_str());
	check_return_value_for_errors(m_L, rv);
}

std::unique_ptr<LuaTable> LuaInterface::run_script(const std::string& path) {
	std::string content;

	if (boost::starts_with(path, "map:")) {
		content = get_file_content(get_egbase(m_L).map().filesystem(), path.substr(4));
	} else {
		content = get_file_content(g_fs, path);
	}

	return run_string_as_script(m_L, path, content);
}

/*
 * Returns a given hook if one is defined, otherwise returns 0
 */
std::unique_ptr<LuaTable> LuaInterface::get_hook(const std::string& name) {
	lua_getglobal(m_L, "hooks");
	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 1);
		return std::unique_ptr<LuaTable>();
	}

	lua_getfield(m_L, -1, name.c_str());
	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 2);
		return std::unique_ptr<LuaTable>();
	}
	lua_remove(m_L, -2);

	return std::unique_ptr<LuaTable>(new LuaTable(m_L));
}


/*
 * ===========================
 * LuaEditorInterface
 * ===========================
 */
LuaEditorInterface::LuaEditorInterface(Widelands::Editor_Game_Base* g)
	: m_factory(new EditorFactory())
{
	setup_for_editor_and_game(m_L, g);
	LuaRoot::luaopen_wlroot(m_L, true);
	LuaEditor::luaopen_wleditor(m_L);

	// Push the factory class into the registry
	lua_pushlightuserdata(m_L, reinterpret_cast<void*>(dynamic_cast<Factory*>(m_factory.get())));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "factory");
}

LuaEditorInterface::~LuaEditorInterface() {
}


/*
 * ===========================
 * LuaGameInterface
 * ===========================
 */

// Special handling of math.random.

// We inject this function to make sure that Lua uses our random number
// generator.  This guarantees that the game stays in sync over the network and
// in replays. Obviously, we only do this for LuaGameInterface, not for
// the others.

// The function was designed to simulate the standard math.random function and
// was therefore copied nearly verbatim from the Lua sources.
static int L_math_random(lua_State * L) {
	Widelands::Game & game = get_game(L);
	uint32_t t = game.logic_rand();

	lua_Number r = t / 4294967296.; // create a double in [0,1)

	switch (lua_gettop(L)) { /* check number of arguments */
		case 0:
		{  /* no arguments */
			lua_pushnumber(L, r);  /* Number between 0 and 1 */
			break;
		}
		case 1:
		{  /* only upper limit */
			int32_t u = luaL_checkint32(L, 1);
			luaL_argcheck(L, 1 <= u, 1, "interval is empty");
			lua_pushnumber(L, floor(r * u) + 1);  /* int between 1 and `u' */
			break;
		}
		case 2:
		{  /* lower and upper limits */
			int32_t l = luaL_checkint32(L, 1);
			int32_t u = luaL_checkint32(L, 2);
			luaL_argcheck(L, l <= u, 2, "interval is empty");
			/* int between `l' and `u' */
			lua_pushnumber(L, floor(r * (u - l + 1)) + l);
			break;
		}
		default: return luaL_error(L, "wrong number of arguments");
	}
	return 1;

}

LuaGameInterface::LuaGameInterface(Widelands::Game * g)
	: m_factory(new GameFactory())
{
	setup_for_editor_and_game(m_L, g);

	// Overwrite math.random
	lua_getglobal(m_L, "math");
	lua_pushcfunction(m_L, L_math_random);
	lua_setfield(m_L, -2, "random");
	lua_pop(m_L, 1); // pop "math"

	LuaRoot::luaopen_wlroot(m_L, false);
	LuaGame::luaopen_wlgame(m_L);

	// Push the game into the registry
	lua_pushlightuserdata(m_L, static_cast<void *>(g));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "game");

	// Push the factory class into the registry
	lua_pushlightuserdata
		(m_L, reinterpret_cast<void *>(dynamic_cast<Factory *>(m_factory.get())));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "factory");
}

LuaGameInterface::~LuaGameInterface() {
}

LuaCoroutine * LuaGameInterface::read_coroutine
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	LuaCoroutine * rv = new LuaCoroutine(nullptr);
	rv->read(m_L, fr, mol, size);

	return rv;
}

uint32_t LuaGameInterface::write_coroutine
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos, LuaCoroutine * cr)
{
	return cr->write(m_L, fw, mos);
}


void LuaGameInterface::read_global_env
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	// Clean out the garbage before loading.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	assert(lua_gettop(m_L) == 0); // S:
	unpersist_object(m_L, fr, mol, size);
	assert(lua_gettop(m_L) == 1); // S: unpersisted_object
	luaL_checktype(m_L, -1, LUA_TTABLE);

	// Now, we have to merge all keys from the loaded table
	// into the global table
	lua_pushnil(m_L);  // S: table nil
	while (lua_next(m_L, 1) != 0) {
		// S: table key value
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);  // S: table key value globals_table
		lua_pushvalue(m_L, -3); // S: table key value globals_table key
		lua_gettable(m_L, -2);  // S: table key value globals_table value_in_globals
		if (lua_compare(m_L, -1, -3, LUA_OPEQ)) {
			lua_pop(m_L, 3); // S: table key
			continue;
		} else {
			// Make this a global value
			lua_pop(m_L, 1);  // S: table key value globals_table
			lua_pushvalue(m_L, -3);  // S: table key value globals_table key
			lua_pushvalue(m_L, -3);  // S: table key value globals_table key value
			lua_settable(m_L, -3);  // S: table key value globals_table
			lua_pop(m_L, 2);  // S: table key
		}
	}

	lua_pop(m_L, 1); // pop the table returned by unpersist_object

	// Clean out the garbage before returning.
	lua_gc(m_L, LUA_GCCOLLECT, 0);
}

uint32_t LuaGameInterface::write_global_env
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos)
{
	// Clean out the garbage before writing.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	// Empty table + object to persist on the stack Stack
	lua_newtable(m_L);
	lua_rawgeti(m_L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

	uint32_t nwritten = persist_object(m_L, fw, mos);

	// Garbage collect once more, so we do not return unnecessary stuff.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	return nwritten;
}
