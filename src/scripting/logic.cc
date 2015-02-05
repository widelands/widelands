/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include "scripting/logic.h"

#include <memory>

#include <boost/algorithm/string/predicate.hpp>

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

namespace  {

// Setup the basic Widelands functions and pushes egbase into the Lua registry
// so that it is available for all the other Lua functions.
void setup_for_editor_and_game(lua_State* L, Widelands::EditorGameBase * g) {
	LuaBases::luaopen_wlbases(L);
	LuaMaps::luaopen_wlmap(L);
	LuaUi::luaopen_wlui(L);

	// Push the editor game base
	lua_pushlightuserdata(L, static_cast<void *>(g));
	lua_setfield(L, LUA_REGISTRYINDEX, "egbase");
}

// Can run script also from the map.
std::unique_ptr<LuaTable> run_script_maybe_from_map(lua_State* L, const std::string& path) {
	if (boost::starts_with(path, "map:")) {
		return run_script(L, path.substr(4), get_egbase(L).map().filesystem());
	}
	return run_script(L, path, g_fs);
}

}  // namespace

LuaEditorInterface::LuaEditorInterface(Widelands::EditorGameBase* g)
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

std::unique_ptr<LuaTable> LuaEditorInterface::run_script(const std::string& script) {
	return run_script_maybe_from_map(m_L, script);
}

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

LuaCoroutine* LuaGameInterface::read_coroutine(FileRead& fr) {
	LuaCoroutine * rv = new LuaCoroutine(nullptr);
	rv->read(m_L, fr);
	return rv;
}

void LuaGameInterface::write_coroutine(FileWrite& fw, LuaCoroutine* cr) {
	cr->write(fw);
}


void LuaGameInterface::read_global_env
	(FileRead & fr, Widelands::MapObjectLoader & mol,
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
	(FileWrite & fw, Widelands::MapObjectSaver & mos)
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

std::unique_ptr<LuaTable> LuaGameInterface::get_hook(const std::string& name) {
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

	std::unique_ptr<LuaTable> return_value(new LuaTable(m_L));
	lua_pop(m_L, 1);
	return return_value;
}

std::unique_ptr<LuaTable> LuaGameInterface::run_script(const std::string& script) {
	return run_script_maybe_from_map(m_L, script);
}
