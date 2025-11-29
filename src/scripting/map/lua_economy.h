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

#ifndef WL_SCRIPTING_MAP_LUA_ECONOMY_H
#define WL_SCRIPTING_MAP_LUA_ECONOMY_H

#include "economy/economy.h"
#include "scripting/lua_map.h"

namespace LuaMaps {

class LuaEconomy : public LuaMapModuleClass {
public:
	LUNA_CLASS_HEAD(LuaEconomy);

	~LuaEconomy() override = default;

	LuaEconomy() = default;
	explicit LuaEconomy(Widelands::Economy* economy) : economy_(economy) {
	}
	explicit LuaEconomy(lua_State* L) : economy_(nullptr) {
		report_error(L, "Cannot instantiate a 'LuaEconomy' directly!");
	}

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override;
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */

	/*
	 * Lua methods
	 */
	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
	int __eq(lua_State* L);
	CLANG_DIAG_RESERVED_IDENTIFIER_ON
	int target_quantity(lua_State*);
	int set_target_quantity(lua_State*);
	int needs(lua_State*);

	/*
	 * C methods
	 */

protected:
	[[nodiscard]] Widelands::Economy* get() const {
		assert(economy_ != nullptr);
		return economy_;
	}
	// For persistence.
	void set_economy_pointer(Widelands::Economy* pointer) {
		economy_ = pointer;
	}

private:
	Widelands::Economy* economy_{nullptr};
};

}  // namespace LuaMaps

#endif
