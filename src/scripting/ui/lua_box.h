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

#ifndef WL_SCRIPTING_UI_LUA_BOX_H
#define WL_SCRIPTING_UI_LUA_BOX_H

#include "scripting/ui/lua_panel.h"
#include "ui/basic/box.h"

namespace LuaUi {

class LuaBox : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaBox);

	LuaBox() = default;
	explicit LuaBox(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaBox(lua_State* L) : LuaPanel(L) {
	}
	~LuaBox() override = default;

	/*
	 * Properties
	 */
	int get_orientation(lua_State* L);
	int get_no_of_items(lua_State* L);
	int get_scrolling(lua_State* L);
	int set_scrolling(lua_State* L);
	int get_force_scrolling(lua_State* L);
	int set_force_scrolling(lua_State* L);
	int get_inner_spacing(lua_State* L);
	int set_inner_spacing(lua_State* L);
	int get_min_desired_breadth(lua_State* L);
	int set_min_desired_breadth(lua_State* L);
	int get_max_width(lua_State* L);
	int set_max_width(lua_State* L);
	int get_max_height(lua_State* L);
	int set_max_height(lua_State* L);

	/*
	 * Lua Methods
	 */
	int clear(lua_State* L);
	int get_index(lua_State* L);
	int is_space(lua_State* L);
	int get_resizing(lua_State* L);
	int get_align(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Box* get() {
		return dynamic_cast<UI::Box*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_BOX_H
