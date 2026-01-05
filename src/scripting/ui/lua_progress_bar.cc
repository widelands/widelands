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

#include "scripting/ui/lua_progress_bar.h"

namespace LuaUi {

/* RST
ProgressBar
-----------

.. class:: ProgressBar

   .. versionadded:: 1.2

   A partially filled bar that indicates the progress of an operation.
*/
const char LuaProgressBar::className[] = "ProgressBar";
const MethodType<LuaProgressBar> LuaProgressBar::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaProgressBar> LuaProgressBar::Properties[] = {
   PROP_RW(LuaProgressBar, state),
   PROP_RW(LuaProgressBar, total),
   PROP_RW(LuaProgressBar, show_percent),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: state

      (RW) The current progress value.

      .. Note:: When using a progress bar to visualize the progress of a blocking script,
         you may need to call :meth:`~Panel.force_redraw` after changing this property to ensure
         the change becomes visible to the user immediately.
*/
int LuaProgressBar::get_state(lua_State* L) {
	lua_pushinteger(L, get()->get_state());
	return 1;
}
int LuaProgressBar::set_state(lua_State* L) {
	get()->set_state(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: total

      (RW) The maximum progress value.
*/
int LuaProgressBar::get_total(lua_State* L) {
	lua_pushinteger(L, get()->get_total());
	return 1;
}
int LuaProgressBar::set_total(lua_State* L) {
	get()->set_total(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: show_percent

      (RW) Whether the progress bar label displays the absolute progress
      or the percentage completed.
*/
int LuaProgressBar::get_show_percent(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_show_percent()));
	return 1;
}
int LuaProgressBar::set_show_percent(lua_State* L) {
	get()->set_show_percent(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

}  // namespace LuaUi
