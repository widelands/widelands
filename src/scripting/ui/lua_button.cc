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

#include "scripting/ui/lua_button.h"

#include "base/log.h"

namespace LuaUi {

/* RST
Button
------

.. class:: Button

   This represents a simple push button.
*/
const char LuaButton::className[] = "Button";
const MethodType<LuaButton> LuaButton::Methods[] = {
   METHOD(LuaButton, press),         METHOD(LuaButton, click),
   METHOD(LuaButton, set_repeating), METHOD(LuaButton, set_perm_pressed),
   METHOD(LuaButton, toggle),        {nullptr, nullptr},
};
const PropertyType<LuaButton> LuaButton::Properties[] = {
   PROP_RW(LuaButton, title),
   PROP_RW(LuaButton, enabled),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: title

      .. versionadded:: 1.2

      (RW) The text shown on the button.
*/
int LuaButton::get_title(lua_State* L) {
	lua_pushstring(L, get()->get_title().c_str());
	return 1;
}
int LuaButton::set_title(lua_State* L) {
	get()->set_title(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: enabled

      .. versionadded:: 1.2

      (RW) Whether the user may interact with the button.
*/
int LuaButton::get_enabled(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->enabled()));
	return 1;
}
int LuaButton::set_enabled(lua_State* L) {
	get()->set_enabled(luaL_checkboolean(L, -1));
	return 0;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: press()

      Press and hold this button. This is mainly to visualize a pressing
      event in tutorials.

      Holding a button does not generate a Clicked event.
      Use :meth:`click` to release the button.
*/
int LuaButton::press(lua_State* /* L */) {
	verb_log_info("Pressing button '%s'\n", get()->get_name().c_str());
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
/* RST
   .. method:: click()

      Click this button just as if the user would have moused over and clicked
      it.
*/
int LuaButton::click(lua_State* /* L */) {
	verb_log_info("Clicking button '%s'\n", get()->get_name().c_str());
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	get()->handle_mouserelease(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}

/* RST
   .. method:: set_repeating(b)

      .. versionadded:: 1.2

      Set whether holding the button will generate repeated clicked events.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaButton::set_repeating(lua_State* L) {
	get()->set_repeating(luaL_checkboolean(L, 2));
	return 0;
}

/* RST
   .. method:: set_perm_pressed(b)

      .. versionadded:: 1.2

      Set whether the button will be styled as if permanently pressed.

      :arg b: :const:`true` or :const:`false`
      :type b: :class:`boolean`
*/
int LuaButton::set_perm_pressed(lua_State* L) {
	get()->set_perm_pressed(luaL_checkboolean(L, 2));
	return 0;
}

/* RST
   .. method:: toggle()

      .. versionadded:: 1.2

      Toggle whether the button will be styled as if permanently pressed.
*/
int LuaButton::toggle(lua_State* /* L */) {
	get()->toggle();
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
