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

#include <boost/lexical_cast.hpp>

#include "coroutine_impl.h"

#include "c_utils.h"
#include "persistence.h"

LuaCoroutine_Impl::LuaCoroutine_Impl(lua_State * ms)
	: m_L(ms), m_idx(LUA_REFNIL)
{
	m_reference();
}

LuaCoroutine_Impl::~LuaCoroutine_Impl()
{
	m_unreference();
}

int LuaCoroutine_Impl::resume(uint32_t * sleeptime)
{
	int rv = lua_resume(m_L, 0);
	int n = lua_gettop(m_L);

	uint32_t sleep_for = 0;
	if (n == 1) {
		sleep_for = luaL_checkint32(m_L, -1);
		lua_pop(m_L, 1);
	}

	if (sleeptime)
		*sleeptime = sleep_for;

	if (rv != 0 && rv != YIELDED)
		return lua_error(m_L);

	return rv;
}

static const char * m_persistent_globals[] = {
	"coroutine.yield", 0,
};
uint32_t LuaCoroutine_Impl::write
	(lua_State * parent, Widelands::FileWrite & fw,
	 Widelands::Map_Map_Object_Saver & mos)
{
	// Empty table + object to persist on the stack Stack
	lua_newtable(parent);
	lua_pushthread(m_L);
	lua_xmove (m_L, parent, 1);

	return persist_object(parent, m_persistent_globals, fw, mos);
}

void LuaCoroutine_Impl::read
	(lua_State * parent, Widelands::FileRead & fr,
	 Widelands::Map_Map_Object_Loader & mol, uint32_t size)
{
	// Empty table + object to persist on the stack Stack
	unpersist_object(parent, m_persistent_globals, fr, mol, size);

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


