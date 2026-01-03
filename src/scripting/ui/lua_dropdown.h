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

#ifndef WL_SCRIPTING_UI_LUA_DROPDOWN_H
#define WL_SCRIPTING_UI_LUA_DROPDOWN_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/dropdown.h"

namespace LuaUi {

// The currently supported kinds of templated UI elements.
using DropdownOfString = UI::Dropdown<std::string>;

class LuaDropdown : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaDropdown);

	LuaDropdown() = default;
	explicit LuaDropdown(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaDropdown(lua_State* L) : LuaPanel(L) {
	}
	~LuaDropdown() override = default;

	/*
	 * Properties
	 */
	int get_datatype(lua_State* L);
	int get_expanded(lua_State* L);
	int get_no_of_items(lua_State* L);
	int get_selection(lua_State* L);
	int get_listselect(lua_State* L);

	/*
	 * Lua Methods
	 */
	int open(lua_State* L);
	int highlight_item(lua_State* L);
	int indicate_item(lua_State* L);
	int select(lua_State* L);
	int clear(lua_State* L);
	int add(lua_State* L);
	int get_value_at(lua_State* L);
	int get_label_at(lua_State* L);
	int get_tooltip_at(lua_State* L);

	/*
	 * C Methods
	 */
	UI::BaseDropdown* get() {
		return dynamic_cast<UI::BaseDropdown*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_DROPDOWN_H
