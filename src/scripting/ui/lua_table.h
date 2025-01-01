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

#ifndef WL_SCRIPTING_UI_LUA_TABLE_H
#define WL_SCRIPTING_UI_LUA_TABLE_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/table.h"

namespace LuaUi {

// The currently supported kinds of templated UI elements.
using TableOfInt = UI::Table<uintptr_t>;

class LuaTable : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTable);

	LuaTable() = default;
	explicit LuaTable(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTable(lua_State* L) : LuaPanel(L) {
	}
	~LuaTable() override = default;

	/*
	 * Properties
	 */
	int get_datatype(lua_State* L);
	int get_no_of_rows(lua_State* L);
	int get_selection_index(lua_State* L);
	int set_selection_index(lua_State* L);
	int get_selections(lua_State* L);
	int get_sort_column(lua_State* L);
	int set_sort_column(lua_State* L);
	int get_sort_descending(lua_State* L);
	int set_sort_descending(lua_State* L);

	/*
	 * Lua Methods
	 */
	int get(lua_State* L);
	int add(lua_State* L);
	int remove_row(lua_State* L);
	int remove_entry(lua_State* L);

	/*
	 * C Methods
	 */
	UI::BaseTable* get() {
		return dynamic_cast<UI::BaseTable*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_TABLE_H
