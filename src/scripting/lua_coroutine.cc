/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#include "io/fileread.h"
#include "io/filewrite.h"
#include "scripting/lua_errors.h"
#include "scripting/lua_game.h"
#include "scripting/lua_map.h"

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
	: m_L(ms), m_idx(LUA_REFNIL), m_ninput_args(0), m_nreturn_values(0)
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

int LuaCoroutine::resume()
{
	int rv = lua_resume(m_L, nullptr, m_ninput_args);
	m_ninput_args = 0;
	m_nreturn_values = lua_gettop(m_L);

	if (rv != 0 && rv != YIELDED) {
		throw LuaError(lua_tostring(m_L, -1));
	}

	return rv;
}

void LuaCoroutine::push_arg(const Widelands::Player * plr) {
	to_lua<LuaGame::LuaPlayer>(m_L, new LuaGame::LuaPlayer(plr->player_number()));
	m_ninput_args++;
}

void LuaCoroutine::push_arg(const Widelands::Coords & coords) {
	to_lua<LuaMaps::LuaField>(m_L, new LuaMaps::LuaField(coords));
	++m_nargs;
	++m_ninput_args;
}

void LuaCoroutine::push_arg(const Widelands::BuildingDescr* building_descr) {
	assert(building_descr != nullptr);
	to_lua<LuaMaps::LuaBuildingDescription>(m_L, new LuaMaps::LuaBuildingDescription(building_descr));
	++m_ninput_args;
}

void LuaCoroutine::push_arg(const Widelands::WareDescr* ware_descr) {
	assert(ware_descr != nullptr);
	to_lua<LuaMaps::LuaWareDescription>(m_L, new LuaMaps::LuaWareDescription(ware_descr));
	++m_ninput_args;
}

void LuaCoroutine::push_arg(const Widelands::WorkerDescr* worker_descr) {
	assert(worker_descr != nullptr);
	to_lua<LuaMaps::LuaWorkerDescription>(m_L, new LuaMaps::LuaWorkerDescription(worker_descr));
	++m_ninput_args;
}

void LuaCoroutine::push_arg(const std::string& string) {
	assert(!string.empty());
	lua_pushstring(m_L, string);
	++m_ninput_args;
}

std::string LuaCoroutine::pop_string() {
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

uint32_t LuaCoroutine::pop_uint32() {
	if (!m_nreturn_values) {
		return 0;
	}
	if (!lua_isnumber(m_L, -1)) {
		throw LuaError("pop_uint32(), but no integer on the stack.");
	}
	const uint32_t return_value = luaL_checkuint32(m_L, -1);
	lua_pop(m_L, 1);
	--m_nreturn_values;
	return return_value;
}

constexpr uint8_t kCoroutineDataPacketVersion = 3;
void LuaCoroutine::write(FileWrite& fw) {
	fw.unsigned_8(kCoroutineDataPacketVersion);

	fw.unsigned_32(m_ninput_args);
	fw.unsigned_32(m_nreturn_values);
	fw.unsigned_32(m_idx);
}

void LuaCoroutine::read(lua_State* parent, FileRead& fr) {
	uint8_t version = fr.unsigned_8();

	if (version != kCoroutineDataPacketVersion)
		throw wexception("Unhandled data packet version: %i\n", version);

	m_ninput_args = fr.unsigned_32();
	m_nreturn_values = fr.unsigned_32();
	m_idx = fr.unsigned_32();

	lua_getglobal(parent, kReferenceTableName);
	lua_rawgeti(parent, -1, m_idx);
	m_L = luaL_checkthread(parent, -1);
	lua_pop(parent, 2);
}
