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
// TODO: document me somehow
const char L_Panel::className[] = "Panel";
const MethodType<L_Panel> L_Panel::Methods[] = {
	{0, 0},
};
const PropertyType<L_Panel> L_Panel::Properties[] = {
	PROP_RO(L_Panel, buttons),
	{0, 0, 0},
};

/*
 * Properties
 */

/* RST
	.. attribute:: buttons

		(RO) An :class:`array` of all buttons in this  TODO
*/
static void _push_all_visible_buttons
	(lua_State * L, UI::Panel * g)
{
	if (not g)
		return ;

	for(UI::Panel * f = g->get_first_child(); f; f = f->get_next_sibling())
	{
		_push_all_visible_buttons(L, f);

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
	_push_all_visible_buttons(L, m_panel);

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
}
// TODO: docu
// UNTESTED
int L_Button::release(lua_State * L) {
	get()->handle_mousein(true);
	get()->handle_mouserelease(SDL_BUTTON_LEFT, 1, 1);
}
// TODO: docu
int L_Button::click(lua_State * L) {
	press(L);
	release(L);
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
.. function:: get

	Gets the most basic Panel that exists. This is the one that contains the
	main view and the build help button.

	:returns: the basic panel
	:rtype: class:`Panel`
*/
static int L_get(lua_State * L) {
	to_lua<L_Panel>(L, new L_Panel(get_game(L).get_ipl()));
	return 1;
}
const static struct luaL_reg wlui [] = {
	{"get", &L_get},
	{0, 0}
};

void luaopen_wlui(lua_State * L) {
	luaL_register(L, "wl.ui", wlui);
	lua_pop(L, 1); // pop the table from the stack

	register_class<L_Panel>(L, "ui");

	register_class<L_Button>(L, "ui", true);
	add_parent<L_Button, L_Panel>(L);
	lua_pop(L, 1); // Pop the meta table

}


