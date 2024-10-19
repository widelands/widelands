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

#ifndef WL_SCRIPTING_MAP_LUA_CONSTRUCTION_SITE_H
#define WL_SCRIPTING_MAP_LUA_CONSTRUCTION_SITE_H

#include "logic/map_objects/tribes/constructionsite.h"
#include "scripting/map/lua_building.h"

namespace LuaMaps {

class LuaConstructionSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaConstructionSite);

	LuaConstructionSite() = default;
	explicit LuaConstructionSite(Widelands::ConstructionSite& mo) : LuaBuilding(mo) {
	}
	explicit LuaConstructionSite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaConstructionSite() override = default;

	/*
	 * Properties
	 */
	int get_building(lua_State*);
	int get_has_builder(lua_State*);
	int set_has_builder(lua_State*);
	int get_setting_soldier_capacity(lua_State*);
	int set_setting_soldier_capacity(lua_State*);
	int get_setting_soldier_preference(lua_State*);
	int set_setting_soldier_preference(lua_State*);
	int get_setting_launch_expedition(lua_State*);
	int set_setting_launch_expedition(lua_State*);
	int get_setting_stopped(lua_State*);
	int set_setting_stopped(lua_State*);

	/*
	 * Lua Methods
	 */
	int get_priority(lua_State*);
	int set_priority(lua_State*);
	int get_desired_fill(lua_State*);
	int set_desired_fill(lua_State*);
	int get_setting_warehouse_policy(lua_State*);
	int set_setting_warehouse_policy(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(ConstructionSite)
};

}  // namespace LuaMaps

#endif
