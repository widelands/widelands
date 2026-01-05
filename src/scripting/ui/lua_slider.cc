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

#include "scripting/ui/lua_slider.h"

namespace LuaUi {

/* RST
Slider
------

.. class:: Slider

   .. versionadded:: 1.2

   A button that can be slid along a line to change a value.
*/
const char LuaSlider::className[] = "Slider";
const MethodType<LuaSlider> LuaSlider::Methods[] = {
   METHOD(LuaSlider, set_enabled),
   METHOD(LuaSlider, set_cursor_fixed_height),
   {nullptr, nullptr},
};
const PropertyType<LuaSlider> LuaSlider::Properties[] = {
   PROP_RW(LuaSlider, value),
   PROP_RW(LuaSlider, min_value),
   PROP_RW(LuaSlider, max_value),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: value

      (RW) The currently selected value.
*/
int LuaSlider::get_value(lua_State* L) {
	lua_pushinteger(L, get()->get_value());
	return 1;
}
int LuaSlider::set_value(lua_State* L) {
	get()->set_value(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: min_value

      (RW) The lowest selectable value.
*/
int LuaSlider::get_min_value(lua_State* L) {
	lua_pushinteger(L, get()->get_min_value());
	return 1;
}
int LuaSlider::set_min_value(lua_State* L) {
	get()->set_min_value(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: max_value

      (RW) The highest selectable value.
*/
int LuaSlider::get_max_value(lua_State* L) {
	lua_pushinteger(L, get()->get_max_value());
	return 1;
}
int LuaSlider::set_max_value(lua_State* L) {
	get()->set_max_value(luaL_checkint32(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/* RST
   .. function:: set_enabled(b)

      Set whether the user can move this slider.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaSlider::set_enabled(lua_State* L) {
	get()->set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. function:: set_cursor_fixed_height(h)

      Set the slider cursor's height.

      :arg h: Height in pixels.
      :type h: :class:`int`
*/
int LuaSlider::set_cursor_fixed_height(lua_State* L) {
	get()->set_cursor_fixed_height(luaL_checkint32(L, -1));
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
