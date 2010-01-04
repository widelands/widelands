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

#include "c_utils.h"
#include "log.h"
#include "logic/game.h"
#include "wui/interactive_player.h"

#include "lua_debug.h"

// LUAMODULE wl.debug

/*
 * Intern definitions of Lua Functions
 */
/*
 * TODO: describe these with doxygen descriptions
 *
 * LUAFUNC: log
 */
static int L_log(lua_State * l) {
	const char * x = luaL_checkstring(l, 1);

	log("%s\n", x);

	return 0;
}

/*
 * TODO: document me
 */
static int L_setSeeAll(lua_State * l) {
	bool bval = luaL_checkint(l, 1);

	get_game(l)->get_ipl()->player().set_see_all(bval);

	return 0;
}

const static struct luaL_reg wldebug [] = {
	{"log", &L_log},
	{"set_see_all", &L_setSeeAll},
	{NULL, NULL}
};


void luaopen_wldebug(lua_State * l) {
	luaL_register(l, "wl.debug", wldebug);
}


