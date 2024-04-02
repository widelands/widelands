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

#ifndef WL_SCRIPTING_MAP_LUA_TERRAIN_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_TERRAIN_DESCRIPTION_H

#include "logic/map_objects/world/terrain_description.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaTerrainDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTerrainDescription);

	~LuaTerrainDescription() override = default;

	LuaTerrainDescription() = default;
	explicit LuaTerrainDescription(const Widelands::TerrainDescription* const terraindescr)
	   : terraindescr_(terraindescr) {
	}
	explicit LuaTerrainDescription(lua_State* L) : terraindescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaTerrainDescription' directly!");
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_name(lua_State*);
	int get_descname(lua_State*);
	int get_default_resource(lua_State*);
	int get_default_resource_amount(lua_State*);
	int get_fertility(lua_State*);
	int get_humidity(lua_State*);
	int get_representative_image(lua_State*);
	int get_temperature(lua_State*);
	int get_valid_resources(lua_State*);

	/*
	 * Lua methods
	 */

	/*
	 * C methods
	 */
	[[nodiscard]] const Widelands::TerrainDescription* get() const {
		assert(terraindescr_ != nullptr);
		return terraindescr_;
	}

protected:
	// For persistence.
	void set_description_pointer(const Widelands::TerrainDescription* pointer) {
		terraindescr_ = pointer;
	}

private:
	const Widelands::TerrainDescription* terraindescr_{nullptr};
};

}  // namespace LuaMaps

#endif
