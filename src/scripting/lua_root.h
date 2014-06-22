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

#ifndef LUA_ROOT_H
#define LUA_ROOT_H

#include "scripting/lua_bases.h"
#include "scripting/luna.h"
#include "third_party/eris/lua.hpp"

namespace LuaRoot {

/*
 * Base class for all classes in wl
 */
class L_RootModuleClass : public LunaClass {
	public:
		const char * get_modulename() override {return "";}
};

class L_Game : public LuaBases::L_EditorGameBase {
public:
	LUNA_CLASS_HEAD(L_Game);
	const char * get_modulename() override {return "";}

	L_Game() {}
	L_Game(lua_State * L);

	virtual void __persist(lua_State * L) override;
	virtual void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_time(lua_State *);
	int get_desired_speed(lua_State *);
	int set_desired_speed(lua_State *);
	int get_allow_autosaving(lua_State *);
	int set_allow_autosaving(lua_State *);
	int get_allow_saving(lua_State *);
	int set_allow_saving(lua_State *);

	/*
	 * Lua methods
	 */
	int launch_coroutine(lua_State *);
	int save(lua_State *);

	/*
	 * C methods
	 */
};

class L_Editor : public LuaBases::L_EditorGameBase {
public:
	LUNA_CLASS_HEAD(L_Editor);
	const char * get_modulename() override {return "";}

	L_Editor() {}
	L_Editor(lua_State * L);
	virtual ~L_Editor() {}

	virtual void __persist(lua_State * L) override;
	virtual void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
};

class L_World : public L_RootModuleClass {
public:
	LUNA_CLASS_HEAD(L_World);
	const char * get_modulename() override {return "";}

	L_World() {}
	L_World(lua_State * L);

	virtual void __persist(lua_State * L) override;
	virtual void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */
	int new_critter_type(lua_State* L);
	int new_editor_immovable_category(lua_State* L);
	int new_editor_terrain_category(lua_State* L);
	int new_immovable_type(lua_State* L);
	int new_resource_type(lua_State* L);
	int new_terrain_type(lua_State* L);

	/*
	 * C methods
	 */
};

void luaopen_wlroot(lua_State *, bool in_editor);

#endif
};
