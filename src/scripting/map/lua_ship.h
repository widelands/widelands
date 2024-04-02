/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_SHIP_H
#define WL_SCRIPTING_MAP_LUA_SHIP_H

#include "logic/map_objects/tribes/ship.h"
#include "scripting/map/lua_bob.h"

namespace LuaMaps {

class LuaShip : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaShip);

	LuaShip() = default;
	explicit LuaShip(Widelands::Ship& s) : LuaBob(s) {
	}
	explicit LuaShip(lua_State* L) : LuaBob(L) {
	}
	~LuaShip() override = default;

	/*
	 * Properties
	 */
	int get_debug_ware_economy(lua_State* L);
	int get_debug_worker_economy(lua_State* L);
	int get_last_portdock(lua_State* L);
	int get_destination(lua_State* L);
	int get_state(lua_State* L);
	int get_type(lua_State* L);
	int get_scouting_direction(lua_State* L);
	int set_scouting_direction(lua_State* L);
	int get_island_explore_direction(lua_State* L);
	int set_island_explore_direction(lua_State* L);
	int get_shipname(lua_State* L);
	int set_shipname(lua_State* L);
	int get_capacity(lua_State* L);
	int set_capacity(lua_State* L);
	/*
	 * Lua methods
	 */
	int get_wares(lua_State* L);
	int get_workers(lua_State* L);
	int build_colonization_port(lua_State* L);
	int make_expedition(lua_State* L);
	int refit(lua_State* L);

	/*
	 * C methods
	 */
	CASTED_GET(Ship)
};

}  // namespace LuaMaps

#endif
