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

#include "scripting/ui/lua_radio_button.h"

namespace LuaUi {

/* RST
RadioButton
-----------

.. class:: RadioButton

   .. versionadded:: 1.2

   One of the buttons in a radio group.
   In each radio group, at most one button can be active at the same time.

   Note that each button in the group also acts as a representation of the group itself.
*/
const char LuaRadioButton::className[] = "RadioButton";
const MethodType<LuaRadioButton> LuaRadioButton::Methods[] = {
   METHOD(LuaRadioButton, set_enabled),
   {nullptr, nullptr},
};
const PropertyType<LuaRadioButton> LuaRadioButton::Properties[] = {
   PROP_RW(LuaRadioButton, state),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: state

      (RW) The index of the radio group's currently active button (0-based; -1 for none).
*/
int LuaRadioButton::get_state(lua_State* L) {
	lua_pushinteger(L, get()->group().get_state());
	return 1;
}
int LuaRadioButton::set_state(lua_State* L) {
	get()->group().set_state(luaL_checkint32(L, -1), true);
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_enabled(b)

      Set whether the user can change the state of the radio group.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaRadioButton::set_enabled(lua_State* L) {
	get()->group().set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
