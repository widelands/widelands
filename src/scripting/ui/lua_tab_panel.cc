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

#include "scripting/ui/lua_tab_panel.h"

namespace LuaUi {

/* RST
TabPanel
--------

.. class:: TabPanel

   .. versionadded:: 1.2

   This represents a panel that allows switching between multiple tabs.
*/
const char LuaTabPanel::className[] = "TabPanel";
const MethodType<LuaTabPanel> LuaTabPanel::Methods[] = {
   METHOD(LuaTabPanel, remove_last_tab),
   {nullptr, nullptr},
};
const PropertyType<LuaTabPanel> LuaTabPanel::Properties[] = {
   PROP_RO(LuaTabPanel, no_of_tabs),
   PROP_RW(LuaTabPanel, active),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: no_of_tabs

      (RO) The number of tabs this tab panel has.
*/
int LuaTabPanel::get_no_of_tabs(lua_State* L) {
	lua_pushinteger(L, get()->tabs().size());
	return 1;
}

/* RST
   .. attribute:: active

      (RW) The index of the currently active tab.
      When assigning this property, it is also allowed to activate a tab by name instead of index.
*/
int LuaTabPanel::get_active(lua_State* L) {
	lua_pushinteger(L, get()->active());
	return 1;
}
int LuaTabPanel::set_active(lua_State* L) {
	if (static_cast<bool>(lua_isnumber(L, -1))) {
		get()->activate(luaL_checkuint32(L, -1));
	} else {
		get()->activate(luaL_checkstring(L, -1));
	}
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: remove_last_tab(name)

      Remove the **last** tab in the panel.

      As a precaution against accidental removal of tabs, the name of the tab
      that will be removed has to be specified.

      :arg name: The name of the last tab.
      :type name: :class:`string`
      :returns: Whether the tab was removed.
      :rtype: :class:`boolean`
*/
int LuaTabPanel::remove_last_tab(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->remove_last_tab(luaL_checkstring(L, 2))));
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
