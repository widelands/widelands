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

#ifndef WL_SCRIPTING_UI_LUA_MULTILINE_TEXTAREA_H
#define WL_SCRIPTING_UI_LUA_MULTILINE_TEXTAREA_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/multilinetextarea.h"

namespace LuaUi {

class LuaMultilineTextarea : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaMultilineTextarea);

	LuaMultilineTextarea() = default;
	explicit LuaMultilineTextarea(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaMultilineTextarea(lua_State* L) : LuaPanel(L) {
	}
	~LuaMultilineTextarea() override = default;

	/*
	 * Properties
	 */
	int get_text(lua_State* L);
	int set_text(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	UI::MultilineTextarea* get() {
		return dynamic_cast<UI::MultilineTextarea*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_MULTILINE_TEXTAREA_H
