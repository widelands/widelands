/*
 * Copyright (C) 2026 by the Widelands Development Team
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

#include "scripting/ui/lua_timer.h"

namespace LuaUi {

/* RST
Timer
------

.. class:: Timer

   .. versionadded:: 1.4

   A plugin timer wraps a snippet of Lua code that is periodically
   executed by the main menu or map view.

   :see also: :meth:`wl.ui.MapView.add_plugin_timer`
   :see also: :meth:`wl.ui.MapView.get_plugin_timer`
   :see also: :attr:`wl.ui.MapView.plugin_timers`
   :see also: :meth:`wl.ui.MapView.remove_plugin_timer`
   :see also: :meth:`wl.ui.MainMenu.add_plugin_timer`
   :see also: :meth:`wl.ui.MainMenu.get_plugin_timer`
   :see also: :attr:`wl.ui.MainMenu.plugin_timers`
   :see also: :meth:`wl.ui.MainMenu.remove_plugin_timer`
*/
const char LuaTimer::className[] = "Timer";
const MethodType<LuaTimer> LuaTimer::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTimer> LuaTimer::Properties[] = {
   PROP_RW(LuaTimer, active),   PROP_RW(LuaTimer, failsafe), PROP_RW(LuaTimer, remaining_count),
   PROP_RW(LuaTimer, name),     PROP_RW(LuaTimer, action),   PROP_RW(LuaTimer, interval),
   PROP_RW(LuaTimer, next_run), {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: active

      (RW) Whether the timer is currently active.

      Note that reactivating an expired timer will turn it into an endless timer
      unless you explicitly set :attr:`remaining_count` as well.
*/
int LuaTimer::get_active(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->active));
	return 1;
}
int LuaTimer::set_active(lua_State* L) {
	get()->active = luaL_checkboolean(L, -1);
	return 0;
}

/* RST
   .. attribute:: failsafe

      (RW) Whether the timer will be safely deleted with an error message if a Lua error occurs,
      instead of crashing without error handling.
*/
int LuaTimer::get_failsafe(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->failsafe));
	return 1;
}
int LuaTimer::set_failsafe(lua_State* L) {
	get()->failsafe = luaL_checkboolean(L, -1);
	return 0;
}

/* RST
   .. attribute:: remaining_count

      (RW) How many more times this timer will be run before being automatically deactivated.

      ``0`` denotes a timer that runs endlessly.
*/
int LuaTimer::get_remaining_count(lua_State* L) {
	lua_pushuint32(L, get()->remaining_count);
	return 1;
}
int LuaTimer::set_remaining_count(lua_State* L) {
	get()->remaining_count = luaL_checkuint32(L, -1);
	return 0;
}

/* RST
   .. attribute:: name

      (RW) The internal name of this timer.
*/
int LuaTimer::get_name(lua_State* L) {
	lua_pushstring(L, get()->name.c_str());
	return 1;
}
int LuaTimer::set_name(lua_State* L) {
	get()->name = luaL_checkstring(L, -1);
	return 0;
}

/* RST
   .. attribute:: action

      (RW) The Lua code run on each invocation of this timer.
*/
int LuaTimer::get_action(lua_State* L) {
	lua_pushstring(L, get()->action.c_str());
	return 1;
}
int LuaTimer::set_action(lua_State* L) {
	get()->action = luaL_checkstring(L, -1);
	return 0;
}

/* RST
   .. attribute:: interval

      (RW) The interval in milliseconds realtime how frequently this timer is run.
*/
int LuaTimer::get_interval(lua_State* L) {
	lua_pushuint32(L, get()->interval);
	return 1;
}
int LuaTimer::set_interval(lua_State* L) {
	get()->interval = luaL_checkuint32(L, -1);
	return 0;
}

/* RST
   .. attribute:: next_run

      (RW) The timestamp in milliseconds realtime since SDL initialization
      when this timer should next be run.

      :see also: :meth:`ticks`
*/
int LuaTimer::get_next_run(lua_State* L) {
	lua_pushuint32(L, get()->next_run);
	return 1;
}
int LuaTimer::set_next_run(lua_State* L) {
	get()->next_run = luaL_checkuint32(L, -1);
	return 0;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

}  // namespace LuaUi
