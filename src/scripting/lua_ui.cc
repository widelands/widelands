/*
 * Copyright (C) 2006-2011 by the Widelands Development Team
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

#include "gamecontroller.h"
#include "logic/player.h"
#include "scripting/c_utils.h"
#include "scripting/lua_map.h"
#include "scripting/luna.h"
#include "upcast.h"
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
const char L_Panel::className[] = "Panel";
const PropertyType<L_Panel> L_Panel::Properties[] = {
	PROP_RO(L_Panel, buttons),
	PROP_RO(L_Panel, tabs),
	PROP_RO(L_Panel, windows),
	PROP_RW(L_Panel, mouse_position_x),
	PROP_RW(L_Panel, mouse_position_y),
	PROP_RW(L_Panel, position_x),
	PROP_RW(L_Panel, position_y),
	PROP_RW(L_Panel, width),
	PROP_RW(L_Panel, height),
	{nullptr, nullptr, nullptr},
};
const MethodType<L_Panel> L_Panel::Methods[] = {
	METHOD(L_Panel, get_descendant_position),
	{nullptr, nullptr},
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
static void _put_all_visible_buttons_into_table
	(lua_State * L, UI::Panel * g)
{
	if (not g) return;

	for (UI::Panel * f = g->get_first_child(); f; f = f->get_next_sibling())
	{
		_put_all_visible_buttons_into_table(L, f);

		if (upcast(UI::Button, b, f))
			if (b->is_visible()) {
				lua_pushstring(L, b->get_name());
				to_lua<L_Button>(L, new L_Button(b));
				lua_rawset(L, -3);
			}
	}
}
int L_Panel::get_buttons(lua_State * L) {
	assert(m_panel);

	lua_newtable(L);
	_put_all_visible_buttons_into_table(L, m_panel);

	return 1;
}

/* RST
	.. attribute:: tabs

		(RO) An :class:`array` of all visible tabs inside this Panel.
*/
static void _put_all_tabs_into_table
	(lua_State * L, UI::Panel * g)
{
	if (not g) return;

	for (UI::Panel * f = g->get_first_child(); f; f = f->get_next_sibling())
	{
		_put_all_tabs_into_table(L, f);

		if (upcast(UI::Tab_Panel, t, f))
			container_iterate_const(UI::Tab_Panel::TabList, t->tabs(), tab) {
				lua_pushstring(L, (*tab)->get_name());
				to_lua<L_Tab>(L, new L_Tab(*tab));
				lua_rawset(L, -3);
			}
	}
}
int L_Panel::get_tabs(lua_State * L) {
	assert(m_panel);

	lua_newtable(L);
	_put_all_tabs_into_table(L, m_panel);

	return 1;
}


/* RST
	.. attribute:: windows

		(RO) A :class:`array` of all currently open windows that are
			children of this Panel.
*/
static void _put_all_visible_windows_into_table
	(lua_State * L, UI::Panel * g)
{
	if (not g) return;

	for (UI::Panel * f = g->get_first_child(); f; f = f->get_next_sibling())
	{
		_put_all_visible_windows_into_table(L, f);

		if (upcast(UI::Window, win, f)) {
			lua_pushstring(L, win->get_name());
			to_lua<L_Window>(L, new L_Window(win));
			lua_rawset(L, -3);
		}
	}
}
int L_Panel::get_windows(lua_State * L) {
	assert(m_panel);

	lua_newtable(L);
	_put_all_visible_windows_into_table(L, m_panel);

	return 1;
}

/* RST
	.. attribute:: mouse_position_x, mouse_position_y

		(RW) The current mouse position relative to this Panels position
*/
int L_Panel::get_mouse_position_x(lua_State * L) {
	assert(m_panel);
	lua_pushint32(L, m_panel->get_mouse_position().x);
	return 1;
}
int L_Panel::set_mouse_position_x(lua_State * L) {
	assert(m_panel);
	Point p = m_panel->get_mouse_position();
	p.x = luaL_checkint32(L, -1);
	m_panel->set_mouse_pos(p);
	return 1;
}
int L_Panel::get_mouse_position_y(lua_State * L) {
	assert(m_panel);
	lua_pushint32(L, m_panel->get_mouse_position().y);
	return 1;
}
int L_Panel::set_mouse_position_y(lua_State * L) {
	assert(m_panel);
	Point p = m_panel->get_mouse_position();
	p.y = luaL_checkint32(L, -1);
	m_panel->set_mouse_pos(p);
	return 1;
}

/* RST
	.. attribute:: width, height

		(RW) The dimensions of this panel in pixels
*/
int L_Panel::get_width(lua_State * L) {
	assert(m_panel);
	lua_pushint32(L, m_panel->get_w());
	return 1;
}
int L_Panel::set_width(lua_State * L) {
	assert(m_panel);
	m_panel->set_size(luaL_checkint32(L, -1), m_panel->get_h());
	return 1;
}
int L_Panel::get_height(lua_State * L) {
	assert(m_panel);
	lua_pushint32(L, m_panel->get_h());
	return 1;
}
int L_Panel::set_height(lua_State * L) {
	assert(m_panel);
	m_panel->set_size(m_panel->get_w(), luaL_checkint32(L, -1));
	return 1;
}

/* RST
	.. attribute:: position_x, position_y

		(RO) The top left pixel of the our inner canvas relative to the
		parent's element inner canvas.
*/
int L_Panel::get_position_x(lua_State * L) {
	assert(m_panel);
	Point p = m_panel->to_parent(Point(0, 0));

	lua_pushint32(L, p.x);
	return 1;
}
int L_Panel::set_position_x(lua_State * L) {
	assert(m_panel);
	Point p(luaL_checkint32(L, -1) - m_panel->get_lborder(), m_panel->get_y());
	m_panel->set_pos(p);
	return 1;
}
int L_Panel::get_position_y(lua_State * L) {
	assert(m_panel);
	Point p = m_panel->to_parent(Point(0, 0));

	lua_pushint32(L, p.y);
	return 1;
}
int L_Panel::set_position_y(lua_State * L) {
	assert(m_panel);
	Point p(m_panel->get_x(), luaL_checkint32(L, -1) - m_panel->get_tborder());
	m_panel->set_pos(p);
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
int L_Panel::get_descendant_position(lua_State * L) {
	assert(m_panel);

	UI::Panel * cur = (*get_base_user_class<L_Panel>(L, 2))->m_panel;

	Point cp = Point(0, 0);
	while (cur && cur != m_panel) {
		cp += cur->to_parent(Point(0, 0));
		cur = cur->get_parent();
	}

	if (!cur)
		return report_error(L, "Widget is not a descendant!");

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
const char L_Button::className[] = "Button";
const MethodType<L_Button> L_Button::Methods[] = {
	METHOD(L_Button, press),
	METHOD(L_Button, click),
	{nullptr, nullptr},
};
const PropertyType<L_Button> L_Button::Properties[] = {
	PROP_RO(L_Button, name),
	{nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int L_Button::get_name(lua_State * L) {
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
int L_Button::press(lua_State * /* L */) {
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
/* RST
	.. method:: click

		Click this button just as if the user would have moused over and clicked
		it.
*/
int L_Button::click(lua_State * /* L */) {
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
const char L_Tab::className[] = "Tab";
const MethodType<L_Tab> L_Tab::Methods[] = {
	METHOD(L_Tab, click),
	{nullptr, nullptr},
};
const PropertyType<L_Tab> L_Tab::Properties[] = {
	PROP_RO(L_Tab, name),
	PROP_RO(L_Tab, active),
	{nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int L_Tab::get_name(lua_State * L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

/* RST
	.. attribute:: active

		(RO) Is this the currently active tab in this window?
*/
int L_Tab::get_active(lua_State * L) {
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
int L_Tab::click(lua_State * /* L */) {
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
const char L_Window::className[] = "Window";
const MethodType<L_Window> L_Window::Methods[] = {
	METHOD(L_Window, close),
	{nullptr, nullptr},
};
const PropertyType<L_Window> L_Window::Properties[] = {
	PROP_RO(L_Window, name),
	{nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

// Documented in parent Class
int L_Window::get_name(lua_State * L) {
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
int L_Window::close(lua_State * /* L */) {
	delete m_panel;
	m_panel = nullptr;
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
const char L_MapView::className[] = "MapView";
const MethodType<L_MapView> L_MapView::Methods[] = {
	METHOD(L_MapView, click),
	METHOD(L_MapView, start_road_building),
	METHOD(L_MapView, abort_road_building),
	METHOD(L_MapView, close),
	{nullptr, nullptr},
};
const PropertyType<L_MapView> L_MapView::Properties[] = {
	PROP_RW(L_MapView, viewpoint_x),
	PROP_RW(L_MapView, viewpoint_y),
	PROP_RW(L_MapView, buildhelp),
	PROP_RW(L_MapView, census),
	PROP_RW(L_MapView, statistics),
	PROP_RO(L_MapView, is_building_road),
	{nullptr, nullptr, nullptr},
};

L_MapView::L_MapView(lua_State * L) :
	L_Panel(get_egbase(L).get_ibase()) {
}

void L_MapView::__unpersist(lua_State* L)
{
	Widelands::Game & game = get_game(L);
	m_panel = game.get_ibase();
}


/*
 * Properties
 */
/* RST
	.. attribute:: viewpoint_x, viewpoint_y

		(RW) The current view position of this view. This defines the map position
		(in pixels) that the top left pixel of this map view currently sees. This
		can be used together with :attr:`wl.map.Field.viewpoint` to move the view
		to fields quickly.
*/
int L_MapView::get_viewpoint_x(lua_State * L) {
	lua_pushuint32(L, get()->get_viewpoint().x);
	return 1;
}
int L_MapView::set_viewpoint_x(lua_State * L) {
	Widelands::Game & game = get_game(L);
	// don't move view in replays
	if (game.gameController()->getGameDescription() == "replay") {
		return 0;
	}

	Map_View * mv = get();
	Point p = mv->get_viewpoint();
	p.x = luaL_checkuint32(L, -1);
	mv->set_viewpoint(p, true);
	return 0;
}
int L_MapView::get_viewpoint_y(lua_State * L) {
	lua_pushuint32(L, get()->get_viewpoint().y);
	return 1;
}
int L_MapView::set_viewpoint_y(lua_State * L) {
	Widelands::Game & game = get_game(L);
	// don't move view in replays
	if (game.gameController()->getGameDescription() == "replay") {
		return 0;
	}

	Map_View * mv = get();
	Point p = mv->get_viewpoint();
	p.y = luaL_checkuint32(L, -1);
	mv->set_viewpoint(p, true);
	return 0;
}

/* RST
	.. attribute:: buildhelp

		(RW) True if the buildhelp is show, false otherwise.
*/
int L_MapView::get_buildhelp(lua_State * L) {
	lua_pushboolean(L, get()->buildhelp());
	return 1;
}
int L_MapView::set_buildhelp(lua_State * L) {
	get()->show_buildhelp(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
	.. attribute:: census

		(RW) True if the census strings are shown on buildings, false otherwise
*/
int L_MapView::get_census(lua_State * L) {
	lua_pushboolean(L, get()->get_display_flag(Interactive_Base::dfShowCensus));
	return 1;
}
int L_MapView::set_census(lua_State * L) {
	get()->set_display_flag
		(Interactive_Base::dfShowCensus, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
	.. attribute:: statistics

		(RW) True if the statistics strings are shown on buildings, false
		otherwise
*/
int L_MapView::get_statistics(lua_State * L) {
	lua_pushboolean
		(L, get()->get_display_flag(Interactive_Base::dfShowStatistics));
	return 1;
}
int L_MapView::set_statistics(lua_State * L) {
	get()->set_display_flag
		(Interactive_Base::dfShowStatistics, luaL_checkboolean(L, -1));
	return 0;
}

/* RST
	.. attribute:: is_building_road

		(RO) Is the player currently in road building mode?
*/
int L_MapView::get_is_building_road(lua_State * L) {
	lua_pushboolean(L, get()->is_building_road());
	return 1;
}

/*
 * Lua Functions
 */

/* RST
	.. method:: click(field)

		Moves the mouse onto a field and clicks it just like the user would
		have.

		:arg field: the field to click on
		:type field: :class:`wl.map.Field`
*/
int L_MapView::click(lua_State * L) {
	get()->warp_mouse_to_node
		((*get_user_class<LuaMap::L_Field>(L, 2))->coords());
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
int L_MapView::start_road_building(lua_State * L) {
	Interactive_Base * me = get();
	if (me->is_building_road())
		return report_error(L, "Already building road!");

	Widelands::Coords starting_field =
		(*get_user_class<LuaMap::L_Flag>(L, 2))->get
			(L, get_egbase(L))->get_position();

	me->warp_mouse_to_node(starting_field);
	me->start_build_road(starting_field, me->get_player()->player_number());

	return 0;
}

/* RST
	.. method:: abort_road_building

		If the player is currently in road building mode, this will cancel it.
		If he wasn't, this will do nothing.
*/
// UNTESTED
int L_MapView::abort_road_building(lua_State * /* L */) {
	Interactive_Base * me = get();
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
int L_MapView::close(lua_State * /* l */) {
	get()->end_modal(0);
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
static int L_set_user_input_allowed(lua_State * L) {
	UI::Panel::set_allow_user_input(luaL_checkboolean(L, -1));
	return 0;
}
/* RST
.. method:: get_user_input_allowed

	Return the current state of this flag.

	:returns: :const:`true` or :const:`false`
	:rtype: :class:`boolean`
*/
static int L_get_user_input_allowed(lua_State * L) {
	lua_pushboolean(L, UI::Panel::allow_user_input());
	return 1;
}


const static struct luaL_Reg wlui [] = {
	{"set_user_input_allowed", &L_set_user_input_allowed},
	{"get_user_input_allowed", &L_get_user_input_allowed},
	{nullptr, nullptr}
};

void luaopen_wlui(lua_State * L) {
	lua_getglobal(L, "wl");  // S: wl_table
	lua_pushstring(L, "ui"); // S: wl_table "ui"
	luaL_newlib(L, wlui);  // S: wl_table "ui" wl.ui_table
	lua_settable(L, -3); // S: wl_table
	lua_pop(L, 1); // S:

	register_class<L_Panel>(L, "ui");

	register_class<L_Button>(L, "ui", true);
	add_parent<L_Button, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Tab>(L, "ui", true);
	add_parent<L_Tab, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Window>(L, "ui", true);
	add_parent<L_Window, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_MapView>(L, "ui", true);
	add_parent<L_MapView, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table
}


};
