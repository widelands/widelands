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

#ifndef WL_SCRIPTING_MAP_LUA_MAP_OBJECT_H
#define WL_SCRIPTING_MAP_LUA_MAP_OBJECT_H

#include "logic/map_objects/map_object.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

#define CASTED_GET(klass)                                                                          \
	Widelands::klass* get(lua_State* L, Widelands::EditorGameBase& egbase) {                        \
		return dynamic_cast<Widelands::klass*>(LuaMapObject::get(L, egbase, #klass));                \
	}

class LuaMapObject : public LuaMapModuleClass {
	Widelands::ObjectPointer ptr_;

public:
	LUNA_CLASS_HEAD(LuaMapObject);

	LuaMapObject() : ptr_(nullptr) {
	}
	explicit LuaMapObject(Widelands::MapObject& mo) : ptr_(&mo) {
	}
	explicit LuaMapObject(lua_State* L) : ptr_(nullptr) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	~LuaMapObject() override {
		ptr_ = nullptr;
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * attributes
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
	int get___hash(lua_State*);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	int get_descr(lua_State*);
	int get_serial(lua_State*);
	int get_exists(lua_State*);

	/*
	 * Lua Methods
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
	int __eq(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	int remove(lua_State* L);
	int destroy(lua_State* L);
	int has_attribute(lua_State* L);

	/*
	 * C Methods
	 */
	Widelands::MapObject*
	get(lua_State*, Widelands::EditorGameBase&, const std::string& = "MapObject");
	Widelands::MapObject* get_or_zero(const Widelands::EditorGameBase&);
};

}  // namespace LuaMaps

#endif
