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

#ifndef WL_SCRIPTING_LUA_UI_H
#define WL_SCRIPTING_LUA_UI_H

#include "scripting/lua.h"
#include "scripting/luna.h"

namespace UI {
class Panel;
}  // namespace UI

namespace LuaUi {

/*
 * Base class for all classes in wl.ui
 */
class LuaUiModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "ui";
	}
};

int upcasted_panel_to_lua(lua_State* L, UI::Panel* panel);

std::string shortcut_string_if_set(const std::string& name, bool rt_escape);

void luaopen_wlui(lua_State*, bool game_or_editor);

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_LUA_UI_H
