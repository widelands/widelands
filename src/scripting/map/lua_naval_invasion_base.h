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

#ifndef WL_SCRIPTING_MAP_LUA_NAVAL_INVASION_BASE_H
#define WL_SCRIPTING_MAP_LUA_NAVAL_INVASION_BASE_H

#include "logic/map_objects/tribes/naval_invasion_base.h"
#include "scripting/map/lua_bob.h"

namespace LuaMaps {

class LuaNavalInvasionBase : public LuaBob {
public:
	LUNA_CLASS_HEAD(LuaNavalInvasionBase);

	LuaNavalInvasionBase() = default;
	explicit LuaNavalInvasionBase(Widelands::NavalInvasionBase& n) : LuaBob(n) {
	}
	explicit LuaNavalInvasionBase(lua_State* L) : LuaBob(L) {
	}
	~LuaNavalInvasionBase() override = default;

	/*
	 * Properties
	 */
	int get_owner(lua_State* L);
	int get_soldiers(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	CASTED_GET(NavalInvasionBase)
};

}  // namespace LuaMaps

#endif
