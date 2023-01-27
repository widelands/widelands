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

#include "scripting/lua_styles.h"

#include "graphic/style_manager.h"

namespace LuaStyles {

/* RST
:mod:`styles`
=============

.. versionadded:: 1.2

.. module:: styles
   :synopsis: Provides helper functions for text formatting using themeable styles

.. moduleauthor:: The Widelands development team

.. currentmodule:: styles

*/

/* RST
Module Functions
^^^^^^^^^^^^^^^^

*/

/* RST
.. function:: as_font(style, text)

   Format the given text with the given font style.

   :type style: class:`string`
   :arg style: name of the font style to use.
   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_font(lua_State* L) {
	const std::string style(luaL_checkstring(L, 1));
	const std::string text(luaL_checkstring(L, 2));
	const std::string result = g_style_manager->font_style(style).as_font_tag(text);
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: as_paragraph(style, text)

   Format the given text as a paragraph with the given style.

   :type style: class:`string`
   :arg style: name of the paragraph style to use.
   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_paragraph(lua_State* L) {
	const std::string style(luaL_checkstring(L, 1));
	const std::string text(luaL_checkstring(L, 2));
	const std::string result = g_style_manager->paragraph_style(style).as_paragraph(text);
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: as_p_with_attr(style, attributes, text)

   Format the given text as a paragraph with the given style and extra attributes.

   :type style: class:`string`
   :arg style: name of the paragraph style to use.
   :type attributes: class:`string`
   :arg attributes: attributes for the paragraph tag.
   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_p_with_attr(lua_State* L) {
	const std::string style(luaL_checkstring(L, 1));
	const std::string attr(luaL_checkstring(L, 2));
	const std::string text(luaL_checkstring(L, 3));
	const std::string result = g_style_manager->paragraph_style(style).as_paragraph(text, attr);
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: open_p(style)

   Return richtext formatting to start a paragraph with the given style.

   :type style: class:`string`
   :arg style: name of the paragraph style to use.

   :returns: The richtext formatting that starts a paragraph.
*/
static int L_open_p(lua_State* L) {
	const std::string style(luaL_checkstring(L, 1));
	const std::string result = g_style_manager->paragraph_style(style).open_paragraph();
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: open_p_with_attr(style, attributes)

   Return richtext formatting to start a paragraph with the given style and extra attributes.

   :type style: class:`string`
   :arg style: name of the paragraph style to use.
   :type attributes: class:`string`
   :arg attributes: attributes for the paragraph tag.

   :returns: The richtext formatting that starts the paragraph.
*/
static int L_open_p_with_attr(lua_State* L) {
	const std::string style(luaL_checkstring(L, 1));
	const std::string attr(luaL_checkstring(L, 2));
	const std::string result = g_style_manager->paragraph_style(style).open_paragraph(attr);
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: close_p(style)

   Return richtext formatting to close a paragraph of the given style.

   :type style: class:`string`
   :arg style: name of the paragraph style to use.

   :returns: The richtext formatting that closes a paragraph.
*/
static int L_close_p(lua_State* L) {
	const std::string style(luaL_checkstring(L, 1));
	const std::string result = g_style_manager->paragraph_style(style).close_paragraph();
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: as_font_from_p(p_style, text)

   Format the given text with the font style of the given paragraph style, without the paragraph
   tags.

   :type style: class:`string`
   :arg p_style: name of the paragraph style to use.
   :type text: class:`string`
   :arg text: text to format.

   :returns: The text with richtext tags.
*/
static int L_as_font_from_p(lua_State* L) {
	const std::string p_style(luaL_checkstring(L, 1));
	const std::string text(luaL_checkstring(L, 2));
	const std::string result = g_style_manager->paragraph_style(p_style).font().as_font_tag(text);
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: color(style_name)

   Return the given color style in hex format suitable for font tags.

   :type style_name: class:`string`
   :arg style_name: name of the color style to use.

   :returns: The color as 6 characters hexadecimal RGB value.
*/
static int L_color(lua_State* L) {
	const std::string style_name(luaL_checkstring(L, 1));
	const std::string result = g_style_manager->color(style_name).hex_value();
	lua_pushstring(L, result.c_str());
	return 1;
}

/* RST
.. function:: get_size(style_name)

   Return the given styled size.

   :type style_name: class:`string`
   :arg style_name: name of the styled size to get.

   :returns: The integer value of the size, or 0 if no styled size is defined with the given
             `style_name`.
*/
static int L_get_size(lua_State* L) {
	const std::string style_name(luaL_checkstring(L, 1));
	const int result = g_style_manager->styled_size(style_name);
	lua_pushinteger(L, result);
	return 1;
}

const static struct luaL_Reg styles[] = {{"as_font", &L_as_font},
                                         {"as_paragraph", &L_as_paragraph},
                                         {"as_p_with_attr", &L_as_p_with_attr},
                                         {"open_p", &L_open_p},
                                         {"open_p_with_attr", &L_open_p_with_attr},
                                         {"close_p", &L_close_p},
                                         {"as_font_from_p", &L_as_font_from_p},
                                         {"color", &L_color},
                                         {"get_size", &L_get_size},
                                         {nullptr, nullptr}};

void luaopen_styles(lua_State* L) {
	luaL_newlib(L, styles);
	lua_setglobal(L, "styles");
}

}  // namespace LuaStyles
