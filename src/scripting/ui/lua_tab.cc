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

#include "scripting/ui/lua_tab.h"

#include "base/log.h"

namespace LuaUi {

/* RST
Tab
------

.. class:: Tab

   A tab button.
*/
const char LuaTab::className[] = "Tab";
const MethodType<LuaTab> LuaTab::Methods[] = {
   METHOD(LuaTab, click),
   {nullptr, nullptr},
};
const PropertyType<LuaTab> LuaTab::Properties[] = {
   PROP_RO(LuaTab, active),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: active

      (RO) Is this the currently active tab in this window?
*/
int LuaTab::get_active(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->active()));
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: click

      Click this tab making it the active one.
*/
int LuaTab::click(lua_State* /* L */) {
	verb_log_info("Clicking tab '%s'\n", get()->get_name().c_str());
	get()->activate();
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
