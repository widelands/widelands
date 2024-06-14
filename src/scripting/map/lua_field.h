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

#ifndef WL_SCRIPTING_MAP_LUA_FIELD_H
#define WL_SCRIPTING_MAP_LUA_FIELD_H

#include "logic/widelands_geometry.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaField : public LuaMapModuleClass {
	Widelands::Coords coords_;

public:
	LUNA_CLASS_HEAD(LuaField);

	LuaField() = default;
	explicit LuaField(int16_t x, int16_t y) : coords_(Widelands::Coords(x, y)) {
	}
	explicit LuaField(Widelands::Coords c) : coords_(c) {
	}
	explicit LuaField(lua_State* L) {
		report_error(L, "Cannot instantiate a 'Field' directly!");
	}
	~LuaField() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
	int get___hash(lua_State*);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	int get_x(lua_State* L);
	int get_y(lua_State* L);
	int get_viewpoint_x(lua_State* L);
	int get_viewpoint_y(lua_State* L);
	int get_height(lua_State* L);
	int set_height(lua_State* L);
	int get_raw_height(lua_State* L);
	int set_raw_height(lua_State* L);
	int get_immovable(lua_State* L);
	int get_bobs(lua_State* L);
	int get_terr(lua_State* L);
	int set_terr(lua_State* L);
	int get_terd(lua_State* L);
	int set_terd(lua_State* L);
	int get_rn(lua_State*);
	int get_ln(lua_State*);
	int get_trn(lua_State*);
	int get_tln(lua_State*);
	int get_bln(lua_State*);
	int get_brn(lua_State*);
	int get_resource(lua_State*);
	int set_resource(lua_State*);
	int get_resource_amount(lua_State*);
	int set_resource_amount(lua_State*);
	int get_initial_resource_amount(lua_State*);
	int set_initial_resource_amount(lua_State*);
	int get_claimers(lua_State*);
	int get_owner(lua_State*);
	int get_buildable(lua_State*);
	int get_has_roads(lua_State* L);

	/*
	 * Lua methods
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
	int __tostring(lua_State* L);
	// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
	int __eq(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	int region(lua_State* L);
	int has_caps(lua_State*);
	int has_max_caps(lua_State*);
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
	int indicate(lua_State* L);
#endif

	/*
	 * C methods
	 */
	inline const Widelands::Coords& coords() {
		return coords_;
	}
	const Widelands::FCoords fcoords(lua_State* L);

private:
	int region(lua_State* L, uint32_t radius);
	int hollow_region(lua_State* L, uint32_t radius, uint32_t inner_radius);
};

}  // namespace LuaMaps

#endif
