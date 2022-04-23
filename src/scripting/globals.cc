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

#include "scripting/globals.h"

Widelands::Game& get_game(lua_State* const L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "game");
	Widelands::Game* g = static_cast<Widelands::Game*>(lua_touserdata(L, -1));
	lua_pop(L, 1);  // pop this userdata

	if (g == nullptr) {
		throw LuaError("\"game\" field was nil. get_game was not called in a game.");
	}

	return *g;
}

Widelands::EditorGameBase& get_egbase(lua_State* const L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "egbase");
	Widelands::EditorGameBase* g = static_cast<Widelands::EditorGameBase*>(lua_touserdata(L, -1));
	lua_pop(L, 1);  // pop this userdata

	if (g == nullptr) {
		throw LuaError("\"egbase\" field was nil. This should be impossible.");
	}

	return *g;
}

Widelands::MapObjectLoader* get_mol(lua_State* const L) {
	lua_pushstring(L, "mol");
	lua_gettable(L, LUA_REGISTRYINDEX);

	Widelands::MapObjectLoader* mol =
	   static_cast<Widelands::MapObjectLoader*>(lua_touserdata(L, -1));

	lua_pop(L, 1);  // pop this userdata

	if (mol == nullptr) {
		throw LuaError("\"mol\" field was nil. This should be impossible.");
	}

	return mol;
}

Widelands::MapObjectSaver* get_mos(lua_State* const L) {
	lua_pushstring(L, "mos");
	lua_gettable(L, LUA_REGISTRYINDEX);

	Widelands::MapObjectSaver* mos = static_cast<Widelands::MapObjectSaver*>(lua_touserdata(L, -1));

	lua_pop(L, 1);  // pop this userdata

	if (mos == nullptr) {
		throw LuaError("\"mos\" field was nil. This should be impossible.");
	}

	return mos;
}
