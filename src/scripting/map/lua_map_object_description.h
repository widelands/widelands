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

#ifndef WL_SCRIPTING_MAP_LUA_MAP_OBJECT_DESCRIPTION_H
#define WL_SCRIPTING_MAP_LUA_MAP_OBJECT_DESCRIPTION_H

#include "logic/map_objects/map_object.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaMapObjectDescription : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaMapObjectDescription);

	~LuaMapObjectDescription() override = default;

	LuaMapObjectDescription() = default;
	explicit LuaMapObjectDescription(const Widelands::MapObjectDescr* const mapobjectdescr)
	   : mapobjectdescr_(mapobjectdescr) {
	}
	explicit LuaMapObjectDescription(lua_State* L) : mapobjectdescr_(nullptr) {
		report_error(L, "Cannot instantiate a 'MapObjectDescription' directly!");
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_descname(lua_State*);
	int get_icon_name(lua_State*);
	int get_name(lua_State*);
	int get_type_name(lua_State*);

	/*
	 * Lua methods
	 */
	int has_attribute(lua_State*);
	int helptexts(lua_State*);

	/*
	 * C methods
	 */
protected:
	[[nodiscard]] const Widelands::MapObjectDescr* get() const {
		assert(mapobjectdescr_ != nullptr);
		return mapobjectdescr_;
	}
	// For persistence.
	void set_description_pointer(const Widelands::MapObjectDescr* pointer) {
		mapobjectdescr_ = pointer;
	}

private:
	const Widelands::MapObjectDescr* mapobjectdescr_{nullptr};
};

#define CASTED_GET_DESCRIPTION(klass)                                                              \
	const Widelands::klass* get() const {                                                           \
		return dynamic_cast<const Widelands::klass*>(LuaMapObjectDescription::get());                \
	}

}  // namespace LuaMaps

#endif
