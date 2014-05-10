/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "scripting/lua_coroutine.h"

#include "scripting/c_utils.h"
#include "scripting/lua_errors.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"
#include "scripting/luna.h"
#include "scripting/persistence.h"

namespace {

const char* kReferenceTableName = "__coroutine_registry";

// Push a reference to this coroutine into a private table so that it will not
// get garbage collected. We do not use the registry here, so that this table
// is saved with the rest of the state and we can use it after load to restore
// our coroutines. Returns the index of the reference.
uint32_t reference_coroutine(lua_State* L) {
	assert(L != nullptr);
	lua_getglobal(L, kReferenceTableName);
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_newtable(L);
		lua_setglobal(L, kReferenceTableName);
		lua_getglobal(L, kReferenceTableName);
	}
	// S: __references
	lua_pushthread(L); // S: __references thread
	uint32_t idx = luaL_ref(L, -2);
	lua_pop(L, 1); // S:
	return idx;
}

// Unreference the coroutine with the given index in the registry again. I might then get garbage
// collected.
void unreference_coroutine(lua_State* L, uint32_t idx) {
	lua_getglobal(L, kReferenceTableName);
	luaL_unref(L, -1, idx);
	lua_pop(L, 1);
}

}  // namespace

LuaCoroutine::LuaCoroutine(lua_State * ms)
	: m_L(ms), m_idx(LUA_REFNIL), m_nargs(0)
{
	if (m_L) {
		m_idx = reference_coroutine(m_L);
	}
}

LuaCoroutine::~LuaCoroutine() {
	unreference_coroutine(m_L, m_idx);
}

int LuaCoroutine::get_status() {
	return lua_status(m_L);
}

int LuaCoroutine::resume(uint32_t * sleeptime)
{
	int rv = lua_resume(m_L, nullptr, m_nargs);
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
		throw LuaError(lua_tostring(m_L, -1));
	}

	return rv;
}

void LuaCoroutine::push_arg(const Widelands::Player * plr) {
	to_lua<LuaGame::L_Player>(m_L, new LuaGame::L_Player(plr->player_number()));
	m_nargs++;
}

void LuaCoroutine::push_arg(const Widelands::Coords & coords) {
	to_lua<LuaMap::L_Field>(m_L, new LuaMap::L_Field(coords));
	++m_nargs;
}

#define COROUTINE_DATA_PACKET_VERSION 2
void LuaCoroutine::write(Widelands::FileWrite& fw) {
	fw.Unsigned8(COROUTINE_DATA_PACKET_VERSION);

	// The current numbers of arguments on the stack
	fw.Unsigned32(m_nargs);
	fw.Unsigned32(m_idx);
}

void LuaCoroutine::read(lua_State* parent, FileRead& fr) {
	uint8_t version = fr.Unsigned8();

	if (version != COROUTINE_DATA_PACKET_VERSION)
		throw wexception("Unhandled data packet version: %i\n", version);

	m_nargs = fr.Unsigned32();
	m_idx = fr.Unsigned32();

	lua_getglobal(parent, kReferenceTableName);
	lua_rawgeti(parent, -1, m_idx);
	m_L = luaL_checkthread(parent, -1);
	lua_pop(parent, 2);
}
