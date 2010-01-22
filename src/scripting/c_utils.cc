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

#include "c_utils.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>

Widelands::Game * get_game(lua_State * const L) {
	lua_pushstring(L, "game");
	lua_gettable(L, LUA_REGISTRYINDEX);

	Widelands::Game * g = static_cast<Widelands::Game *>(lua_touserdata(L, -1));
	lua_pop(L, 1); // pop this userdata

	return g;
}

/*
 * Returns an error to lua. Returns 0
 */
int report_error(lua_State * L, const char * const fmt, ...) {
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	lua_pushstring(L, buffer);

	lua_error(L);

	return 0;
}
