/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_WORKER_H
#define WL_SCRIPTING_MAP_LUA_WORKER_H

#include "logic/map_objects/tribes/worker.h"
#include "scripting/map/lua_bob.h"

namespace LuaMaps {

class LuaWorker : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaWorker);

	LuaWorker() = default;
	explicit LuaWorker(Widelands::Worker& w) : LuaBob(w) {
	}
	explicit LuaWorker(lua_State* L) : LuaBob(L) {
	}
	~LuaWorker() override = default;

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_location(lua_State*);

	/*
	 * Lua methods
	 */
	int evict(lua_State* L);

	/*
	 * C methods
	 */
	CASTED_GET(Worker)
};

}  // namespace LuaMaps

#endif
