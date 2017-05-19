/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "scripting/lua_ui.h"

#include "base/macros.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "scripting/globals.h"
#include "scripting/lua_map.h"
#include "scripting/luna.h"
#include "wui/interactive_player.h"

namespace LuaUi {

/* RST
:mod:`wl.ui`
=============

.. module:: wl.ui
   :synopsis: Provides access on user interface. Mainly for tutorials and
      debugging.

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.ui

.. Note::

   The objects inside this module can not be persisted. That is if the player
   tries to save the game while any of these objects are assigned to variables,
   the game will crash. So when using these, make sure that you only create
   objects for a short amount of time where the user can't take control to do
   something else.

*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/

/* RST
Panel
-----

.. class:: Panel

   The Panel is the most basic ui class. Each ui element is a panel.
*/
const char LuaPanel::className[] = "Panel";
const PropertyType<LuaPanel> LuaPanel::Properties[] = {
   PROP_RO(LuaPanel, buttons),    PROP_RO(LuaPanel, tabs),       PROP_RO(LuaPanel, windows),
   PROP_RW(LuaPanel, position_x), PROP_RW(LuaPanel, position_y), PROP_RW(LuaPanel, width),
   PROP_RW(LuaPanel, height),     {nullptr, nullptr, nullptr},
};
const MethodType<LuaPanel> LuaPanel::Methods[] = {
   METHOD(LuaPanel, get_descendant_position), {nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: name

      (RO) The name of this panel
*/

/* RST
   .. attribute:: buttons

      (RO) An :class:`array` of all visible buttons inside this Panel.
*/
static void put_all_visible_buttons_into_table(lua_State* L, UI::Panel* g) {
	if (!g)
		return;

	for (UI::Panel* f = g->get_first_child(); f; f = f->get_next_sibling()) {
		put_all_visible_buttons_into_table(L, f);

		if (upcast(UI::Button, b, f))
			if (b->is_visible()) {
				lua_pushstring(L, b->get_name());
				to_lua<LuaButton>(L, new LuaButton(b));
				lua_rawset(L, -3);
			}
	}
}
int LuaPanel::get_buttons(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_visible_buttons_into_table(L, panel_);

	return 1;
}

/* RST
   .. attribute:: tabs

      (RO) An :class:`array` of all visible tabs inside this Panel.
*/
static void put_all_tabs_into_table(lua_State* L, UI::Panel* g) {
	if (!g)
		return;

	for (UI::Panel* f = g->get_first_child(); f; f = f->get_next_sibling()) {
		put_all_tabs_into_table(L, f);

		if (upcast(UI::TabPanel, t, f))
			for (UI::Tab* tab : t->tabs()) {
				lua_pushstring(L, tab->get_name());
				to_lua<LuaTab>(L, new LuaTab(tab));
				lua_rawset(L, -3);
			}
	}
}
int LuaPanel::get_tabs(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_tabs_into_table(L, panel_);

	return 1;
}

/* RST
   .. attribute:: windows

      (RO) A :class:`array` of all currently open windows that are
         children of this Panel.
*/
static void put_all_visible_windows_into_table(lua_State* L, UI::Panel* g) {
	if (!g)
		return;

	for (UI::Panel* f = g->get_first_child(); f; f = f->get_next_sibling()) {
		put_all_visible_windows_into_table(L, f);

		if (upcast(UI::Window, win, f)) {
			lua_pushstring(L, win->get_name());
			to_lua<LuaWindow>(L, new LuaWindow(win));
			lua_rawset(L, -3);
		}
	}
}
int LuaPanel::get_windows(lua_State* L) {
	assert(panel_);

	lua_newtable(L);
	put_all_visible_windows_into_table(L, panel_);

	return 1;
}

/* RST
   .. attribute:: width, height

      (RW) The dimensions of this panel in pixels
*/
int LuaPanel::get_width(lua_State* L) {
	assert(panel_);
	lua_pushint32(L, panel_->get_w());
	return 1;
}
int LuaPanel::set_width(lua_State* L) {
	assert(panel_);
	panel_->set_size(luaL_checkint32(L, -1), panel_->get_h());
	return 1;
}
int LuaPanel::get_height(lua_State* L) {
	assert(panel_);
	lua_pushint32(L, panel_->get_h());
	return 1;
}
int LuaPanel::set_height(lua_State* L) {
	assert(panel_);
	panel_->set_size(panel_->get_w(), luaL_checkint32(L, -1));
	return 1;
}

/* RST
   .. attribute:: position_x, position_y

      (RO) The top left pixel of the our inner canvas relative to the
      parent's element inner canvas.
*/
int LuaPanel::get_position_x(lua_State* L) {
	assert(panel_);
	Vector2i p = panel_->to_parent(Vector2i::zero());

	lua_pushint32(L, p.x);
	return 1;
}
int LuaPanel::set_position_x(lua_State* L) {
	assert(panel_);
	Vector2i p(luaL_checkint32(L, -1) - panel_->get_lborder(), panel_->get_y());
	panel_->set_pos(p);
	return 1;
}
int LuaPanel::get_position_y(lua_State* L) {
	assert(panel_);
	Vector2i p = panel_->to_parent(Vector2i::zero());

	lua_pushint32(L, p.y);
	return 1;
}
int LuaPanel::set_position_y(lua_State* L) {
	assert(panel_);
	Vector2i p(panel_->get_x(), luaL_checkint32(L, -1) - panel_->get_tborder());
	panel_->set_pos(p);
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: get_descendant_position(child)

      Get the child position relative to the inner canvas of this Panel in
      pixels. Throws an error if child is not really a child.

      :arg child: children to get position for
      :type child: :class:`Panel`

      :returns: x, y
      :rtype: both are :class:`integers`
*/
int LuaPanel::get_descendant_position(lua_State* L) {
	assert(panel_);

	UI::Panel* cur = (*get_base_user_class<LuaPanel>(L, 2))->panel_;

	Vector2i cp = Vector2i::zero();
	while (cur && cur != panel_) {
		cp += cur->to_parent(Vector2i::zero());
		cur = cur->get_parent();
	}

	if (!cur)
		report_error(L, "Widget is not a descendant!");

	lua_pushint32(L, cp.x);
	lua_pushint32(L, cp.y);
	return 2;
}

/*
 * C Functions
 */

/* RST
Button
------

.. class:: Button

   Child of: :class:`Panel`

   This represents a simple push button.
*/
const char LuaButton::className[] = "Button";
const MethodType<LuaButton> LuaButton::Methods[] = {
   METHOD(LuaButton, press), METHOD(LuaButton, click), {nullptr, nullptr},
};
const PropertyType<LuaButton> LuaButton::Properties[] = {
   PROP_RO(LuaButton, name), {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaButton::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

/*
 * Lua Functions
 */
/* RST
   .. method:: press

      Press and hold this button. This is mainly to visualize a pressing
      event in tutorials
*/
int LuaButton::press(lua_State* /* L */) {
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
/* RST
   .. method:: click

      Click this button just as if the user would have moused over and clicked
      it.
*/
int LuaButton::click(lua_State* /* L */) {
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	get()->handle_mouserelease(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}

/*
 * C Functions
 */

/* RST
Tab
------

.. class:: Tab

   Child of: :class:`Panel`

   A tab button.
*/
const char LuaTab::className[] = "Tab";
const MethodType<LuaTab> LuaTab::Methods[] = {
   METHOD(LuaTab, click), {nullptr, nullptr},
};
const PropertyType<LuaTab> LuaTab::Properties[] = {
   PROP_RO(LuaTab, name), PROP_RO(LuaTab, active), {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaTab::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

/* RST
   .. attribute:: active

      (RO) Is this the currently active tab in this window?
*/
int LuaTab::get_active(lua_State* L) {
	lua_pushboolean(L, get()->active());
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
	get()->activate();
	return 0;
}

/*
 * C Functions
 */

/* RST
Window
------

.. class:: Window

   Child of: :class:`Panel`

   This represents a Window.
*/
const char LuaWindow::className[] = "Window";
const MethodType<LuaWindow> LuaWindow::Methods[] = {
   METHOD(LuaWindow, close), {nullptr, nullptr},
};
const PropertyType<LuaWindow> LuaWindow::Properties[] = {
   PROP_RO(LuaWindow, name), {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int LuaWindow::get_name(lua_State* L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

/*
 * Lua Functions
 */

/* RST
   .. method:: close

      Closes this window. This invalidates this Object, do
      not use it any longer.
*/
int LuaWindow::close(lua_State* /* L */) {
	delete panel_;
	panel_ = nullptr;
	return 0;
}

/*
 * C Functions
 */

/* RST
MapView
-------

.. class:: MapView

   Child of :class:`Panel`

   The map view is the main widget and the root of all panels. It is the big
   view of the map that is visible at all times while playing.
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
   {nullptr, nullptr},
};
const PropertyType<LuaMapView> LuaMapView::Properties[] = {
   PROP_RO(LuaMapView, center_map_pixel), PROP_RW(LuaMapView, buildhelp),
   PROP_RW(LuaMapView, census),           PROP_RW(LuaMapView, statistics),
   PROP_RO(LuaMapView, is_building_road), PROP_RO(LuaMapView, is_animating),
   {nullptr, nullptr, nullptr},
};

LuaMapView::LuaMapView(lua_State* L) : LuaPanel(get_egbase(L).get_ibase()) {
}

void LuaMapView::__unpersist(lua_State* L) {
	Widelands::Game& game = get_game(L);
	panel_ = game.get_ibase();
}

/*
 * Properties
 */
/* RST
   .. attribute:: center_map_pixel

		(RO) The map position (in pixels) that the center pixel of this map view
		currently sees. This is a table containing 'x', 'y'.
*/
int LuaMapView::get_center_map_pixel(lua_State* L) {
	const Vector2f center = get()->view_area().rect().center();
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
	lua_pushboolean(L, get()->buildhelp());
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
	lua_pushboolean(L, get()->get_display_flag(InteractiveBase::dfShowCensus));
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
	lua_pushboolean(L, get()->get_display_flag(InteractiveBase::dfShowStatistics));
	return 1;
}
int LuaMapView::set_statistics(lua_State* L) {
	get()->set_display_flag(InteractiveBase::dfShowStatistics, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: is_building_road

      (RO) Is the player currently in road building mode?
*/
int LuaMapView::get_is_building_road(lua_State* L) {
	lua_pushboolean(L, get()->is_building_road());
	return 1;
}

/* RST
   .. attribute:: is_animating

		(RO) True if this MapView is currently paning or zooming.
*/
int LuaMapView::get_is_animating(lua_State* L) {
	lua_pushboolean(L, get()->is_animating());
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
	get()->mouse_to_field(
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->coords(), MapView::Transition::Jump);
	get()->fieldclicked();
	return 0;
}

/* RST
   .. method:: start_road_building(flag)

      Enters the road building mode as if the player has clicked
      the flag and chosen build road. It will also warp the mouse
      to the given starting node. Throws an error if we are already in road
      building mode.

      :arg flag: :class:`wl.map.Flag` object to start building from.
*/
// UNTESTED
int LuaMapView::start_road_building(lua_State* L) {
	InteractiveBase* me = get();
	if (me->is_building_road())
		report_error(L, "Already building road!");

	Widelands::Coords starting_field =
	   (*get_user_class<LuaMaps::LuaFlag>(L, 2))->get(L, get_egbase(L))->get_position();

	me->mouse_to_field(starting_field, MapView::Transition::Jump);
	me->start_build_road(starting_field, me->get_player()->player_number());

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
	if (me->is_building_road())
		me->abort_build_road();
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
	Widelands::Game& game = get_game(L);
	// don't move view in replays
	if (game.game_controller()->get_game_type() == GameController::GameType::REPLAY) {
		return 0;
	}

	const Vector2f center(luaL_checkdouble(L, 2), luaL_checkdouble(L, 3));
	get()->scroll_to_map_pixel(center, MapView::Transition::Smooth);
	return 0;
}

/* RST
   .. method:: scroll_to_map_pixel(field)

		Starts an animation to center the view on top of the 'field'. Use
		`is_animating` to check if the animation is still going on.

      :arg field: the field to center on
      :type field: :class:`wl.map.Field`
*/
int LuaMapView::scroll_to_field(lua_State* L) {
	get()->scroll_to_field(
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
	lua_pushboolean(
	   L, get()->view_area().contains((*get_user_class<LuaMaps::LuaField>(L, 2))->coords()));
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
	get()->mouse_to_pixel(Vector2i(x, y), MapView::Transition::Smooth);
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
	get()->mouse_to_field(
	   (*get_user_class<LuaMaps::LuaField>(L, 2))->coords(), MapView::Transition::Smooth);
	return 0;
}

/*
 * C Functions
 */

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

/* RST
.. function:: set_user_input_allowed(b)

   Allow or disallow user input. Be warned, setting this will make that
   mouse movements and keyboard presses are completely ignored. Only
   scripted stuff will still happen.

   :arg b: :const:`true` or :const:`false`
   :type b: :class:`boolean`
*/
static int L_set_user_input_allowed(lua_State* L) {
	UI::Panel::set_allow_user_input(luaL_checkboolean(L, -1));
	return 0;
}
/* RST
.. method:: get_user_input_allowed

   Return the current state of this flag.

   :returns: :const:`true` or :const:`false`
   :rtype: :class:`boolean`
*/
static int L_get_user_input_allowed(lua_State* L) {
	lua_pushboolean(L, UI::Panel::allow_user_input());
	return 1;
}

const static struct luaL_Reg wlui[] = {{"set_user_input_allowed", &L_set_user_input_allowed},
                                       {"get_user_input_allowed", &L_get_user_input_allowed},
                                       {nullptr, nullptr}};

void luaopen_wlui(lua_State* L) {
	lua_getglobal(L, "wl");   // S: wl_table
	lua_pushstring(L, "ui");  // S: wl_table "ui"
	luaL_newlib(L, wlui);     // S: wl_table "ui" wl.ui_table
	lua_settable(L, -3);      // S: wl_table
	lua_pop(L, 1);            // S:

	register_class<LuaPanel>(L, "ui");

	register_class<LuaButton>(L, "ui", true);
	add_parent<LuaButton, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTab>(L, "ui", true);
	add_parent<LuaTab, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWindow>(L, "ui", true);
	add_parent<LuaWindow, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMapView>(L, "ui", true);
	add_parent<LuaMapView, LuaPanel>(L);
	lua_pop(L, 1);  // Pop the meta table
}
}
