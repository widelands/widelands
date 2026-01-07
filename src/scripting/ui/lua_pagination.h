/*
 * Copyright (C) 2025-2026 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_UI_LUA_PAGINATION_H
#define WL_SCRIPTING_UI_LUA_PAGINATION_H

#include "scripting/ui/lua_panel.h"
#include "ui_basic/pagination.h"

namespace LuaUi {

class LuaPagination : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaPagination);

	LuaPagination() = default;
	explicit LuaPagination(UI::Panel* p) : LuaPanel(p) {
	}
	explicit LuaPagination(lua_State* L) : LuaPanel(L) {
	}
	~LuaPagination() override = default;

	/*
	 * Properties
	 */
	int get_current_page(lua_State* L);
	int set_current_page(lua_State* L);
	int get_pagesize(lua_State* L);
	int set_pagesize(lua_State* L);
	int get_no_of_items(lua_State* L);
	int get_no_of_pages(lua_State* L);

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
	UI::Pagination* get() {
		return dynamic_cast<UI::Pagination*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_PAGINATION_H
