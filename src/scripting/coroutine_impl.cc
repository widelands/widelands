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

#include <csetjmp>
#include <boost/lexical_cast.hpp>

#include "logic/player.h"

#include "c_utils.h"
#include "lua_game.h"
#include "lua_map.h"
#include "persistence.h"

#include "coroutine_impl.h"

#include "log.h"

LuaCoroutine_Impl::LuaCoroutine_Impl(lua_State * ms)
	: m_L(ms), m_idx(LUA_REFNIL), m_nargs(0)
{
	m_reference();
}

LuaCoroutine_Impl::~LuaCoroutine_Impl()
{
	m_unreference();
}

int LuaCoroutine_Impl::resume(uint32_t * sleeptime)
{
	int rv = lua_resume(m_L, m_nargs);
	m_nargs = 0;
	int n = lua_gettop(m_L);

	uint32_t sleep_for = 0;
	if (n == 1) {
		sleep_for = luaL_checkint32(m_L, -1);
		lua_pop(m_L, 1);
	}

	if (sleeptime)
		*sleeptime = sleep_for;

	if (rv != 0 && rv != YIELDED) {
		// lua_error() never returns. prints error and exit program imediately
		//return  lua_error(m_L);
		const char * err = lua_tostring(m_L, -1);
		throw LuaError(err);
	}

	return rv;
}

/*
 * Push an argument that will be passed to the coroutine the next time it is
 * resumed
 */
void LuaCoroutine_Impl::push_arg(const Widelands::Player * plr) {
	to_lua<LuaGame::L_Player>(m_L, new LuaGame::L_Player(plr->player_number()));
	m_nargs++;
}
void LuaCoroutine_Impl::push_arg(const Widelands::Coords & coords) {
	to_lua<LuaMap::L_Field>(m_L, new LuaMap::L_Field(coords));
	++m_nargs;
}

#define COROUTINE_DATA_PACKET_VERSION 1
uint32_t LuaCoroutine_Impl::write
	(lua_State * parent, Widelands::FileWrite & fw,
	 Widelands::Map_Map_Object_Saver & mos)
{
	fw.Unsigned8(COROUTINE_DATA_PACKET_VERSION);

	// The current numbers of arguments on the stack
	fw.Unsigned32(m_nargs);

	// Empty table + object to persist on the stack Stack
	lua_newtable(parent);
	lua_pushthread(m_L);
	lua_xmove (m_L, parent, 1);

	return persist_object(parent, fw, mos);
}

void LuaCoroutine_Impl::read
	(lua_State * parent, Widelands::FileRead & fr,
	 Widelands::Map_Map_Object_Loader & mol, uint32_t size)
{
	uint8_t version = fr.Unsigned8();

	if (version != COROUTINE_DATA_PACKET_VERSION)
		throw wexception("Unknown data packet version: %i\n", version);

	// The current numbers of arguments on the stack
	m_nargs = fr.Unsigned32();

	// Empty table + object to persist on the stack Stack
	unpersist_object(parent, fr, mol, size);

	m_L = luaL_checkthread(parent, -1);
	lua_pop(parent, 1);

	// Cache a lua reference to this object so that it does not
	// get garbage collected
	lua_pushthread(m_L);
	m_idx = luaL_ref(m_L, LUA_REGISTRYINDEX);

}

/*
 * ========================================================================
 *                         PRIVATE FUNCTIONS
 * ========================================================================
 */
/**
 * Cache a lua reference to this object so that it does not get garbage
 * collected
 */
void LuaCoroutine_Impl::m_reference() {
	if (m_L) {
		lua_pushthread(m_L);
		m_idx = luaL_ref(m_L, LUA_REGISTRYINDEX);
	}
}
void LuaCoroutine_Impl::m_unreference() {
	luaL_unref(m_L, LUA_REGISTRYINDEX, m_idx);
}


