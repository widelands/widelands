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

#include "scripting/ui/lua_spin_box.h"

namespace LuaUi {

/* RST
SpinBox
-------

.. class:: SpinBox

   .. versionadded:: 1.2

   A box with buttons to increase or decrease a numerical value.
*/
const char LuaSpinBox::className[] = "SpinBox";
const MethodType<LuaSpinBox> LuaSpinBox::Methods[] = {
   METHOD(LuaSpinBox, set_unit_width),
   METHOD(LuaSpinBox, set_interval),
   METHOD(LuaSpinBox, add_replacement),
   {nullptr, nullptr},
};
const PropertyType<LuaSpinBox> LuaSpinBox::Properties[] = {
   PROP_RW(LuaSpinBox, value),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: value

      (RW) The currently selected value.
*/
int LuaSpinBox::get_value(lua_State* L) {
	lua_pushinteger(L, get()->get_value());
	return 1;
}
int LuaSpinBox::set_value(lua_State* L) {
	get()->set_value(luaL_checkint32(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_unit_width(w)

      Set the width of the spinbox's buttons and content.

      :arg w: Width in pixels.
      :type min: :class:`int`
*/
int LuaSpinBox::set_unit_width(lua_State* L) {
	get()->set_unit_width(luaL_checkuint32(L, 2));
	return 0;
}

/* RST
   .. function:: set_interval(min, max)

      Set the minimum and maximum value of the spinbox.

      :arg min: Minimum value.
      :type min: :class:`int`
      :arg max: Maximum value.
      :type max: :class:`int`
*/
int LuaSpinBox::set_interval(lua_State* L) {
	get()->set_interval(luaL_checkint32(L, 2), luaL_checkint32(L, 3));
	return 0;
}

/* RST
   .. function:: add_replacement(value, label)

      Replacement string to display instead of a specific value when that value is selected.

      :arg value: Value to replace.
      :type value: :class:`int`
      :arg label: Replacement text.
      :type label: :class:`string`
*/
int LuaSpinBox::add_replacement(lua_State* L) {
	get()->add_replacement(luaL_checkint32(L, 2), luaL_checkstring(L, 3));
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
