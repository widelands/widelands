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

#ifndef WL_SCRIPTING_UI_LUA_CHECKBOX_H
#define WL_SCRIPTING_UI_LUA_CHECKBOX_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/checkbox.h"

namespace LuaUi {

class LuaCheckbox : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaCheckbox);

	LuaCheckbox() = default;
	explicit LuaCheckbox(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaCheckbox(lua_State* L) : LuaPanel(L) {
	}
	~LuaCheckbox() override = default;

	/*
	 * Properties
	 */
	int get_state(lua_State* L);
	int set_state(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_enabled(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Checkbox* get() {
		return dynamic_cast<UI::Checkbox*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_CHECKBOX_H
