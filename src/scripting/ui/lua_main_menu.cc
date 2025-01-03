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

#include "scripting/ui/lua_main_menu.h"

#include "scripting/globals.h"

namespace LuaUi {

/* RST
MainMenu
--------

.. class:: MainMenu

   .. versionadded:: 1.3

   The main menu screen is the main widget and the root of all panels.

   This class may not be accessed in a game or the editor.
   You can construct as many instances of it as you like,
   and they will all refer to the same main menu.
*/
const char LuaMainMenu::className[] = "MainMenu";
const MethodType<LuaMainMenu> LuaMainMenu::Methods[] = {
   METHOD(LuaMainMenu, set_keyboard_shortcut),
   METHOD(LuaMainMenu, set_keyboard_shortcut_release),
   METHOD(LuaMainMenu, add_plugin_timer),
   {nullptr, nullptr},
};
const PropertyType<LuaMainMenu> LuaMainMenu::Properties[] = {
   {nullptr, nullptr, nullptr},
};

LuaMainMenu::LuaMainMenu(lua_State* L) : LuaPanel(&get_main_menu(L)) {
}

void LuaMainMenu::__unpersist(lua_State* L) {
	panel_ = &get_main_menu(L);
}

/*
 * Lua Functions
 */

/* RST
   .. method:: set_keyboard_shortcut(internal_name, action[, failsafe=true])

      Associate a named keyboard shortcut with a piece of code to run when the shortcut is pressed.
      This replaces any existing action associated with pressing the shortcut.

      :arg internal_name: The internal name of the keyboard shortcut.
      :type internal_name: :class:`string`
      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg failsafe: In event of an error, an error message is shown and the shortcut binding
         is removed. If this is set to :const:`false`, the game will be aborted with no
         error handling instead.
      :type failsafe: :class:`boolean`

      :see also: :meth:`set_keyboard_shortcut_release`
*/
int LuaMainMenu::set_keyboard_shortcut(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	std::string action = luaL_checkstring(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);
	if (!shortcut_exists(name)) {
		report_error(L, "Invalid shortcut name '%s'", name.c_str());
	}
	get()->set_lua_shortcut(name, action, failsafe, true);
	return 0;
}

/* RST
   .. method:: set_keyboard_shortcut_release(internal_name, action[, failsafe=true])

      Associate a named keyboard shortcut with a piece of code to run when the shortcut is released
      after having been previously pressed.
      This replaces any existing action associated with releasing the shortcut.

      You don't need this in normal cases. When in doubt, use only meth:`set_keyboard_shortcut`.

      :arg internal_name: The internal name of the keyboard shortcut.
      :type internal_name: :class:`string`
      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg failsafe: In event of an error, an error message is shown and the shortcut binding
         is removed. If this is set to :const:`false`, the game will be aborted with no
         error handling instead.
      :type failsafe: :class:`boolean`

      :see also: :meth:`set_keyboard_shortcut`
*/
int LuaMainMenu::set_keyboard_shortcut_release(lua_State* L) {
	std::string name = luaL_checkstring(L, 2);
	std::string action = luaL_checkstring(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);
	if (!shortcut_exists(name)) {
		report_error(L, "Invalid shortcut name '%s'", name.c_str());
	}
	get()->set_lua_shortcut(name, action, failsafe, false);
	return 0;
}

/* RST
   .. method:: add_plugin_timer(action, interval[, failsafe=true])

      Register a piece of code that will be run periodically as long as the main menu is running
      and its Lua context is not reset.

      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg interval: The interval in milliseconds realtime in which the code will be invoked.
      :type interval: :class:`int`
      :arg failsafe: In event of an error, an error message is shown and the timer is removed.
         If this is set to :const:`false`, the game will be aborted with no error handling instead.
      :type failsafe: :class:`boolean`
*/
int LuaMainMenu::add_plugin_timer(lua_State* L) {
	std::string action = luaL_checkstring(L, 2);
	uint32_t interval = luaL_checkuint32(L, 3);
	bool failsafe = lua_gettop(L) < 4 || luaL_checkboolean(L, 4);

	if (interval == 0) {
		report_error(L, "Timer interval must be non-zero");
	}

	get()->add_plugin_timer(action, interval, failsafe);
	return 0;
}

/*
 * C Functions
 */

}  // namespace LuaUi
