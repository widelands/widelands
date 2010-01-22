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

using namespace Widelands;

/*
 * Functions of game
 */

/*
 * TODO: document me
 */
static int L_call_at(lua_State* L) {
   int nargs = lua_gettop();
   if(nargs < 2)
      report_error("Too little arguments to run_at");

   Game & game = *get_game(L);

   int gametime = luaL_checkint32(L, 1);
   
   // Run the init script, if the map provides on.
   game.enqueue_command(
         new Cmd_LuaCallFunction(game.get_gametime(), "map", "init", true));

}

const static struct luaL_reg wlgame [] = {
	{"call_at", &L_call_at},
	{0, 0}
};

void luaopen_wlgame(lua_State * L) {
	luaL_register(L, "wl.game", wlgame);
}

