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

#ifndef WL_SCRIPTING_UI_LUA_SLIDER_H
#define WL_SCRIPTING_UI_LUA_SLIDER_H

#include "scripting/ui/lua_panel.h"
#include "ui/basic/slider.h"

namespace LuaUi {

class LuaSlider : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaSlider);

	LuaSlider() = default;
	explicit LuaSlider(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaSlider(lua_State* L) : LuaPanel(L) {
	}
	~LuaSlider() override = default;

	/*
	 * Properties
	 */
	int get_value(lua_State* L);
	int set_value(lua_State* L);
	int get_min_value(lua_State* L);
	int set_min_value(lua_State* L);
	int get_max_value(lua_State* L);
	int set_max_value(lua_State* L);

	/*
	 * Lua Methods
	 */
	int set_enabled(lua_State* L);
	int set_cursor_fixed_height(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Slider* get() {
		return dynamic_cast<UI::Slider*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_SLIDER_H
