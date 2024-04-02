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

#ifndef WL_SCRIPTING_MAP_LUA_SOLDIER_H
#define WL_SCRIPTING_MAP_LUA_SOLDIER_H

#include "logic/map_objects/tribes/soldier.h"
#include "scripting/map/lua_worker.h"

namespace LuaMaps {

class LuaSoldier : public LuaWorker {
public:
	LUNA_CLASS_HEAD(LuaSoldier);

	LuaSoldier() = default;
	explicit LuaSoldier(Widelands::Soldier& w) : LuaWorker(w) {
	}
	explicit LuaSoldier(lua_State* L) : LuaWorker(L) {
	}
	~LuaSoldier() override = default;

	/*
	 * Properties
	 */
	int get_attack_level(lua_State*);
	int get_defense_level(lua_State*);
	int get_health_level(lua_State*);
	int get_evade_level(lua_State*);
	int get_current_health(lua_State*);
	int set_current_health(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(Soldier)
};

}  // namespace LuaMaps

#endif
