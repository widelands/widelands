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

#include "scripting/ui/lua_window.h"

#include "base/log.h"

namespace LuaUi {

/* RST
Window
------

.. class:: Window

   This represents a Window.
*/
const char LuaWindow::className[] = "Window";
const MethodType<LuaWindow> LuaWindow::Methods[] = {
   METHOD(LuaWindow, close),
   {nullptr, nullptr},
};
const PropertyType<LuaWindow> LuaWindow::Properties[] = {
   PROP_RW(LuaWindow, title),
   PROP_RW(LuaWindow, pinned),
   PROP_RW(LuaWindow, minimal),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: title

      .. versionadded:: 1.2

      (RW) The title shown in the window's title bar.
*/
int LuaWindow::get_title(lua_State* L) {
	lua_pushstring(L, get()->get_title().c_str());
	return 1;
}
int LuaWindow::set_title(lua_State* L) {
	get()->set_title(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: pinned

      .. versionadded:: 1.2

      (RW) Whether the window is pinned so it can't be closed accidentally.
*/
int LuaWindow::get_pinned(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_pinned()));
	return 1;
}
int LuaWindow::set_pinned(lua_State* L) {
	get()->set_pinned(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: minimal

      .. versionadded:: 1.2

      (RW) Whether the window is minimized and only the title bar visible.
*/
int LuaWindow::get_minimal(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_minimal()));
	return 1;
}
int LuaWindow::set_minimal(lua_State* L) {
	if (luaL_checkboolean(L, -1)) {
		if (!get()->is_minimal()) {
			get()->minimize();
		}
	} else {
		if (get()->is_minimal()) {
			get()->restore();
		}
	}
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: close

      Closes this window. This invalidates this Object, do
      not use it any longer.
*/
int LuaWindow::close(lua_State* /* L */) {
	verb_log_info("Closing window '%s'\n", get()->get_name().c_str());
	panel_->die();
	panel_ = nullptr;
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
