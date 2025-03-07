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

#ifndef WL_SCRIPTING_MAP_LUA_PRODUCTION_SITE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_PRODUCTION_SITE_DESCRIPTION_H

#include "logic/map_objects/tribes/productionsite.h"
#include "scripting/map/lua_building_description.h"

namespace LuaMaps {

class LuaProductionSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaProductionSiteDescription);

	~LuaProductionSiteDescription() override = default;

	LuaProductionSiteDescription() = default;
	explicit LuaProductionSiteDescription(
	   const Widelands::ProductionSiteDescr* const productionsitedescr)
	   : LuaBuildingDescription(productionsitedescr) {
	}
	explicit LuaProductionSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

	/*
	 * Properties
	 */
	int get_inputs(lua_State*);
	int get_collected_bobs(lua_State*);
	int get_collected_immovables(lua_State*);
	int get_collected_resources(lua_State*);
	int get_created_bobs(lua_State*);
	int get_created_immovables(lua_State*);
	int get_created_resources(lua_State*);
	int get_output_ware_types(lua_State*);
	int get_output_worker_types(lua_State*);
	int get_production_programs(lua_State*);
	int get_supported_productionsites(lua_State*);
	int get_supported_by_productionsites(lua_State*);
	int get_working_positions(lua_State*);

	/*
	 * Lua methods
	 */

	int consumed_wares_workers(lua_State*);
	int produced_wares(lua_State*);
	int recruited_workers(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ProductionSiteDescr)
};

}  // namespace LuaMaps

#endif
