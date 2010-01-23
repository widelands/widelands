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

#ifndef LUA_GAME_H
#define LUA_GAME_H

#include <lua.hpp>

#include "logic/player.h"

#include "luna.h"

/*
 * Base class for all classes in wl.game
 */
class L_GameModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "game";}
};

class L_Player : public L_GameModuleClass {
	Widelands::Player_Number m_pl;
	enum {NONE = -1};

public:
	LUNA_CLASS_HEAD(L_Player);

	L_Player() : m_pl(NONE) {}
	L_Player(lua_State * L) {
		m_pl = luaL_checkuint32(L, -1);
	}

	virtual void __persist(lua_State * L);
	virtual void __unpersist(lua_State * L);

	/*
	 * Properties
	 */
	int get_number(lua_State * L);

	/*
	 * Lua methods
	 */
	int build_flag(lua_State * L);
	int force_building(lua_State * L);

	/*
	 * C methods
	 */
private:
	Widelands::Player & m_get(Widelands::Game & game) {return game.player(m_pl);}
};

void luaopen_wlgame(lua_State *);

#endif

