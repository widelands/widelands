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

#ifndef LUA_BASES_H
#define LUA_BASES_H

#include "scripting/eris/lua.hpp"
#include "scripting/luna.h"

namespace LuaBases {

/*
 * Base class for all classes in wl.base
 */
class L_BasesModuleClass : public LunaClass {
	public:
		const char * get_modulename() override {return "bases";}
};


class L_EditorGameBase : public L_BasesModuleClass {
public:
	LUNA_CLASS_HEAD(L_EditorGameBase);

	L_EditorGameBase() {}
	L_EditorGameBase(lua_State * L) {
		report_error(L, "Cannot instantiate a 'EditorGameBase' directly!");
	}
	virtual ~L_EditorGameBase() {}

	virtual void __persist(lua_State * L) override;
	virtual void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_map(lua_State *);
	int get_players(lua_State *);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
};


class L_PlayerBase : public L_BasesModuleClass {
	Widelands::Player_Number m_pl;
	enum {NONE = -1};

public:
	LUNA_CLASS_HEAD(L_PlayerBase);


	L_PlayerBase() : m_pl(NONE) {}
	L_PlayerBase(lua_State * L) : m_pl(NONE) {
		report_error(L, "Cannot instantiate a 'PlayerBase' directly!");
	}
	L_PlayerBase(Widelands::Player_Number n) {
		m_pl = n;
	}
	virtual ~L_PlayerBase() {}

	virtual void __persist(lua_State * L) override;
	virtual void __unpersist(lua_State * L) override;

	/*
	 * Properties
	 */
	int get_number(lua_State * L);
	int get_tribe_name(lua_State * L);

	/*
	 * Lua methods
	 */
	int __eq(lua_State * L);
	int __tostring(lua_State * L);
	int place_flag(lua_State * L);
	int place_road(lua_State * L);
	int place_building(lua_State * L);
	int place_bob(lua_State* L);
	int conquer(lua_State * L);
	int get_workers(lua_State* L);
	int get_wares(lua_State* L);

	/*
	 * C methods
	 */
	Widelands::Player & get(lua_State * L, Widelands::Editor_Game_Base &);

protected:
	inline Widelands::Player_Number player_number() {return m_pl;}
};

void luaopen_wlbases(lua_State *);

};

#endif
