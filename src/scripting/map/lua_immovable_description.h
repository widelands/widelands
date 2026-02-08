/*
 * Copyright (C) 2006-2026 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_MAP_LUA_IMMOVABLE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_IMMOVABLE_DESCRIPTION_H

#include "logic/map_objects/immovable.h"
#include "scripting/map/lua_map_object_description.h"

namespace LuaMaps {

class LuaImmovableDescription : public LuaMapObjectDescription {
public:
	LUNA_CLASS_HEAD(LuaImmovableDescription);

	~LuaImmovableDescription() override = default;

	LuaImmovableDescription() = default;
	explicit LuaImmovableDescription(const Widelands::ImmovableDescr* const immovabledescr)
	   : LuaMapObjectDescription(immovabledescr) {
	}
	explicit LuaImmovableDescription(lua_State* L) : LuaMapObjectDescription(L) {
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_species(lua_State*);
	int get_buildcost(lua_State*);
	int get_becomes(lua_State*);
	int get_terrain_affinity(lua_State*);
	int get_size(lua_State*);

	/*
	 * Lua methods
	 */
	int probability_to_grow(lua_State*);

	/*
	 * C methods
	 */

private:
	CASTED_GET_DESCRIPTION(ImmovableDescr)
};

}  // namespace LuaMaps

#endif
