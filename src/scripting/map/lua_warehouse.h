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

#ifndef WL_SCRIPTING_MAP_LUA_WAREHOUSE_H
#define WL_SCRIPTING_MAP_LUA_WAREHOUSE_H

#include "logic/map_objects/tribes/warehouse.h"
#include "scripting/map/lua_building.h"

namespace LuaMaps {

class LuaWarehouse : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaWarehouse);

	LuaWarehouse() = default;
	explicit LuaWarehouse(Widelands::Warehouse& mo) : LuaBuilding(mo) {
	}
	explicit LuaWarehouse(lua_State* L) : LuaBuilding(L) {
	}
	~LuaWarehouse() override = default;

	/*
	 * Properties
	 */
	int get_portdock(lua_State* L);
	int get_expedition_in_progress(lua_State* L);
	int get_warehousename(lua_State* L);
	int set_warehousename(lua_State* L);
	int get_max_garrison(lua_State*);
	int get_soldier_preference(lua_State*);
	int set_soldier_preference(lua_State*);
	int get_garrison(lua_State*);
	int set_garrison(lua_State*);

	/*
	 * Lua Methods
	 */
	int get_wares(lua_State*);
	int get_workers(lua_State*);
	int set_wares(lua_State*);
	int set_workers(lua_State*);
	int set_soldiers(lua_State*);
	int get_soldiers(lua_State*);
	int set_warehouse_policies(lua_State*);
	int get_warehouse_policies(lua_State*);
	int start_expedition(lua_State*);
	int cancel_expedition(lua_State*);

	/*
	 * C Methods
	 */
	CASTED_GET(Warehouse)
};

}  // namespace LuaMaps

#endif
