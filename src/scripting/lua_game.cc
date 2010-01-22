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

#include "scripting.h"
#include "c_utils.h"

#include "logic/cmd_luafunction.h"

using namespace Widelands;

/*
 * Functions of game
 */

/*
 * TODO: document me
 */
static int L_run_coroutine(lua_State * L) {
	log("In corotine!\n");
	int nargs = lua_gettop(L);
	log("nargs: %i\n", nargs);
	if (nargs < 1)
		report_error(L, "Too little arguments to run_at");

	LuaCoroutine * cr = create_lua_state(L)->pop_coroutine();
	Game & game = *get_game(L);

	game.enqueue_command
		(new Widelands::Cmd_LuaFunction(game.get_gametime(), cr));

	return 0;
}

const static struct luaL_reg wlgame [] = {
	{"run_coroutine", &L_run_coroutine},
	{0, 0}
};

void luaopen_wlgame(lua_State * L) {
	luaL_register(L, "wl.game", wlgame);
}

