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

#ifndef WL_SCRIPTING_LUA_COROUTINE_H
#define WL_SCRIPTING_LUA_COROUTINE_H

#include <memory>

#include "scripting/lua.h"

class FileRead;
class FileWrite;
class LuaTable;

namespace Widelands {
class Player;
struct Coords;
}  // namespace Widelands

// Easy handling of function objects and coroutines.
class LuaCoroutine {
public:
	// The state of the coroutine, which can either be yielded, i.e. it expects
	// to be resumed again or done which means that it will not do any more work
	// and can be deleted.
	enum { DONE = 0, YIELDED = LUA_YIELD };

	explicit LuaCoroutine(lua_State* ms);
	virtual ~LuaCoroutine();

	// Returns either 'DONE' or 'YIELDED'.
	int get_status();

	// Resumes the coroutine and returns it's state after it did its execution.
	int resume();

	// Push the given arguments onto the Lua stack, so that a Coroutine can
	// receive them. This is for example used in the initialization scripts or
	// in hooks.
	void push_arg(const Widelands::Player*);
	void push_arg(const Widelands::Coords&);
	void push_arg(const std::string&);
	void push_arg(int);

	// Accesses the returned values from the run of the coroutine.
	uint32_t pop_uint32();
	std::string pop_string();
	std::unique_ptr<LuaTable> pop_table();

private:
	friend class LuaGameInterface;

	// Input/Output for coroutines. Do not call directly, instead use
	// LuaGameInterface methods for this.
	void write(FileWrite&) const;
	void read(lua_State*, FileRead&);

	lua_State* lua_state_;
	uint32_t idx_;
	uint32_t nargs_;
	uint32_t ninput_args_;
	uint32_t nreturn_values_;
};

#endif  // end of include guard: WL_SCRIPTING_LUA_COROUTINE_H
