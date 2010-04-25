/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <lua.hpp>

#include "upcast.h"
#include "wui/interactive_player.h"

#include "c_utils.h"
#include "lua_map.h"

#include "lua_ui.h"

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
 *                            MODULE CLASSES
 * ========================================================================
 */

/* RST
Panel
-----

.. class:: Panel

	The Panel is the most basic ui class. Each ui element is a panel.
*/
const char L_Panel::className[] = "Panel";
const MethodType<L_Panel> L_Panel::Methods[] = {
	{0, 0},
};
const PropertyType<L_Panel> L_Panel::Properties[] = {
	PROP_RO(L_Panel, buttons),
	PROP_RO(L_Panel, windows),
	PROP_RW(L_Panel, mouse_position_x),
	PROP_RW(L_Panel, mouse_position_y),
	PROP_RW(L_Panel, position_x),
	PROP_RW(L_Panel, position_y),
	PROP_RW(L_Panel, width),
	PROP_RW(L_Panel, height),
	{0, 0, 0},
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

	for(UI::Panel * f = g->get_first_child(); f; f = f->get_next_sibling())
	{
		_put_all_visible_buttons_into_table(L, f);

		if(upcast(UI::Button, b, f))
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
	.. attribute:: windows

		(RO) A :class:`array` of all currently open windows that are
			children of this Panel.
*/
static void _put_all_visible_windows_into_table
	(lua_State * L, UI::Panel * g)
{
	if (not g) return;

	for(UI::Panel * f = g->get_first_child(); f; f = f->get_next_sibling())
	{
		_put_all_visible_windows_into_table(L, f);

		if(upcast(UI::Window, win, f)) {
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
	Point p = m_panel->to_parent(Point(0,0));

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
	Point p = m_panel->to_parent(Point(0,0));

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
	{0, 0},
};
const PropertyType<L_Button> L_Button::Properties[] = {
	PROP_RO(L_Button, name),
	{0, 0, 0},
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
int L_Button::press(lua_State * L) {
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
/* RST
	.. method:: click

		Click this button just as if the user would have moused over and clicked
		it.
*/
int L_Button::click(lua_State * L) {
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	get()->handle_mouserelease(SDL_BUTTON_LEFT, 1, 1);
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
	{0, 0},
};
const PropertyType<L_Window> L_Window::Properties[] = {
	PROP_RO(L_Window, name),
	{0, 0, 0},
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
int L_Window::close(lua_State * L) {
	delete m_panel;
	m_panel = 0;
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
	{0, 0},
};
const PropertyType<L_MapView> L_MapView::Properties[] = {
	PROP_RW(L_MapView, viewpoint_x),
	PROP_RW(L_MapView, viewpoint_y),
	{0, 0, 0},
};

L_MapView::L_MapView(lua_State * L) :
	L_Panel(get_egbase(L).get_ibase()) {
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
	Map_View * mv = get();
	Point p = mv->get_viewpoint();
	p.x = luaL_checkuint32(L, -1);
	mv->set_viewpoint(p);
	return 0;
}
int L_MapView::get_viewpoint_y(lua_State * L) {
	lua_pushuint32(L, get()->get_viewpoint().y);
	return 1;
}
int L_MapView::set_viewpoint_y(lua_State * L) {
	Map_View * mv = get();
	Point p = mv->get_viewpoint();
	p.y = luaL_checkuint32(L, -1);
	mv->set_viewpoint(p);
	return 0;
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
	get()->warp_mouse_to_node((*get_user_class<L_Field>(L, 2))->coords());
	get()->fieldclicked.call();
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

const static struct luaL_reg wlui [] = {
	{0, 0}
};

void luaopen_wlui(lua_State * L) {
	luaL_register(L, "wl.ui", wlui);
	lua_pop(L, 1); // pop the table from the stack

	register_class<L_Panel>(L, "ui");

	register_class<L_Button>(L, "ui", true);
	add_parent<L_Button, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Window>(L, "ui", true);
	add_parent<L_Window, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_MapView>(L, "ui", true);
	add_parent<L_MapView, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table
}


