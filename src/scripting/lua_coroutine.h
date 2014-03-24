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

#ifndef LUA_COROUTINE_H
#define LUA_COROUTINE_H

#include <stdint.h>

#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "scripting/eris/lua.hpp"

namespace Widelands {
class Map_Map_Object_Loader;
class Player;
struct Coords;
struct Map_Map_Object_Saver;
}  // namespace Widelands

// Easy handling of function objects and coroutines.
class LuaCoroutine {
public:
	// The state of the coroutine, which can either be yielded, i.e. it expects
	// to be resumed again or done which means that it will not do any more work
	// and can be deleted.
	enum {
		DONE = 0,
		YIELDED = LUA_YIELD
	};

	LuaCoroutine(lua_State* L);
	virtual ~LuaCoroutine();

	// Returns either 'DONE' or 'YIELDED'.
	int get_status();

	// Resumes the coroutine and returns it's state after it did its execution.
	// If 'sleeptime' is not null, it will contain the time in milliseconds the
	// lua code requested for this coroutine to sleep before it should be
	// resumed again.
	int resume(uint32_t* sleeptime = nullptr);

	// Push the given arguments onto the Lua stack, so that a Coroutine can
	// receive them. This is for example used in the initialization scripts or
	// in hooks.
	void push_arg(const Widelands::Player*);
	void push_arg(const Widelands::Coords&);

private:
	friend class LuaGameInterface;

	// Input/Output for coroutines. Do not call directly, instead use
	// LuaGameInterface methods for this.
	uint32_t write(lua_State*, Widelands::FileWrite&, Widelands::Map_Map_Object_Saver&);
	void read(lua_State*, Widelands::FileRead&, Widelands::Map_Map_Object_Loader&, uint32_t);

	lua_State* m_L;
	uint32_t m_idx;
	uint32_t m_nargs;
};

#endif /* end of include guard: LUA_COROUTINE_H */
