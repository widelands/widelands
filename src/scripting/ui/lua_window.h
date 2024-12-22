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

#ifndef WL_SCRIPTING_UI_LUA_WINDOW_H
#define WL_SCRIPTING_UI_LUA_WINDOW_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/window.h"

namespace LuaUi {

class LuaWindow : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaWindow);

	LuaWindow() = default;
	explicit LuaWindow(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaWindow(lua_State* L) : LuaPanel(L) {
	}
	~LuaWindow() override = default;

	/*
	 * Properties
	 */
	int get_title(lua_State* L);
	int set_title(lua_State* L);
	int get_pinned(lua_State* L);
	int set_pinned(lua_State* L);
	int get_minimal(lua_State* L);
	int set_minimal(lua_State* L);

	/*
	 * Lua Methods
	 */
	int close(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Window* get() {
		return dynamic_cast<UI::Window*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_WINDOW_H
