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

#ifndef WL_SCRIPTING_MAP_LUA_CONSTRUCTION_SITE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_CONSTRUCTION_SITE_DESCRIPTION_H

#include "logic/map_objects/tribes/constructionsite.h"
#include "scripting/map/lua_building_description.h"

namespace LuaMaps {

class LuaConstructionSiteDescription : public LuaBuildingDescription {
public:
	LUNA_CLASS_HEAD(LuaConstructionSiteDescription);

	~LuaConstructionSiteDescription() override = default;

	LuaConstructionSiteDescription() = default;
	explicit LuaConstructionSiteDescription(
	   const Widelands::ConstructionSiteDescr* const constructionsitedescr)
	   : LuaBuildingDescription(constructionsitedescr) {
	}
	explicit LuaConstructionSiteDescription(lua_State* L) : LuaBuildingDescription(L) {
	}

private:
	CASTED_GET_DESCRIPTION(ConstructionSiteDescr)
};

}  // namespace LuaMaps

#endif
