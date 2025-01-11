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

#ifndef WL_SCRIPTING_MAP_LUA_PLAYER_SLOT_H
#define WL_SCRIPTING_MAP_LUA_PLAYER_SLOT_H

#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaPlayerSlot : public LuaMapModuleClass {
	Widelands::PlayerNumber player_number_{0U};

public:
	LUNA_CLASS_HEAD(LuaPlayerSlot);

	LuaPlayerSlot() = default;
	explicit LuaPlayerSlot(Widelands::PlayerNumber plr) : player_number_(plr) {
	}
	explicit LuaPlayerSlot(lua_State* L) : player_number_(0) {
		report_error(L, "Cannot instantiate a 'PlayerSlot' directly!");
	}
	~LuaPlayerSlot() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_tribe_name(lua_State*);
	int set_tribe_name(lua_State*);
	int get_name(lua_State*);
	int set_name(lua_State*);
	int get_starting_field(lua_State*);
	int set_starting_field(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
};

}  // namespace LuaMaps

#endif
