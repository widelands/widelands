/*
 * Copyright (C) 2026 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_UI_LUA_TIMER_H
#define WL_SCRIPTING_UI_LUA_TIMER_H

#include "scripting/lua_ui.h"
#include "wui/plugins.h"

namespace LuaUi {

class LuaTimer : public LuaUiModuleClass {
public:
	LUNA_CLASS_HEAD(LuaTimer);

	LuaTimer() = default;
	explicit LuaTimer(PluginActions::Timer* t) : timer_(t) {
	}
	explicit LuaTimer(lua_State* L) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	~LuaTimer() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State* L) override {
		report_error(L, "Trying to persist a Plugin Timer which is not supported!");
	}
	void __unpersist(lua_State* L) override {
		report_error(L, "Trying to unpersist a Plugin Timer which is not supported!");
	}
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_active(lua_State* L);
	int set_active(lua_State* L);
	int get_failsafe(lua_State* L);
	int set_failsafe(lua_State* L);
	int get_remaining_count(lua_State* L);
	int set_remaining_count(lua_State* L);
	int get_name(lua_State* L);
	int set_name(lua_State* L);
	int get_action(lua_State* L);
	int set_action(lua_State* L);
	int get_interval(lua_State* L);
	int set_interval(lua_State* L);
	int get_next_run(lua_State* L);
	int set_next_run(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	PluginActions::Timer* get() {
		return timer_;
	}

private:
	PluginActions::Timer* timer_ = nullptr;
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_TIMER_H
