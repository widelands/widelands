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


#include "scripting/c_utils.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>

#include "scripting/factory.h"
#include "scripting/scripting.h"

Factory & get_factory(lua_State * const L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "factory");
	Factory * fac = static_cast<Factory *>(lua_touserdata(L, -1));
	lua_pop(L, 1); // pop this userdata

	if (!fac)
		throw LuaError("\"factory\" field was nil, which should be impossible!");

	return *fac;
}

Widelands::Game & get_game(lua_State * const L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "game");
	Widelands::Game * g = static_cast<Widelands::Game *>(lua_touserdata(L, -1));
	lua_pop(L, 1); // pop this userdata

	if (!g)
		throw LuaError
			("\"game\" field was nil. get_game was not called in a game.");

	return *g;
}

Widelands::EditorGameBase & get_egbase(lua_State * const L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "egbase");
	Widelands::EditorGameBase * g = static_cast<Widelands::EditorGameBase *>
		(lua_touserdata(L, -1));
	lua_pop(L, 1); // pop this userdata

	if (!g)
		throw LuaError
			("\"egbase\" field was nil. This should be impossible.");


	return *g;
}

Widelands::MapObjectLoader * get_mol(lua_State * const L) {
	lua_pushstring(L, "mol");
	lua_gettable(L, LUA_REGISTRYINDEX);

	Widelands::MapObjectLoader * mol =
		static_cast<Widelands::MapObjectLoader *>(lua_touserdata(L, -1));

	lua_pop(L, 1); // pop this userdata

	if (!mol)
		throw LuaError
			("\"mol\" field was nil. This should be impossible.");

	return mol;
}

Widelands::MapObjectSaver * get_mos(lua_State * const L) {
	lua_pushstring(L, "mos");
	lua_gettable(L, LUA_REGISTRYINDEX);

	Widelands::MapObjectSaver * mos =
		static_cast<Widelands::MapObjectSaver *>(lua_touserdata(L, -1));

	lua_pop(L, 1); // pop this userdata

	if (!mos)
		throw LuaError
			("\"mos\" field was nil. This should be impossible.");

	return mos;
}

/*
 * Returns an error to lua. This method never returns as lua_error long jumps.
 */
void report_error(lua_State * L, const char * const fmt, ...) {
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	// Also create a traceback
	lua_getglobal(L, "debug");
	assert(lua_istable(L, -1));
	lua_getfield(L, -1, "traceback");
	assert(lua_isfunction(L, -1));
	lua_pushstring(L, buffer); // error message
	lua_pushinteger(L, 2);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */

	lua_error(L); // lua_error never returns.
	throw LuaError("Never here."); // Shutup compiler warnings.
}
