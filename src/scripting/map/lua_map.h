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

#ifndef WL_SCRIPTING_MAP_LUA_MAP_H
#define WL_SCRIPTING_MAP_LUA_MAP_H

#include "logic/map.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaMap : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaMap);

	~LuaMap() override = default;

	LuaMap() = default;
	explicit LuaMap(lua_State* L) {
		report_error(L, "Cannot instantiate a 'Map' directly!");
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_allows_seafaring(lua_State*);
	int get_number_of_port_spaces(lua_State*);
	int get_port_spaces(lua_State*);
	int get_width(lua_State*);
	int get_height(lua_State*);
	int get_player_slots(lua_State*);
	int get_waterway_max_length(lua_State*);
	int set_waterway_max_length(lua_State*);

	/*
	 * Lua methods
	 */
	int count_conquerable_fields(lua_State*);
	int count_terrestrial_fields(lua_State*);
	int count_owned_valuable_fields(lua_State*);
	int place_immovable(lua_State*);
	int get_field(lua_State*);
	int wrap_field(lua_State*);
	int recalculate(lua_State*);
	int recalculate_seafaring(lua_State*);
	int set_port_space(lua_State*);
	int sea_route_exists(lua_State*);
	int find_ocean_fields(lua_State*);

	/*
	 * C methods
	 */
	int do_get_field(lua_State* L, uint32_t x, uint32_t y);

private:
};

}  // namespace LuaMaps

#endif
