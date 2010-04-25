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

// TODO: check this whole file, a lot of documentation is missing

/* RST
:mod:`wl.ui`
=============

.. module:: wl.ui
   :synopsis: Provides access on user interface. Mainly for tutorials and
		debugging.

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.ui

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


// TODO: document me somehow
const char L_Button::className[] = "Button";
const MethodType<L_Button> L_Button::Methods[] = {
	METHOD(L_Button, press),
	METHOD(L_Button, release),
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

/* RST
	.. attribute:: name

		(RO) The name of this button
*/
int L_Button::get_name(lua_State * L) {
	lua_pushstring(L, get()->get_name());
	return 1;
}

/*
 * Lua Functions
 */
// TODO: docu
// UNTESTED
int L_Button::press(lua_State * L) {
	get()->handle_mousein(true);
	get()->handle_mousepress(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
// TODO: docu
// UNTESTED
int L_Button::release(lua_State * L) {
	get()->handle_mousein(true);
	get()->handle_mouserelease(SDL_BUTTON_LEFT, 1, 1);
	return 0;
}
// TODO: docu
int L_Button::click(lua_State * L) {
	press(L);
	release(L);
	return 0;
}

/*
 * C Functions
 */

// TODO: document me somehow
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

/* RST
	.. attribute:: name

		(RO) The name of this button
*/
// TODO: maybe this should go to panel
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


// TODO: document me somehow
const char L_MapView::className[] = "MapView";
const MethodType<L_MapView> L_MapView::Methods[] = {
	METHOD(L_MapView, click),
	METHOD(L_MapView, move_mouse_to),
	{0, 0},
};
const PropertyType<L_MapView> L_MapView::Properties[] = {
	{0, 0, 0},
};

L_MapView::L_MapView(lua_State * L) :
	L_Panel(get_egbase(L).get_ibase()) {
}


/*
 * Properties
 */

/*
 * Lua Functions
 */
int L_MapView::move_mouse_to(lua_State * L) {
	get()->warp_mouse_to_node((*get_user_class<L_Field>(L, 2))->coords());
	return 0;
}
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


