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

#ifndef LUA_ROOT_H
#define LUA_ROOT_H

#include <lua.hpp>

#include "luna.h"

namespace LuaRoot {

/*
 * Base class for all classes in wl
 */
class L_RootModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "";}
};

class L_Game : public LuaBases::L_EditorGameBase {
public:
	LUNA_CLASS_HEAD(L_Game);
	const char * get_modulename() {return "";}

	L_Game() {}
	L_Game(lua_State * L);

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_time(lua_State *);
	int get_desired_speed(lua_State *);
	int set_desired_speed(lua_State *);
	int get_allow_autosaving(lua_State *);
	int set_allow_autosaving(lua_State *);

	/*
	 * Lua methods
	 */
	int launch_coroutine(lua_State *);
	int save(lua_State *);

	/*
	 * C methods
	 */
protected:
	virtual void player_to_lua(lua_State * L, Widelands::Player_Number plr) {
		to_lua<LuaGame::L_Player>(L, new LuaGame::L_Player(plr));
	}
};

class L_Editor : public LuaBases::L_EditorGameBase {
public:
	LUNA_CLASS_HEAD(L_Editor);
	const char * get_modulename() {return "";}

	L_Editor() {}
	L_Editor(lua_State * L);
	virtual ~L_Editor() {}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
protected:
	virtual void player_to_lua(lua_State * L, Widelands::Player_Number plr) {
		to_lua<LuaEditor::L_Player>(L, new LuaEditor::L_Player(plr));
	}
};

void luaopen_wlroot(lua_State *, bool in_editor);

#endif
};


