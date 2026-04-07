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

#ifndef WL_SCRIPTING_UI_LUA_TAB_PANEL_H
#define WL_SCRIPTING_UI_LUA_TAB_PANEL_H

#include "scripting/ui/lua_panel.h"
#include "ui/basic/tabpanel.h"

namespace LuaUi {

class LuaTabPanel : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaTabPanel);

	LuaTabPanel() = default;
	explicit LuaTabPanel(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaTabPanel(lua_State* L) : LuaPanel(L) {
	}
	~LuaTabPanel() override = default;

	/*
	 * Properties
	 */
	int get_no_of_tabs(lua_State* L);
	int get_active(lua_State* L);
	int set_active(lua_State* L);

	/*
	 * Lua Methods
	 */
	int remove_last_tab(lua_State* L);

	/*
	 * C Methods
	 */
	UI::TabPanel* get() {
		return dynamic_cast<UI::TabPanel*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_TAB_PANEL_H
