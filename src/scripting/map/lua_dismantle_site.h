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

#ifndef WL_SCRIPTING_MAP_LUA_DISMANTLE_SITE_H
#define WL_SCRIPTING_MAP_LUA_DISMANTLE_SITE_H

#include "logic/map_objects/tribes/dismantlesite.h"
#include "scripting/map/lua_building.h"

namespace LuaMaps {

class LuaDismantleSite : public LuaBuilding {
public:
	LUNA_CLASS_HEAD(LuaDismantleSite);

	LuaDismantleSite() = default;
	explicit LuaDismantleSite(Widelands::DismantleSite& mo) : LuaBuilding(mo) {
	}
	explicit LuaDismantleSite(lua_State* L) : LuaBuilding(L) {
	}
	~LuaDismantleSite() override = default;

	/*
	 * Properties
	 */
	int get_has_builder(lua_State*);
	int set_has_builder(lua_State*);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(DismantleSite)
};

}  // namespace LuaMaps

#endif
