/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#include "scripting/lua_textstyle.h"

#include "graphic/text_layout.h"

namespace LuaTextstyle {

/* RST
:mod:`textstyle`
=============

.. module:: textstyle
   :synopsis: Provides helper functions for text formatting using themeable styles

.. moduleauthor:: The Widelands development team

.. currentmodule:: textstyle

*/

/* RST
Module Functions
^^^^^^^^^^^^^^^^

*/

/* RST
.. function:: as_title(text)

   Format the given text as main document title.

   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_title(lua_State* L) {
	lua_pushstring(L, as_ingame_title(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: as_h1(text)

   Format the given text as level 1 heading.

   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_h1(lua_State* L) {
	lua_pushstring(L, as_ingame_heading_1(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: as_h2(text)

   Format the given text as level 2 heading.

   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_h2(lua_State* L) {
	lua_pushstring(L, as_ingame_heading_2(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: as_h3(text)

   Format the given text as level 3 heading.

   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_h3(lua_State* L) {
	lua_pushstring(L, as_ingame_heading_3(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: as_h4(text)

   Format the given text as level 4 heading.

   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_h4(lua_State* L) {
	lua_pushstring(L, as_ingame_heading_4(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: as_p(text)

   Format the given text as a paragraph.

   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_p(lua_State* L) {
	lua_pushstring(L, as_ingame_text(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: as_p_with_attr(attributes, text)

   Format the given text as a paragraph with the given attributes.

   :type attributes: class:`string`
   :arg attributes: attributes for the paragraph tag.
   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_p_with_attr(lua_State* L) {
	lua_pushstring(L, as_ingame_text(luaL_checkstring(L, 2), luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: open_p()

   Return richtext formatting to start a paragraph.

   :returns: The richtext formatting that starts a paragraph.
*/
static int L_open_p(lua_State* L) {
	lua_pushstring(L, open_ingame_text());
	return 1;
}

/* RST
.. function:: open_p_with_attr(attributes)

   Return richtext formatting to start a paragraph with the given attributes.

   :type attributes: class:`string`
   :arg attributes: attributes for the paragraph tag.

   :returns: The richtext formatting that starts the paragraph.
*/
static int L_open_p_with_attr(lua_State* L) {
	lua_pushstring(L, open_ingame_text(luaL_checkstring(L, 1)));
	return 1;
}

/* RST
.. function:: close_p()

   Return richtext formatting to close a paragraph.

   :returns: The richtext formatting that closes a paragraph.
*/
static int L_close_p(lua_State* L) {
	lua_pushstring(L, close_ingame_text());
	return 1;
}

const static struct luaL_Reg textstyle[] = {{"as_title", &L_as_title},
                                            {"as_h1", &L_as_h1},
                                            {"as_h2", &L_as_h2},
                                            {"as_h3", &L_as_h3},
                                            {"as_h4", &L_as_h4},
                                            {"as_p", &L_as_p},
                                            {"as_p_with_attr", &L_as_p_with_attr},
                                            {"open_p", &L_open_p},
                                            {"open_p_with_attr", &L_open_p_with_attr},
                                            {"close_p", &L_close_p},
                                            {nullptr, nullptr}};

void luaopen_textstyle(lua_State* L) {
	luaL_newlib(L, textstyle);
	lua_setglobal(L, "textstyle");
}

}  // namespace LuaTextstyle
