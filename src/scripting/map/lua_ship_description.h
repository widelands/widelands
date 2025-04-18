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

#ifndef WL_SCRIPTING_MAP_LUA_SHIP_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_SHIP_DESCRIPTION_H

#include "logic/map_objects/tribes/ship.h"
#include "scripting/map/lua_map_object_description.h"

namespace LuaMaps {

class LuaShipDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaShipDescription);

	~LuaShipDescription() override = default;

	LuaShipDescription() = default;
	explicit LuaShipDescription(const Widelands::ShipDescr* const shipdescr)
	   : LuaMapObjectDescription(shipdescr) {
	}
	explicit LuaShipDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ShipDescr)
};

}  // namespace LuaMaps

#endif
