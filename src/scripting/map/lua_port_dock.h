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

#ifndef WL_SCRIPTING_MAP_LUA_PORT_DOCK_H
#define WL_SCRIPTING_MAP_LUA_PORT_DOCK_H

#include "economy/portdock.h"
#include "scripting/map/lua_player_immovable.h"

namespace LuaMaps {

class LuaPortDock : public LuaPlayerImmovable {
public:
	LUNA_CLASS_HEAD(LuaPortDock);

	LuaPortDock() = default;
	explicit LuaPortDock(Widelands::PortDock& mo) : LuaPlayerImmovable(mo) {
	}
	explicit LuaPortDock(lua_State* L) : LuaPlayerImmovable(L) {
	}
	~LuaPortDock() override = default;

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(PortDock)
};

}  // namespace LuaMaps

#endif
