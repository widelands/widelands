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

namespace LuaBases {

/*
 * Base class for all classes in wl.base
 */
class L_BasesModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "bases";}
};


class L_PlayerBase : public L_BasesModuleClass {
	Widelands::Player_Number m_pl;
	enum {NONE = -1};

public:
	LUNA_CLASS_HEAD(L_PlayerBase);


	L_PlayerBase() : m_pl(NONE) {}
	L_PlayerBase(lua_State * L) {
		report_error(L, "Cannot instantiate a 'PlayerBase' directly!");
	}
	L_PlayerBase(Widelands::Player_Number n) {
		m_pl = n;
	}
	virtual ~L_PlayerBase() {}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_number(lua_State * L);
	int get_tribe(lua_State * L);

	/*
	 * Lua methods
	 */
	int __eq(lua_State * L);

	/*
	 * C methods
	 */
	Widelands::Player & get(lua_State * L, Widelands::Editor_Game_Base &);

protected:
	inline Widelands::Player_Number player_number() { return m_pl; }
};

void luaopen_wlbases(lua_State *);

};

#endif
