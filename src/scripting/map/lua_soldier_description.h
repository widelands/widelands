/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_SOLDIER_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_SOLDIER_DESCRIPTION_H

#include "logic/map_objects/tribes/soldier.h"
#include "scripting/map/lua_worker_description.h"

namespace LuaMaps {

class LuaSoldierDescription : public LuaWorkerDescription {
public:
	LUNA_CLASS_HEAD(LuaSoldierDescription);

	~LuaSoldierDescription() override = default;

	LuaSoldierDescription() = default;
	explicit LuaSoldierDescription(const Widelands::SoldierDescr* const soldierdescr)
	   : LuaWorkerDescription(soldierdescr) {
	}
	explicit LuaSoldierDescription(lua_State* L) : LuaWorkerDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_max_health_level(lua_State*);
	int get_max_attack_level(lua_State*);
	int get_max_defense_level(lua_State*);
	int get_max_evade_level(lua_State*);
	int get_base_health(lua_State*);
	int get_base_min_attack(lua_State*);
	int get_base_max_attack(lua_State*);
	int get_base_defense(lua_State*);
	int get_base_evade(lua_State*);
	int get_health_incr_per_level(lua_State*);
	int get_attack_incr_per_level(lua_State*);
	int get_defense_incr_per_level(lua_State*);
	int get_evade_incr_per_level(lua_State*);

	/*
	 * Lua methods
	 */
	int get_health_level_image_filepath(lua_State* L);
	int get_attack_level_image_filepath(lua_State* L);
	int get_defense_level_image_filepath(lua_State* L);
	int get_evade_level_image_filepath(lua_State* L);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(SoldierDescr)
};

}  // namespace LuaMaps

#endif
