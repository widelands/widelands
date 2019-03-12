/*
 * Copyright (C) 2006-2019 by the Widelands Development Team
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

#include "scripting/report_error.h"

#include <cassert>

#include "base/wexception.h"
#include "scripting/lua_errors.h"

/*
 * Returns an error to lua. This method never returns as lua_error long jumps.
 */
void report_error(lua_State* L, const char* const fmt, ...) {
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	// Also create a traceback
	lua_getglobal(L, "debug");
	assert(lua_istable(L, -1));
	lua_getfield(L, -1, "traceback");
	assert(lua_isfunction(L, -1));
	lua_pushstring(L, buffer);  // error message
	lua_pushinteger(L, 2);      /* skip this function and traceback */
	lua_call(L, 2, 1);          /* call debug.traceback */

	lua_error(L);  // lua_error never returns.
	NEVER_HERE();
}
