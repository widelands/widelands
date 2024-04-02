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

#ifndef WL_SCRIPTING_MAP_LUA_TRIBE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_TRIBE_DESCRIPTION_H

#include "logic/map_objects/tribes/tribe_descr.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaTribeDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTribeDescription);

	~LuaTribeDescription() override = default;

	LuaTribeDescription() = default;
	explicit LuaTribeDescription(const Widelands::TribeDescr* const tribedescr)
	   : tribedescr_(tribedescr) {
	}
	explicit LuaTribeDescription(lua_State* L) : tribedescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaTribeDescription' directly!");
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_buildings(lua_State*);
	int get_builder(lua_State* L);
	int get_carriers(lua_State*);
	int get_carrier(lua_State*);
	int get_carrier2(lua_State*);
	int get_ferry(lua_State*);
	int get_descname(lua_State*);
	int get_immovables(lua_State*);
	int get_resource_indicators(lua_State*);
	int get_geologist(lua_State*);
	int get_name(lua_State*);
	int get_port(lua_State*);
	int get_ship(lua_State*);
	int get_soldier(lua_State*);
	int get_wares(lua_State*);
	int get_workers(lua_State*);
	int get_directory(lua_State*);
	int get_collectors_points_table(lua_State*);

	/*
	 * Lua methods
	 */
	int has_building(lua_State*);
	int has_ware(lua_State*);
	int has_worker(lua_State*);

	/*
	 * C methods
	 */
protected:
	[[nodiscard]] const Widelands::TribeDescr* get() const {
		assert(tribedescr_ != nullptr);
		return tribedescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::TribeDescr* pointer) {
		tribedescr_ = pointer;
	}

private:
	const Widelands::TribeDescr* tribedescr_{nullptr};
};

}  // namespace LuaMaps

#endif
