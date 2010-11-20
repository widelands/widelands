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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "lua_editor.h"
#include "lua_game.h"

#include "factory.h"


void EditorFactory::push_player
	(lua_State * const L, Widelands::Player_Number const plr)
{
	to_lua<LuaEditor::L_Player>(L, new LuaEditor::L_Player(plr));
}

void GameFactory::push_player
	(lua_State * const L, Widelands::Player_Number const plr)
{
	to_lua<LuaGame::L_Player>(L, new LuaGame::L_Player(plr));
}

