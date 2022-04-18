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

#include "scripting/factory.h"

#include "scripting/lua_editor.h"
#include "scripting/lua_game.h"

void EditorFactory::push_player(lua_State* L, Widelands::PlayerNumber plr) {
	to_lua<LuaEditor::LuaPlayer>(L, new LuaEditor::LuaPlayer(plr));
}

void GameFactory::push_player(lua_State* L, Widelands::PlayerNumber plr) {
	to_lua<LuaGame::LuaPlayer>(L, new LuaGame::LuaPlayer(plr));
}

Factory& get_factory(lua_State* const L) {
	lua_getfield(L, LUA_REGISTRYINDEX, "factory");
	Factory* fac = static_cast<Factory*>(lua_touserdata(L, -1));
	lua_pop(L, 1);  // pop this userdata

	if (fac == nullptr) {
		throw LuaError("\"factory\" field was nil, which should be impossible!");
	}

	return *fac;
}
