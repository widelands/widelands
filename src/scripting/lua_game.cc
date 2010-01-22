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

#include "lua_game.h"

#include "c_utils.h"
#include "coroutine_impl.h"
#include "logic/cmd_luafunction.h"
#include "scripting.h"


using namespace Widelands;

/*
 * ========================================================================
 *                                CLASSES
 * ========================================================================
 */

/*
 * Functions of game
 */
/*
 * TODO: document me
 */
static int L_get_time(lua_State * L) {
	Game & game = *get_game(L);
	lua_pushint32(L, game.get_gametime());
	return 1;
}


/*
 * TODO: document me
 */
static int L_run_coroutine(lua_State * L) {
	int nargs = lua_gettop(L);
	if (nargs < 1)
		report_error(L, "Too little arguments to run_at");

	LuaCoroutine * cr = new LuaCoroutine_Impl(luaL_checkthread(L, -1));
	Game & game = *get_game(L);

	game.enqueue_command
		(new Widelands::Cmd_LuaFunction(game.get_gametime(), cr));

	return 0;
}

const static struct luaL_reg wlgame [] = {
	{"run_coroutine", &L_run_coroutine},
	{"get_time", &L_get_time},
	{0, 0}
};

void luaopen_wlgame(lua_State * L) {
	luaL_register(L, "wl.game", wlgame);
}

