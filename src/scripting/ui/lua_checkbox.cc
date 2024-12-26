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

#include "scripting/ui/lua_checkbox.h"

namespace LuaUi {

/* RST
Checkbox
--------

.. class:: Checkbox

   .. versionadded:: 1.2

   A tick box that can be toggled on or off by the user.
*/
const char LuaCheckbox::className[] = "Checkbox";
const MethodType<LuaCheckbox> LuaCheckbox::Methods[] = {
   METHOD(LuaCheckbox, set_enabled),
   {nullptr, nullptr},
};
const PropertyType<LuaCheckbox> LuaCheckbox::Properties[] = {
   PROP_RW(LuaCheckbox, state),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: state

      (RW) Whether the checkbox is currently checked.
*/
int LuaCheckbox::get_state(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_state()));
	return 1;
}
int LuaCheckbox::set_state(lua_State* L) {
	get()->set_state(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_enabled(b)

      Set whether the user can change the state of this checkbox.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaCheckbox::set_enabled(lua_State* L) {
	get()->set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
