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

#ifndef WL_SCRIPTING_UI_LUA_SPIN_BOX_H
#define WL_SCRIPTING_UI_LUA_SPIN_BOX_H

#include "scripting/ui/lua_panel.h"
#include "ui/basic/spinbox.h"

namespace LuaUi {

class LuaSpinBox : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaSpinBox);

	LuaSpinBox() = default;
	explicit LuaSpinBox(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaSpinBox(lua_State* L) : LuaPanel(L) {
	}
	~LuaSpinBox() override = default;

	/*
	 * Properties
	 */
	int get_value(lua_State* L);
	int set_value(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_unit_width(lua_State* L);
	int set_interval(lua_State* L);
	int add_replacement(lua_State* L);

	/*
	 * C Methods
	 */
	UI::SpinBox* get() {
		return dynamic_cast<UI::SpinBox*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_SPIN_BOX_H
