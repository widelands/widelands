/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_FLAG_H
#define WL_SCRIPTING_MAP_LUA_FLAG_H

#include "economy/flag.h"
#include "scripting/map/lua_player_immovable.h"

namespace LuaMaps {

class LuaFlag : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaFlag);

	LuaFlag() = default;
	explicit LuaFlag(Widelands::Flag& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaFlag(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaFlag() override = default;

	/*
	 * Properties
	 */
	int get_ware_economy(lua_State* L);
	int get_worker_economy(lua_State* L);
	int get_roads(lua_State* L);
	int get_building(lua_State* L);
	/*
	 * Lua Methods
	 */
	int set_wares(lua_State*);
	int get_wares(lua_State*);
	int get_distance(lua_State*);
	int send_geologist(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(Flag)
};

}  // namespace LuaMaps

#endif
