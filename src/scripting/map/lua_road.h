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

#ifndef WL_SCRIPTING_MAP_LUA_ROAD_H
#define WL_SCRIPTING_MAP_LUA_ROAD_H

#include "economy/roadbase.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "scripting/map/lua_player_immovable.h"

namespace LuaMaps {

class LuaRoad : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaRoad);

	LuaRoad() = default;
	explicit LuaRoad(Widelands::RoadBase& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaRoad(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaRoad() override = default;

	/*
	 * Properties
	 */
	int get_end_flag(lua_State* L);
	int get_length(lua_State* L);
	int get_road_type(lua_State* L);
	int get_start_flag(lua_State* L);
	int get_valid_workers(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_workers(lua_State* L);
	int set_workers(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(RoadBase)
	static bool create_new_worker(lua_State* L,
	                              Widelands::RoadBase& r,
	                              Widelands::EditorGameBase&,
	                              const Widelands::WorkerDescr*);
};

}  // namespace LuaMaps

#endif
