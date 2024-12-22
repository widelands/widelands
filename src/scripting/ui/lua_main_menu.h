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

#ifndef WL_SCRIPTING_UI_LUA_MAIN_MENU_H
#define WL_SCRIPTING_UI_LUA_MAIN_MENU_H

#include "scripting/ui/lua_panel.h"
#include "ui_fsmenu/main.h"

namespace LuaUi {

class LuaMainMenu : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaMainMenu);

	LuaMainMenu() = default;
	explicit LuaMainMenu(FsMenu::MainMenu* p) : LuaPanel(p) {
	}
	explicit LuaMainMenu(lua_State* L);
	~LuaMainMenu() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override {
	}
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */

	/*
	 * Lua Methods
	 */
	int set_keyboard_shortcut(lua_State* L);
	int set_keyboard_shortcut_release(lua_State* L);
	int add_plugin_timer(lua_State* L);

	/*
	 * C Methods
	 */
	FsMenu::MainMenu* get() {
		return dynamic_cast<FsMenu::MainMenu*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_MAIN_MENU_H
