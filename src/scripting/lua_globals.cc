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

#include <lua.hpp>

#include "logic/game.h"

#include "scripting.h"
#include "c_utils.h"

#include "lua_globals.h"


/* RST
Global functions
======================

The following functions are imported into the global namespace
of all scripts that are running inside widelands. They provide convenient
access to other scripts in other locations, localisation features and more.

*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */


/*
 * ========================================================================
 *                         MODULE FUNCTIONS
 * ========================================================================
 */
/* RST
	.. function:: use(ns, script)

		Includes the script referenced at the caller location. Use this
		to factor your scripts into smaller parts.

		:arg ns:
			The namespace were the imported script resides. Can be any of
				:const:`maps`
					The script is in the ``scripting/`` directory of the current map.

		:type ns: :class:`string`
		:arg script: The filename of the string without the extension ``.lua``.
		:type script: :class:`string`
		:returns: :const:`nil`
*/
static int L_use(lua_State * L) {
	const char* ns = luaL_checkstring(L, -2);
	const char* script = luaL_checkstring(L, -1);

	// remove our argument so that the executed script gets a clear stack
	lua_pop(L, 2);

	get_game(L).lua().run_script(ns, script);

	return 0;
}

const static struct luaL_reg globals [] = {
	{"use", &L_use},
	{0, 0}
};

void luaopen_globals(lua_State * L) {
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, 0, globals);
	lua_pop(L, 1);
}


