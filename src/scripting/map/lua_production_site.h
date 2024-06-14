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

#ifndef WL_SCRIPTING_MAP_LUA_PRODUCTION_SITE_H
#define WL_SCRIPTING_MAP_LUA_PRODUCTION_SITE_H

#include "logic/map_objects/tribes/productionsite.h"
#include "scripting/map/lua_building.h"

namespace LuaMaps {

class LuaProductionSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaProductionSite);

	LuaProductionSite() = default;
	explicit LuaProductionSite(Widelands::ProductionSite& mo) : LuaBuilding(mo) {
	}
	explicit LuaProductionSite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaProductionSite() override = default;

	/*
	 * Properties
	 */
	int get_valid_inputs(lua_State* L);
	int get_valid_workers(lua_State* L);
	int get_is_stopped(lua_State* L);
	int get_productivity(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get_inputs(lua_State* L);
	int get_workers(lua_State* L);
	int set_inputs(lua_State* L);
	int set_workers(lua_State* L);
	int toggle_start_stop(lua_State* L);
	int get_priority(lua_State*);
	int set_priority(lua_State*);
	int get_desired_fill(lua_State*);
	int set_desired_fill(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(ProductionSite)
	static bool create_new_worker(lua_State* L,
	                              Widelands::ProductionSite& ps,
	                              Widelands::EditorGameBase&,
	                              const Widelands::WorkerDescr*);
};

}  // namespace LuaMaps

#endif
