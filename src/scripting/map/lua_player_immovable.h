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

#ifndef WL_SCRIPTING_MAP_LUA_PLAYER_IMMOVABLE_H
#define WL_SCRIPTING_MAP_LUA_PLAYER_IMMOVABLE_H

#include "scripting/map/lua_base_immovable.h"

namespace LuaMaps {

class LuaPlayerImmovable : public LuaBaseImmovable {
public:
	LUNA_CLASS_HEAD(LuaPlayerImmovable);

	LuaPlayerImmovable() = default;
	explicit LuaPlayerImmovable(Widelands::PlayerImmovable& mo) : LuaBaseImmovable(mo) {
	}
	explicit LuaPlayerImmovable(lua_State* L) : LuaBaseImmovable(L) {
	}
	~LuaPlayerImmovable() override = default;

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_debug_ware_economy(lua_State* L);
	int get_debug_worker_economy(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(PlayerImmovable)
};

}  // namespace LuaMaps

#endif
