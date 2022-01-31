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

#include "scripting/logic.h"

#include <memory>

#include "base/string.h"
#include "io/filesystem/layered_filesystem.h"
#include "scripting/factory.h"
#include "scripting/globals.h"
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
#include "scripting/run_script.h"

namespace {

// Setup the basic Widelands functions and pushes egbase into the Lua registry
// so that it is available for all the other Lua functions.
void setup_for_editor_and_game(lua_State* L, Widelands::EditorGameBase* g) {
	LuaBases::luaopen_wlbases(L);
	LuaMaps::luaopen_wlmap(L);
	LuaUi::luaopen_wlui(L);

	// Push the editor game base
	lua_pushlightuserdata(L, static_cast<void*>(g));
	lua_setfield(L, LUA_REGISTRYINDEX, "egbase");
}

// Can run script also from the map.
std::unique_ptr<LuaTable> run_script_maybe_from_map(lua_State* L, const std::string& path) {
	if (starts_with(path, "map:")) {
		return run_script(L, path.substr(4), get_egbase(L).map().filesystem());
	}
	return run_script(L, path, g_fs);
}

}  // namespace

LuaEditorInterface::LuaEditorInterface(Widelands::EditorGameBase* g)
   : factory_(new EditorFactory()) {
	setup_for_editor_and_game(lua_state_, g);
	LuaRoot::luaopen_wlroot(lua_state_, true);
	LuaEditor::luaopen_wleditor(lua_state_);

	// Push the factory class into the registry
	lua_pushlightuserdata(
	   lua_state_, reinterpret_cast<void*>(dynamic_cast<Factory*>(factory_.get())));
	lua_setfield(lua_state_, LUA_REGISTRYINDEX, "factory");
}

std::unique_ptr<LuaTable> LuaEditorInterface::run_script(const std::string& script) {
	return run_script_maybe_from_map(lua_state_, script);
}

// Special handling of math.random.

// We inject this function to make sure that Lua uses our random number
// generator.  This guarantees that the game stays in sync over the network and
// in replays. Obviously, we only do this for LuaGameInterface, not for
// the others.

// The function was designed to simulate the standard math.random function and
// was therefore copied nearly verbatim from the Lua sources.
static int L_math_random(lua_State* L) {
	Widelands::Game& game = get_game(L);
	uint32_t t = game.logic_rand();

	lua_Number r = t / 4294967296.;  // create a double in [0,1)

	switch (lua_gettop(L)) { /* check number of arguments */
	case 0: {                /* no arguments */
		lua_pushdouble(L, r); /* Number between 0 and 1 */
		break;
	}
	case 1: { /* only upper limit */
		int32_t u = luaL_checkint32(L, 1);
		luaL_argcheck(L, 1 <= u, 1, "interval is empty");
		lua_pushuint32(L, floor(r * u) + 1); /* int between 1 and `u' */
		break;
	}
	case 2: { /* lower and upper limits */
		int32_t l = luaL_checkint32(L, 1);
		int32_t u = luaL_checkint32(L, 2);
		luaL_argcheck(L, l <= u, 2, "interval is empty");
		/* int between `l' and `u' */
		lua_pushint32(L, floor(r * (u - l + 1)) + l);
		break;
	}
	default:
		return luaL_error(L, "wrong number of arguments");
	}
	return 1;
}

LuaGameInterface::LuaGameInterface(Widelands::Game* g) : factory_(new GameFactory()) {
	setup_for_editor_and_game(lua_state_, g);

	// Overwrite math.random
	lua_getglobal(lua_state_, "math");
	lua_pushcfunction(lua_state_, L_math_random);
	lua_setfield(lua_state_, -2, "random");
	lua_pop(lua_state_, 1);  // pop "math"

	LuaRoot::luaopen_wlroot(lua_state_, false);
	LuaGame::luaopen_wlgame(lua_state_);

	// Push the game into the registry
	lua_pushlightuserdata(lua_state_, static_cast<void*>(g));
	lua_setfield(lua_state_, LUA_REGISTRYINDEX, "game");

	// Push the factory class into the registry
	lua_pushlightuserdata(
	   lua_state_, reinterpret_cast<void*>(dynamic_cast<Factory*>(factory_.get())));
	lua_setfield(lua_state_, LUA_REGISTRYINDEX, "factory");
}

std::unique_ptr<LuaCoroutine> LuaGameInterface::read_coroutine(FileRead& fr) {
	std::unique_ptr<LuaCoroutine> rv(new LuaCoroutine(nullptr));
	rv->read(lua_state_, fr);
	return rv;
}

void LuaGameInterface::write_coroutine(FileWrite& fw, const LuaCoroutine& cr) {
	cr.write(fw);
}

void LuaGameInterface::read_global_env(FileRead& fr,
                                       Widelands::MapObjectLoader& mol,
                                       uint32_t size) {
	// Clean out the garbage before loading.
	lua_gc(lua_state_, LUA_GCCOLLECT, 0);

	assert(lua_gettop(lua_state_) == 0);  // S:
	unpersist_object(lua_state_, fr, mol, size);
	assert(lua_gettop(lua_state_) == 1);  // S: unpersisted_object
	luaL_checktype(lua_state_, -1, LUA_TTABLE);

	// Now, we have to merge all keys from the loaded table
	// into the global table
	lua_pushnil(lua_state_);  // S: table nil
	while (lua_next(lua_state_, 1) != 0) {
		// S: table key value
		lua_rawgeti(
		   lua_state_, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);  // S: table key value globals_table
		lua_pushvalue(lua_state_, -3);                        // S: table key value globals_table key
		lua_gettable(lua_state_, -2);  // S: table key value globals_table value_in_globals
		if (lua_compare(lua_state_, -1, -3, LUA_OPEQ)) {
			lua_pop(lua_state_, 3);  // S: table key
			continue;
		} else {
			// Make this a global value
			lua_pop(lua_state_, 1);         // S: table key value globals_table
			lua_pushvalue(lua_state_, -3);  // S: table key value globals_table key
			lua_pushvalue(lua_state_, -3);  // S: table key value globals_table key value
			lua_settable(lua_state_, -3);   // S: table key value globals_table
			lua_pop(lua_state_, 2);         // S: table key
		}
	}

	lua_pop(lua_state_, 1);  // pop the table returned by unpersist_object

	// Clean out the garbage before returning.
	lua_gc(lua_state_, LUA_GCCOLLECT, 0);
}

void LuaGameInterface::read_textdomain_stack(FileRead& fr) {
	LuaGlobals::read_textdomain_stack(fr, lua_state_);
}
void LuaGameInterface::write_textdomain_stack(FileWrite& fw) {
	LuaGlobals::write_textdomain_stack(fw, lua_state_);
}

uint32_t LuaGameInterface::write_global_env(FileWrite& fw, Widelands::MapObjectSaver& mos) {
	// Clean out the garbage before writing.
	lua_gc(lua_state_, LUA_GCCOLLECT, 0);

	// Empty table + object to persist on the stack Stack
	lua_newtable(lua_state_);
	lua_rawgeti(lua_state_, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

	uint32_t nwritten = persist_object(lua_state_, fw, mos);

	// Garbage collect once more, so we do not return unnecessary stuff.
	lua_gc(lua_state_, LUA_GCCOLLECT, 0);

	return nwritten;
}

std::unique_ptr<LuaTable> LuaGameInterface::get_hook(const std::string& name) {
	lua_getglobal(lua_state_, "hooks");
	if (lua_isnil(lua_state_, -1)) {
		lua_pop(lua_state_, 1);
		return std::unique_ptr<LuaTable>();
	}

	lua_getfield(lua_state_, -1, name.c_str());
	if (lua_isnil(lua_state_, -1)) {
		lua_pop(lua_state_, 2);
		return std::unique_ptr<LuaTable>();
	}
	lua_remove(lua_state_, -2);

	std::unique_ptr<LuaTable> return_value(new LuaTable(lua_state_));
	lua_pop(lua_state_, 1);
	return return_value;
}

std::unique_ptr<LuaTable> LuaGameInterface::run_script(const std::string& script) {
	return run_script_maybe_from_map(lua_state_, script);
}
