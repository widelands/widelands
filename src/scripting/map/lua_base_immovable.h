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

#ifndef WL_SCRIPTING_MAP_LUA_BASE_IMMOVABLE_H
#define WL_SCRIPTING_MAP_LUA_BASE_IMMOVABLE_H

#include "logic/map_objects/immovable.h"
#include "scripting/map/lua_map_object.h"

namespace LuaMaps {

class LuaBaseImmovable : public LuaMapObject {
public:
	LUNA_CLASS_HEAD(LuaBaseImmovable);

	LuaBaseImmovable() = default;
	explicit LuaBaseImmovable(Widelands::BaseImmovable& mo) : LuaMapObject(mo) {
	}
	explicit LuaBaseImmovable(lua_State* L) : LuaMapObject(L) {
	}
	~LuaBaseImmovable() override = default;

	/*
	 * Properties
	 */
	int get_fields(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	CASTED_GET(BaseImmovable)
};

}  // namespace LuaMaps

#endif
