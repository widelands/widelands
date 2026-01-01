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

#ifndef WL_SCRIPTING_MAP_LUA_BUILDING_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_BUILDING_DESCRIPTION_H

#include "logic/map_objects/tribes/building.h"
#include "scripting/map/lua_map_object_description.h"

namespace LuaMaps {

class LuaBuildingDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaBuildingDescription);

	~LuaBuildingDescription() override = default;

	LuaBuildingDescription() = default;
	explicit LuaBuildingDescription(const Widelands::BuildingDescr* const buildingdescr)
	   : LuaMapObjectDescription(buildingdescr) {
	}
	explicit LuaBuildingDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_buildcost(lua_State*);
	int get_buildable(lua_State*);
	int get_conquers(lua_State*);
	int get_destructible(lua_State*);
	int get_enhanced(lua_State*);
	int get_enhanced_from(lua_State*);
	int get_enhancement_cost(lua_State*);
	int get_enhancement(lua_State*);
	int get_is_mine(lua_State*);
	int get_is_port(lua_State*);
	int get_size(lua_State*);
	int get_isproductionsite(lua_State*);
	int get_returns_on_dismantle(lua_State*);
	int get_enhancement_returns_on_dismantle(lua_State*);
	int get_vision_range(lua_State*);
	int get_workarea_radius(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(BuildingDescr)
};

}  // namespace LuaMaps

#endif
