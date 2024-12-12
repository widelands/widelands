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

#ifndef WL_SCRIPTING_MAP_LUA_RESOURCE_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_RESOURCE_DESCRIPTION_H

#include "logic/map_objects/world/resource_description.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaResourceDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaResourceDescription);

	~LuaResourceDescription() override = default;

	LuaResourceDescription() = default;
	explicit LuaResourceDescription(const Widelands::ResourceDescription* const resourcedescr)
	   : resourcedescr_(resourcedescr) {
	}
	explicit LuaResourceDescription(lua_State* L) : resourcedescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaResourceDescription' directly!");
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
	int get_is_detectable(lua_State*);
	int get_max_amount(lua_State*);
	int get_representative_image(lua_State*);

	/*
	 * Lua methods
	 */

	int editor_image(lua_State*);

	/*
	 * C methods
	 */
protected:
	[[nodiscard]] const Widelands::ResourceDescription* get() const {
		assert(resourcedescr_ != nullptr);
		return resourcedescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::ResourceDescription* pointer) {
		resourcedescr_ = pointer;
	}

private:
	const Widelands::ResourceDescription* resourcedescr_{nullptr};
};

}  // namespace LuaMaps

#endif
