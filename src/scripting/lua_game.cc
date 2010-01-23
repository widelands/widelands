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

#include "logic/cmd_luafunction.h"

#include "c_utils.h"
#include "coroutine_impl.h"
#include "lua_map.h"
#include "scripting.h"

#include "lua_game.h"


using namespace Widelands;

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */
/*
 * TODO: document me
 */
const char L_Player::className[] = "Player";
const MethodType<L_Player> L_Player::Methods[] = {
	METHOD(L_Player, build_flag),
	METHOD(L_Player, force_building),

	{0, 0},
};
const PropertyType<L_Player> L_Player::Properties[] = {
	PROP_RO(L_Player, number),
	{0, 0, 0},
};

void L_Player::__persist(lua_State * L) {
	PERS_UINT32("player", m_pl);
}
void L_Player::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", m_pl);
}


/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/*
 * TODO: document me
 */
int L_Player::get_number(lua_State * L) {
	lua_pushuint32(L, m_pl);
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
// TODO: document me
// TODO: should return Flag
int L_Player::build_flag(lua_State * L) {
	L_Field * c = *get_user_class<L_Field>(L, -1);

	m_get(get_game(L)).build_flag(c->coords());
	return 0;
}

// TODO: document me
// TODO: should return Building
int L_Player::force_building(lua_State * L) {
	const char * name = luaL_checkstring(L, - 2);
	L_Field * c = *get_user_class<L_Field>(L, -1);

	Building_Index i = m_get(get_game(L)).tribe().building_index(name);

	m_get(get_game(L)).force_building(c->coords(), i, 0, 0, Soldier_Counts());

	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */



/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */
/*
 * TODO: document me
 */
static int L_get_time(lua_State * L) {
	Game & game = get_game(L);
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
	Game & game = get_game(L);

	lua_pop(L, 1); // Remove coroutine from stack

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
	lua_pop(L, 1); // pop the table

	register_class<L_Player>(L, "game");
}

