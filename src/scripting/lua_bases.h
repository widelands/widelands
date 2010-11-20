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

#ifndef LUA_BASES_H
#define LUA_BASES_H

#include <lua.hpp>

#include "luna.h"

namespace Widelands {struct Player;}

namespace LuaBases {

/*
 * Base class for all classes in wl.base
 */
struct L_BasesModuleClass : public LunaClass {
	char const * get_modulename() {return "bases";}
};


struct L_EditorGameBase : public L_BasesModuleClass {
	LUNA_CLASS_HEAD(L_EditorGameBase);

	L_EditorGameBase() {}
	L_EditorGameBase(lua_State * const L) {
		report_error(L, "Cannot instantiate a 'EditorGameBase' directly!");
	}
	virtual ~L_EditorGameBase() {}

	virtual void __persist(lua_State *);
	virtual void __unpersist(lua_State *);

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


struct L_PlayerBase : public L_BasesModuleClass {
	LUNA_CLASS_HEAD(L_PlayerBase);


	L_PlayerBase() : m_pl(NONE) {}
	L_PlayerBase(lua_State * const L) {
		report_error(L, "Cannot instantiate a 'PlayerBase' directly!");
	}
	L_PlayerBase(Widelands::Player_Number const n) {
		m_pl = n;
	}
	virtual ~L_PlayerBase() {}

	virtual void __persist(lua_State *);
	virtual void __unpersist(lua_State *);

	/*
	 * Properties
	 */
	int get_number(lua_State *);
	int get_tribe(lua_State *);

	/*
	 * Lua methods
	 */
	int __eq(lua_State *);
	int __tostring(lua_State *);
	int place_flag(lua_State *);
	int place_road(lua_State *);
	int place_building(lua_State *);
	int conquer(lua_State *);

	/*
	 * C methods
	 */
	Widelands::Player & get(lua_State *, Widelands::Editor_Game_Base &);

protected:
	inline Widelands::Player_Number player_number() {return m_pl;}

private:
	Widelands::Player_Number m_pl;
	enum {NONE = -1};
};

void luaopen_wlbases(lua_State *);

}

#endif
