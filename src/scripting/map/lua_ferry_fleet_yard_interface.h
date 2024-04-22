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

#ifndef WL_SCRIPTING_MAP_LUA_FERRY_FLEET_YARD_INTERFACE_H
#define WL_SCRIPTING_MAP_LUA_FERRY_FLEET_YARD_INTERFACE_H

#include "economy/ferry_fleet.h"
#include "scripting/map/lua_bob.h"

namespace LuaMaps {

class LuaFerryFleetYardInterface : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaFerryFleetYardInterface);

	LuaFerryFleetYardInterface() = default;
	explicit LuaFerryFleetYardInterface(Widelands::FerryFleetYardInterface& i) : LuaBob(i) {
	}
	explicit LuaFerryFleetYardInterface(lua_State* L) : LuaBob(L) {
	}
	~LuaFerryFleetYardInterface() override = default;

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_building(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(FerryFleetYardInterface)
};

}  // namespace LuaMaps

#endif
