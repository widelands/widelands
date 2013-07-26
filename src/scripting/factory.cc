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

#include "scripting/factory.h"

#include "scripting/lua_editor.h"
#include "scripting/lua_game.h"

void EditorFactory::push_player(lua_State * L, Widelands::Player_Number plr) {
	to_lua<LuaEditor::L_Player>(L, new LuaEditor::L_Player(plr));
}

void GameFactory::push_player(lua_State * L, Widelands::Player_Number plr) {
		to_lua<LuaGame::L_Player>(L, new LuaGame::L_Player(plr));
}

