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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "scripting/coroutine_impl.h"

#include <csetjmp>

#include <boost/lexical_cast.hpp>

#include "log.h"
#include "logic/player.h"
#include "scripting/c_utils.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"
#include "scripting/persistence.h"

LuaCoroutine_Impl::LuaCoroutine_Impl(lua_State * ms)
	: m_L(ms), m_idx(LUA_REFNIL), m_ninput_args(0), m_nreturn_values(0)
{
	m_reference();
}

LuaCoroutine_Impl::~LuaCoroutine_Impl()
{
	m_unreference();
}

int LuaCoroutine_Impl::resume()
{
	int rv = lua_resume(m_L, m_ninput_args);
	m_ninput_args = 0;
	m_nreturn_values = lua_gettop(m_L);

	if (rv != 0 && rv != YIELDED) {
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
	m_ninput_args++;
}
void LuaCoroutine_Impl::push_arg(const Widelands::Coords & coords) {
	to_lua<LuaMap::L_Field>(m_L, new LuaMap::L_Field(coords));
	++m_ninput_args;
}
void LuaCoroutine_Impl::push_arg(const Widelands::Building_Descr* building_descr) {
	assert(building_descr != nullptr);
	to_lua<LuaMap::L_BuildingDescription>(m_L, new LuaMap::L_BuildingDescription(building_descr));
	++m_ninput_args;
}

std::string LuaCoroutine_Impl::pop_string() {
	if (!m_nreturn_values) {
		return "";
	}
	if (!lua_isstring(m_L, -1)) {
		throw LuaError("pop_string(), but no string on the stack.");
	}
	const std::string return_value = lua_tostring(m_L, -1);
	lua_pop(m_L, 1);
	--m_nreturn_values;
	return return_value;
}

uint32_t LuaCoroutine_Impl::pop_uint32() {
	if (!m_nreturn_values) {
		return 0;
	}
	if (!lua_isnumber(m_L, -1)) {
		throw LuaError("pop_string(), but no integer on the stack.");
	}
	const uint32_t return_value = luaL_checkuint32(m_L, -1);
	lua_pop(m_L, 1);
	--m_nreturn_values;
	return return_value;
}


#define COROUTINE_DATA_PACKET_VERSION 2
uint32_t LuaCoroutine_Impl::write
	(lua_State * parent, Widelands::FileWrite & fw,
	 Widelands::Map_Map_Object_Saver & mos)
{
	fw.Unsigned8(COROUTINE_DATA_PACKET_VERSION);

	fw.Unsigned32(m_ninput_args);
	fw.Unsigned32(m_nreturn_values);

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

	if (version > COROUTINE_DATA_PACKET_VERSION)
		throw wexception("Unknown data packet version: %i\n", version);

	// The current numbers of arguments on the stack
	m_ninput_args = fr.Unsigned32();
	if (version > 1) {
		m_nreturn_values = fr.Unsigned32();
	}

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
