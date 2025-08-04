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

#ifndef WL_SCRIPTING_MAP_LUA_BUILDING_H
#define WL_SCRIPTING_MAP_LUA_BUILDING_H

#include "logic/map_objects/tribes/building.h"
#include "scripting/map/lua_player_immovable.h"

namespace LuaMaps {

class LuaBuilding : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaBuilding);

	LuaBuilding() = default;
	explicit LuaBuilding(Widelands::Building& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaBuilding(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaBuilding() override = default;

	/*
	 * Properties
	 */
	int get_flag(lua_State* L);
	int get_destruction_blocked(lua_State* L);
	int set_destruction_blocked(lua_State* L);

	/*
	 * Lua Methods
	 */
	int dismantle(lua_State* L);
	int enhance(lua_State* L);
	int subscribe_to_muted(lua_State* L);

	/*
	 * C Methods
	 */
	CASTED_GET(Building)
};

}  // namespace LuaMaps

#endif
