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

#ifndef WL_SCRIPTING_MAP_LUA_WAREHOUSE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_WAREHOUSE_DESCRIPTION_H

#include "logic/map_objects/tribes/warehouse.h"
#include "scripting/map/lua_building_description.h"

namespace LuaMaps {

class LuaWarehouseDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaWarehouseDescription);

	~LuaWarehouseDescription() override = default;

	LuaWarehouseDescription() = default;
	explicit LuaWarehouseDescription(const Widelands::WarehouseDescr* const warehousedescr)
	   : LuaBuildingDescription(warehousedescr) {
	}
	explicit LuaWarehouseDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_heal_per_second(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(WarehouseDescr)
};

}  // namespace LuaMaps

#endif
