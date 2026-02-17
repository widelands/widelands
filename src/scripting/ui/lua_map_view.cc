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

#include "scripting/ui/lua_map_view.h"

#include "logic/game_controller.h"
#include "scripting/globals.h"
#include "scripting/lua_root_notifications.h"
#include "scripting/map/lua_field.h"
#include "scripting/map/lua_flag.h"
#include "scripting/ui/lua_timer.h"
#include "wui/interactive_player.h"

namespace LuaUi {

/* RST
MapView
-------

.. class:: MapView

   The map view is the main widget and the root of all panels. It is the big
   view of the map that is visible at all times while playing.

   This class may only be accessed in a game or the editor.
   You can construct as many instances of it as you like,
   and they will all refer to the same map view.
*/
const char LuaMapView::className[] = "MapView";
const MethodType<LuaMapView> LuaMapView::Methods[] = {
   METHOD(LuaMapView, click),
   METHOD(LuaMapView, start_road_building),
   METHOD(LuaMapView, abort_road_building),
   METHOD(LuaMapView, close),
   METHOD(LuaMapView, scroll_to_field),
   METHOD(LuaMapView, scroll_to_map_pixel),
   METHOD(LuaMapView, is_visible),
   METHOD(LuaMapView, mouse_to_field),
   METHOD(LuaMapView, mouse_to_pixel),
   METHOD(LuaMapView, add_toolbar_plugin),
   METHOD(LuaMapView, update_toolbar),
   METHOD(LuaMapView, set_keyboard_shortcut),
   METHOD(LuaMapView, set_keyboard_shortcut_release),
   METHOD(LuaMapView, add_plugin_timer),
   METHOD(LuaMapView, get_plugin_timer),
   METHOD(LuaMapView, remove_plugin_timer),
   METHOD(LuaMapView, subscribe_to_jump),
   METHOD(LuaMapView, subscribe_to_changeview),
   METHOD(LuaMapView, subscribe_to_track_selection),
   METHOD(LuaMapView, subscribe_to_field_clicked),
   {nullptr, nullptr},
};
const PropertyType<LuaMapView> LuaMapView::Properties[] = {
   PROP_RO(LuaMapView, average_fps),
   PROP_RO(LuaMapView, center_map_pixel),
   PROP_RW(LuaMapView, buildhelp),
   PROP_RW(LuaMapView, census),
   PROP_RW(LuaMapView, statistics),
   PROP_RO(LuaMapView, is_building_road),
   PROP_RO(LuaMapView, auto_roadbuilding_mode),
   PROP_RO(LuaMapView, is_animating),
   PROP_RO(LuaMapView, toolbar),
   PROP_RO(LuaMapView, plugin_timers),
   {nullptr, nullptr, nullptr},
};

LuaMapView::LuaMapView(lua_State* L) : LuaPanel(get_egbase(L).get_ibase()) {
}

void LuaMapView::__unpersist(lua_State* L) {
	panel_ = get_egbase(L).get_ibase();
}

/*
 * Properties
 */
/* RST
   .. attribute:: toolbar

      .. versionadded:: 1.2

      (RO) The main toolbar.
*/
int LuaMapView::get_toolbar(lua_State* L) {
	to_lua<LuaPanel>(L, new LuaPanel(get()->toolbar()));
	return 1;
}

/* RST
   .. attribute:: average_fps

      (RO) The average frames per second that the user interface is being drawn at.
*/
int LuaMapView::get_average_fps(lua_State* L) {
	lua_pushdouble(L, get()->average_fps());
	return 1;
}
/* RST
   .. attribute:: center_map_pixel

      (RO) The map position (in pixels) that the center pixel of this map view
      currently sees. This is a table containing 'x', 'y'.
*/
int LuaMapView::get_center_map_pixel(lua_State* L) {
	const Vector2f center = get()->map_view()->view_area().rect().center();
	lua_newtable(L);

	lua_pushstring(L, "x");
	lua_pushdouble(L, center.x);
	lua_rawset(L, -3);

	lua_pushstring(L, "y");
	lua_pushdouble(L, center.y);
	lua_rawset(L, -3);
	return 1;
}

/* RST
   .. attribute:: buildhelp

      (RW) True if the buildhelp is show, false otherwise.
*/
int LuaMapView::get_buildhelp(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->buildhelp()));
	return 1;
}
int LuaMapView::set_buildhelp(lua_State* L) {
	get()->show_buildhelp(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: census

      (RW) True if the census strings are shown on buildings, false otherwise
*/
int LuaMapView::get_census(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_display_flag(InteractiveBase::dfShowCensus)));
	return 1;
}
int LuaMapView::set_census(lua_State* L) {
	get()->set_display_flag(InteractiveBase::dfShowCensus, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: statistics

      (RW) True if the statistics strings are shown on buildings, false
      otherwise
*/
int LuaMapView::get_statistics(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_display_flag(InteractiveBase::dfShowStatistics)));
	return 1;
}
int LuaMapView::set_statistics(lua_State* L) {
	get()->set_display_flag(InteractiveBase::dfShowStatistics, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: is_building_road

      (RO) Is the player currently in road/waterway building mode?
*/
int LuaMapView::get_is_building_road(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->in_road_building_mode()));
	return 1;
}

/* RST
   .. attribute:: auto_roadbuild_mode

      (RO) Is the player using automatic road building mode?
*/
int LuaMapView::get_auto_roadbuilding_mode(lua_State* L) {
	InteractivePlayer* ipl = get_game(L).get_ipl();
	if (ipl == nullptr) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, static_cast<int>(ipl->auto_roadbuild_mode()));
	}
	return 1;
}

/* RST
   .. attribute:: is_animating

      (RO) True if this MapView is currently panning or zooming.
*/
int LuaMapView::get_is_animating(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->map_view()->is_animating()));
	return 1;
}

/* RST
   .. attribute:: plugin_timers

      .. versionadded:: 1.4

      (RO) An :class:`array` of all registered :class:`~wl.ui.Timer` instances.

      :see also: :meth:`add_plugin_timer`
      :see also: :meth:`get_plugin_timer`
      :see also: :meth:`remove_plugin_timer`
*/
int LuaMapView::get_plugin_timers(lua_State* L) {
	lua_newtable(L);
	int i = 1;
	for (auto& timer : get_egbase(L).get_ibase()->get_plugin_actions().all_timers()) {
		lua_pushint32(L, i++);
		to_lua<LuaTimer>(L, new LuaTimer(timer.get()));
		lua_rawset(L, -3);
	}
	return 1;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: click(field)

      Jumps the mouse onto a field and clicks it just like the user would
      have.

      :arg field: the field to click on
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::click(lua_State* L) {
	auto* const field = *get_user_class<LuaMaps::LuaField>(L, 2);
	get()->map_view()->mouse_to_field(field->coords(), MapView::Transition::Jump);

	// We fake the triangle here, since we only support clicking on Nodes from
	// Lua.
	Widelands::NodeAndTriangle<> node_and_triangle{
	   field->coords(), Widelands::TCoords<>(field->coords(), Widelands::TriangleIndex::D)};
	get()->map_view()->field_clicked(node_and_triangle);
	return 0;
}

/* RST
   .. method:: start_road_building(flag[, waterway = false])

      Enters the road building mode as if the player has clicked
      the flag and chosen build road. It will also warp the mouse
      to the given starting node. Throws an error if we are already in road
      building mode.

      :arg flag: :class:`wl.map.Flag` object to start building from.
      :arg waterway: if `true`, start building a waterway rather than a road
*/
// UNTESTED
int LuaMapView::start_road_building(lua_State* L) {
	InteractiveBase* me = get();
	if (me->in_road_building_mode()) {
		report_error(L, "Already building road!");
	}

	Widelands::Coords starting_field =
	   (*get_user_class<LuaMaps::LuaFlag>(L, 2))->get(L, get_egbase(L))->get_position();

	me->map_view()->mouse_to_field(starting_field, MapView::Transition::Jump);
	me->start_build_road(starting_field, me->get_player()->player_number(),
	                     lua_gettop(L) > 2 && luaL_checkboolean(L, 3) ? RoadBuildingType::kWaterway :
	                                                                    RoadBuildingType::kRoad);

	return 0;
}

/* RST
   .. method:: abort_road_building

      If the player is currently in road building mode, this will cancel it.
      If he wasn't, this will do nothing.
*/
// UNTESTED
int LuaMapView::abort_road_building(lua_State* /* L */) {
	InteractiveBase* me = get();
	if (me->in_road_building_mode()) {
		me->abort_build_road();
	}
	return 0;
}

/* RST
   .. method:: close

      Closes the MapView. Note that this is the equivalent as clicking on
      the exit button in the game; that is the game will be exited.

      This is especially useful for automated testing of features and is for
      example used in the widelands Lua test suite.
*/
int LuaMapView::close(lua_State* /* l */) {
	get()->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	return 0;
}

/* RST
   .. method:: scroll_to_map_pixel(x, y)

      Starts an animation to center the view on top of the pixel (x, y) in map
      pixel space. Use `is_animating` to check if the animation is still going
      on.

      :arg x: x coordinate of the pixel
      :type x: number
      :arg y: y coordinate of the pixel
      :type y: number
*/
int LuaMapView::scroll_to_map_pixel(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	// don't move view in replays
	if (egbase.is_game() &&
	    dynamic_cast<Widelands::Game&>(egbase).game_controller()->get_game_type() ==
	       GameController::GameType::kReplay) {
		return 0;
	}

	const Vector2f center(luaL_checkdouble(L, 2), luaL_checkdouble(L, 3));
	get()->map_view()->scroll_to_map_pixel(center, MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: scroll_to_field(field)

      Starts an animation to center the view on top of the 'field'. Use
      `is_animating` to check if the animation is still going on.

      :arg field: the field to center on
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::scroll_to_field(lua_State* L) {
	get()->map_view()->scroll_to_field(
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->coords(), MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: is_visible(field)

      Returns `true` if `field` is currently visible in the map view.

      :arg field: the field
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::is_visible(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->map_view()->view_area().contains(
	                      (*get_user_class<LuaMaps::LuaField>(L, 2))->coords())));
	return 1;
}

/* RST
   .. method:: mouse_to_pixel(x, y)

      Starts an animation to move the mouse onto the pixel (x, y) of this panel.
      Use `is_animating` to check if the animation is still going on.

      :arg x: x coordinate of the pixel
      :type x: number
      :arg y: y coordinate of the pixel
      :type y: number
*/
int LuaMapView::mouse_to_pixel(lua_State* L) {
	int x = luaL_checkint32(L, 2);
	int y = luaL_checkint32(L, 3);
	get()->map_view()->mouse_to_pixel(Vector2i(x, y), MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: mouse_to_field(field)

      Starts an animation to move the mouse onto the 'field'. If 'field' is not
      visible on the screen currently, does nothing. Use `is_animating` to
      check if the animation is still going on.

      :arg field: the field
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::mouse_to_field(lua_State* L) {
	get()->map_view()->mouse_to_field(
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->coords(), MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: update_toolbar()

      .. versionadded:: 1.2

      Recompute the size and position of the toolbar.
      Call this after you have modified the toolbar in any way.
*/
int LuaMapView::update_toolbar(lua_State* L) {
	get_egbase(L).get_ibase()->finalize_toolbar();
	return 0;
}

/* RST
   .. method:: add_toolbar_plugin(action, icon, name[, tooltip = "", hotkey = nil])

      .. versionadded:: 1.2

      .. versionchanged:: 1.3
         Added ``hotkey`` parameter.

      Add an entry to the main toolbar's Plugin dropdown.
      This makes the plugin dropdown visible if it was hidden.

      :arg action: The Lua code to run when the user selects the entry.
      :type action: :class:`string`
      :arg icon: Icon filepath for the entry.
      :type icon: :class:`string`
      :arg name: Label for the entry.
      :type name: :class:`string`
      :arg tooltip: Tooltip for the entry.
      :type tooltip: :class:`string`
      :arg hotkey: The internal name of the hotkey for this entry.
      :type hotkey: :class:`string`
*/
int LuaMapView::add_toolbar_plugin(lua_State* L) {
	get_egbase(L).get_ibase()->add_toolbar_plugin(
	   luaL_checkstring(L, 2), luaL_checkstring(L, 3), luaL_checkstring(L, 4),
	   lua_gettop(L) >= 5 ? luaL_checkstring(L, 5) : "",
	   lua_gettop(L) >= 6 ? shortcut_string_if_set(luaL_checkstring(L, 6), false) : "");
	return 0;
}

/* RST
   .. method:: set_keyboard_shortcut(internal_name, action[, failsafe=true])

      .. versionadded:: 1.3

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
int LuaMapView::set_keyboard_shortcut(lua_State* L) {
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

      .. versionadded:: 1.3

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
int LuaMapView::set_keyboard_shortcut_release(lua_State* L) {
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

      .. versionadded:: 1.2

      .. versionchanged:: 1.4
         Added ``name`` and ``count`` parameters and added return type.

      Register a piece of code that will be run periodically as long as the game/editor is running.

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
int LuaMapView::add_plugin_timer(lua_State* L) {
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
	   get_egbase(L).get_ibase()->add_plugin_timer(name, action, interval, count, true, failsafe);
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
int LuaMapView::get_plugin_timer(lua_State* L) {
	PluginActions::Timer* timer =
	   get_egbase(L).get_ibase()->get_plugin_actions().get_timer(luaL_checkstring(L, 2));
	if (timer != nullptr) {
		to_lua<LuaTimer>(L, new LuaTimer(timer));
	} else {
		lua_pushnil(L);
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
int LuaMapView::remove_plugin_timer(lua_State* L) {
	const bool only_first = lua_gettop(L) >= 3 && luaL_checkboolean(L, 3);
	std::optional<std::string> name;
	if (lua_gettop(L) >= 2 && !lua_isnil(L, 2)) {
		name = luaL_checkstring(L, 2);
	} else if (only_first) {
		report_error(L, "Cannot remove first plugin timer if no name is given");
	}
	lua_pushuint32(L, get_egbase(L).get_ibase()->get_plugin_actions().remove_timer(name, only_first));
	return 1;
}

/* RST
   .. method:: subscribe_to_jump()

      .. versionadded:: 1.3

      Subscribe to the "jump" signal, which is triggered when
      the viewport jumps immediately to a different position.

      This signal is not triggered by animated map transitions.

      This signal provides no arguments.

      :returns: :class:`wl.Subscriber`
*/
int LuaMapView::subscribe_to_jump(lua_State* L) {
	to_lua<LuaRoot::LuaSubscriber>(
	   L,
	   new LuaRoot::LuaSubscriber(get_egbase(L), LuaRoot::LuaNotifications::create_mapview_jump(L)));
	return 1;
}

/* RST
   .. method:: subscribe_to_changeview()

      .. versionadded:: 1.3

      Subscribe to the "changeview" signal, which is triggered when the viewport moves.

      This signal is triggered repeatedly by animated map transitions.

      This signal provides no arguments.

      :returns: :class:`wl.Subscriber`
*/
int LuaMapView::subscribe_to_changeview(lua_State* L) {
	to_lua<LuaRoot::LuaSubscriber>(
	   L, new LuaRoot::LuaSubscriber(
	         get_egbase(L), LuaRoot::LuaNotifications::create_mapview_changeview(L)));
	return 1;
}

/* RST
   .. method:: subscribe_to_field_clicked()

      .. versionadded:: 1.3

      Subscribe to the "field_clicked" signal, which is triggered when the user clicks on a field.

      Note that a user's click on a field can have other, unrelated effects, such as opening a
      field action window or building window, interacting with road building mode, or
      (in the editor) performing a tool action.
      It is not currently possible to suppress these effects.
      The ability to create custom tools that trigger on a click on a field without other effects
      is a `planned future addition <https://codeberg.org/wl/widelands/issues/5273>`_.

      The signal provides as arguments both the node and the triangle which are
      closest to the clicked pixel.
      The coordinates of the node are given by the keys ``"node_x"`` and ``"node_y"``.
      The coordinates of the triangle are given by the keys ``"triangle_x"``, ``"triangle_y"``,
      and ``"triangle_t"``, where ``"triangle_t"`` is one of ``"D"`` and ``"R"``.

      :returns: :class:`wl.Subscriber`
*/
int LuaMapView::subscribe_to_field_clicked(lua_State* L) {
	to_lua<LuaRoot::LuaSubscriber>(
	   L, new LuaRoot::LuaSubscriber(
	         get_egbase(L), LuaRoot::LuaNotifications::create_mapview_field_clicked(L)));
	return 1;
}

/* RST
   .. method:: subscribe_to_track_selection()

      .. versionadded:: 1.3

      Subscribe to the "track_selection" signal, which is triggered when
      the field under the mouse cursor changes.

      The signal provides as arguments both the node and the triangle which are
      closest to the mouse pixel.
      The coordinates of the node are given by the keys ``"node_x"`` and ``"node_y"``.
      The coordinates of the triangle are given by the keys ``"triangle_x"``, ``"triangle_y"``,
      and ``"triangle_t"``, where ``"triangle_t"`` is one of ``"D"`` and ``"R"``.

      :returns: :class:`wl.Subscriber`
*/
int LuaMapView::subscribe_to_track_selection(lua_State* L) {
	to_lua<LuaRoot::LuaSubscriber>(
	   L, new LuaRoot::LuaSubscriber(
	         get_egbase(L), LuaRoot::LuaNotifications::create_mapview_track_selection(L)));
	return 1;
}

/*
 * C Functions
 */

}  // namespace LuaUi
