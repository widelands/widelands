/*
 * Copyright (C) 2006-2009 by the Widelands Development Team
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


extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#include "log.h"

#include "lua_debug.h"
#include "lua_map.h"
#include "scripting.h"


LuaInterface::LuaInterface(Widelands::Game * game) :
	m_last_error("") {
	m_luastate = lua_open();

	// We need the basis library and the math
	luaopen_base(m_luastate);
	luaopen_math(m_luastate);

	// Now our own
	luaopen_wldebug(m_luastate);
	luaopen_wlmap(m_luastate);

	// Push the game onto the stac
	lua_pushstring(m_luastate, "game");
	lua_pushlightuserdata(m_luastate, static_cast<void *>(game));
	lua_settable(m_luastate, LUA_REGISTRYINDEX);
}

LuaInterface::~LuaInterface() {
	lua_close(m_luastate);
}

int LuaInterface::interpret_string(std::string cmd) {
	log("In LuaInterface::interpret_string:\n");
	log(" <%s>\n", cmd.c_str());

	int rv = luaL_dostring(m_luastate, cmd.c_str());
	if (rv)
	{
		 const char * msg = lua_tostring(m_luastate, -1);
		 m_last_error = msg;
		 lua_pop(m_luastate, 1);
	}
	return rv;
}

