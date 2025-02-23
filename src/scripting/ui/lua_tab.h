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

#ifndef WL_SCRIPTING_UI_LUA_TAB_H
#define WL_SCRIPTING_UI_LUA_TAB_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/tabpanel.h"

namespace LuaUi {

class LuaTab : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTab);

	LuaTab() = default;
	explicit LuaTab(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTab(lua_State* L) : LuaPanel(L) {
	}
	~LuaTab() override = default;

	/*
	 * Properties
	 */
	int get_active(lua_State* L);

	/*
	 * Lua Methods
	 */
	int click(lua_State* L);

	/*
	 * C Methods
	 */
	UI::Tab* get() {
		return dynamic_cast<UI::Tab*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_TAB_H
