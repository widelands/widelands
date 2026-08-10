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

#include "scripting/ui/lua_main_menu.h"

#include "scripting/globals.h"
#include "scripting/ui/lua_timer.h"

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
   METHOD(LuaMainMenu, set_keyboard_shortcut), METHOD(LuaMainMenu, set_keyboard_shortcut_release),
   METHOD(LuaMainMenu, add_plugin_timer),      METHOD(LuaMainMenu, get_plugin_timer),
   METHOD(LuaMainMenu, remove_plugin_timer),   {nullptr, nullptr},
};
const PropertyType<LuaMainMenu> LuaMainMenu::Properties[] = {
   PROP_RO(LuaMainMenu, plugin_timers),
   {nullptr, nullptr, nullptr},
};

LuaMainMenu::LuaMainMenu(lua_State* L) : LuaPanel(&get_main_menu(L)) {
}

void LuaMainMenu::__unpersist(lua_State* L) {
	panel_ = &get_main_menu(L);
}

/*
 * Properties
 */

/* RST
   .. attribute:: plugin_timers

      .. versionadded:: 1.4

      (RO) An :class:`array` of all registered :class:`~wl.ui.Timer` instances.

      :see also: :meth:`add_plugin_timer`
      :see also: :meth:`get_plugin_timer`
      :see also: :meth:`remove_plugin_timer`
*/
int LuaMainMenu::get_plugin_timers(lua_State* L) {
	lua_newtable(L);
	int i = 1;
	for (auto& timer : get()->get_plugin_actions().all_timers()) {
		lua_pushint32(L, i++);
		to_lua<LuaTimer>(L, new LuaTimer(timer.get()));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. method:: remove_plugin_timer([name = nil[, only_first = false]])

      .. versionadded:: 1.4

      Remove one or several plugin timers.

      If the parameter ``only_first`` is set to :const:`true`,
      only the first timer matching the name (if any) will be removed;
      otherwise, all matching timers are removed.
      ``only_first`` may only be used if a ``name`` is provided.

      If ``name`` is not :const:`nil`, this method looks for a timer with the given internal name;
      otherwise all timers are matched.

      The method returns the total number of timers removed.

      :arg name: The internal name of the timer to remove, or :const:`nil` for any.
      :type name: :class:`string`
      :arg only_first: Whether to remove at most one timer.
      :type only_first: :class:`boolean`
      :returns: :class:`int`

      :see also: :attr:`plugin_timers`
      :see also: :meth:`add_plugin_timer`
      :see also: :meth:`get_plugin_timer`
*/
int LuaMainMenu::remove_plugin_timer(lua_State* L) {
	const bool only_first = lua_gettop(L) >= 3 && luaL_checkboolean(L, 3);
	std::optional<std::string> name;
	if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
		name = luaL_checkstring(L, 2);
	} else if (only_first) {
		report_error(L, "Cannot remove first plugin timer if no name is given");
	}
	lua_pushuint32(L, get()->get_plugin_actions().remove_timer(name, only_first));
	return 1;
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
   .. method:: add_plugin_timer(action, interval[, name=""[, count=0]][, failsafe=true])

      .. versionchanged:: 1.4
         Added ``name`` and ``count`` parameters and added return type.

      Register a piece of code that will be run periodically as long as the main menu is running.

      :arg action: The Lua code to run.
      :type action: :class:`string`
      :arg interval: The interval in milliseconds realtime in which the code will be invoked.
      :type interval: :class:`int`
      :arg name: The internal name of the timer. Names do not have to be unique.
      :type name: :class:`string`
      :arg count: The number of runs after which the timer will be deactivated.
         Use ``0`` for an endless timer.
      :type count: :class:`int`
      :arg failsafe: In event of an error, an error message is shown and the timer is removed.
         If this is set to :const:`false`, the game will be aborted with no error handling instead.
      :type failsafe: :class:`boolean`
      :returns: :class:`wl.ui.Timer`

      :see also: :attr:`plugin_timers`
      :see also: :meth:`get_plugin_timer`
      :see also: :meth:`remove_plugin_timer`
*/
int LuaMainMenu::add_plugin_timer(lua_State* L) {
	std::string action = luaL_checkstring(L, 2);
	uint32_t interval = luaL_checkuint32(L, 3);

	std::string name;
	int count = 0;
	bool failsafe = true;

	if (const int top = lua_gettop(L); top >= 4) {
		if (lua_isboolean(L, 4)) {
			failsafe = luaL_checkboolean(L, 4);
		} else {
			name = luaL_checkstring(L, 4);
		}
		if (top >= 5) {
			count = luaL_checkuint32(L, 5);
			if (top >= 6) {
				failsafe = luaL_checkboolean(L, 6);
			}
		}
	}

	if (interval == 0) {
		report_error(L, "Timer interval must be non-zero");
	}

	PluginActions::Timer& timer =
	   get()->add_plugin_timer(name, action, interval, count, true, failsafe);
	to_lua<LuaTimer>(L, new LuaTimer(&timer));
	return 1;
}

/* RST
   .. method:: get_plugin_timer(name)

      .. versionadded:: 1.4

      Find a timer by its internal name.
      If multiple timers with the name exist, the first one is returned.
      If no timer with the name exists, :const:`nil` is returned.

      :arg name: The internal name of the timer to find.
      :type name: :class:`string`
      :returns: :class:`wl.ui.Timer`

      :see also: :attr:`plugin_timers`
      :see also: :meth:`add_plugin_timer`
      :see also: :meth:`remove_plugin_timer`
*/
int LuaMainMenu::get_plugin_timer(lua_State* L) {
	PluginActions::Timer* timer = get()->get_plugin_actions().get_timer(luaL_checkstring(L, 2));
	if (timer != nullptr) {
		to_lua<LuaTimer>(L, new LuaTimer(timer));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*
 * C Functions
 */

}  // namespace LuaUi
