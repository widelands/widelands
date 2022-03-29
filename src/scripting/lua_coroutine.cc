/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scripting/lua_coroutine.h"

#include <memory>

#include "io/fileread.h"
#include "io/filewrite.h"
#include "logic/game_data_error.h"
#include "scripting/lua_errors.h"
#include "scripting/lua_game.h"
#include "scripting/lua_globals.h"
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
	lua_pushthread(L);  // S: __references thread
	uint32_t idx = luaL_ref(L, -2);
	lua_pop(L, 1);  // S:
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

LuaCoroutine::LuaCoroutine(lua_State* ms)
   : lua_state_(ms), idx_(LUA_REFNIL), ninput_args_(0), nreturn_values_(0) {
	if (lua_state_ != nullptr) {
		idx_ = reference_coroutine(lua_state_);
	}
}

LuaCoroutine::~LuaCoroutine() {
	unreference_coroutine(lua_state_, idx_);
}

int LuaCoroutine::get_status() {
	return lua_status(lua_state_);
}

int LuaCoroutine::resume() {
	int rv = lua_resume(lua_state_, nullptr, ninput_args_);
	ninput_args_ = 0;
	nreturn_values_ = lua_gettop(lua_state_);

	if (rv != 0 && rv != YIELDED) {
		throw LuaError(lua_tostring(lua_state_, -1));
	}

	return rv;
}

void LuaCoroutine::push_arg(const Widelands::Player* plr) {
	to_lua<LuaGame::LuaPlayer>(lua_state_, new LuaGame::LuaPlayer(plr->player_number()));
	ninput_args_++;
}

void LuaCoroutine::push_arg(const Widelands::Coords& coords) {
	to_lua<LuaMaps::LuaField>(lua_state_, new LuaMaps::LuaField(coords));
	++nargs_;
	++ninput_args_;
}

void LuaCoroutine::push_arg(const std::string& string) {
	assert(!string.empty());
	lua_pushstring(lua_state_, string);
	++ninput_args_;
}

void LuaCoroutine::push_arg(const int number) {
	lua_pushinteger(lua_state_, number);
	++ninput_args_;
}

std::string LuaCoroutine::pop_string() {
	if (nreturn_values_ == 0u) {
		return "";
	}
	if (lua_isstring(lua_state_, -1) == 0) {
		throw LuaError("pop_string(), but no string on the stack.");
	}
	std::string return_value = lua_tostring(lua_state_, -1);
	lua_pop(lua_state_, 1);
	--nreturn_values_;
	return return_value;
}

uint32_t LuaCoroutine::pop_uint32() {
	if (nreturn_values_ == 0u) {
		return 0;
	}
	if (lua_isinteger(lua_state_, -1) == 0) {
		throw LuaError("pop_uint32(), but no integer on the stack.");
	}
	const uint32_t return_value = luaL_checkuint32(lua_state_, -1);
	lua_pop(lua_state_, 1);
	--nreturn_values_;
	return return_value;
}

std::unique_ptr<LuaTable> LuaCoroutine::pop_table() {
	std::unique_ptr<LuaTable> result(nullptr);
	if (nreturn_values_ == 0u) {
		return result;
	}
	result.reset(new LuaTable(lua_state_));
	lua_pop(lua_state_, 1);
	--nreturn_values_;
	return result;
}

constexpr uint8_t kCoroutineDataPacketVersion = 5;
void LuaCoroutine::write(FileWrite& fw) const {
	fw.unsigned_8(kCoroutineDataPacketVersion);

	fw.unsigned_32(ninput_args_);
	fw.unsigned_32(nreturn_values_);
	fw.unsigned_32(idx_);

	LuaGlobals::write_textdomain_stack(fw, lua_state_);
}

void LuaCoroutine::read(lua_State* parent, FileRead& fr) {
	const uint8_t packet_version = fr.unsigned_8();

	if (packet_version > kCoroutineDataPacketVersion || packet_version < 4) {
		throw Widelands::UnhandledVersionError(
		   "LuaCoroutine", packet_version, kCoroutineDataPacketVersion);
	}

	ninput_args_ = fr.unsigned_32();
	nreturn_values_ = fr.unsigned_32();
	idx_ = fr.unsigned_32();

	lua_getglobal(parent, kReferenceTableName);
	lua_rawgeti(parent, -1, idx_);
	lua_state_ = luaL_checkthread(parent, -1);
	lua_pop(parent, 2);

	// TODO(Nordfriese): Savegame compatibility
	if (packet_version >= 5) {
		LuaGlobals::read_textdomain_stack(fr, lua_state_);
	}
}
