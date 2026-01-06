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

#include "scripting/ui/lua_text_input_panel.h"

namespace LuaUi {

/* RST
TextInputPanel
--------------

.. class:: TextInputPanel

   .. versionadded:: 1.2

   This represents a box containing arbitrary text that can be edited by the user.
*/
const char LuaTextInputPanel::className[] = "TextInputPanel";
const MethodType<LuaTextInputPanel> LuaTextInputPanel::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTextInputPanel> LuaTextInputPanel::Properties[] = {
   PROP_RW(LuaTextInputPanel, text),      PROP_RO(LuaTextInputPanel, selected_text),
   PROP_RW(LuaTextInputPanel, password),  PROP_RW(LuaTextInputPanel, warning),
   PROP_RW(LuaTextInputPanel, caret_pos), PROP_RO(LuaTextInputPanel, multiline),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: text

      (RW) The text currently held in this box.
*/
int LuaTextInputPanel::get_text(lua_State* L) {
	lua_pushstring(L, get()->get_text().c_str());
	return 1;
}
int LuaTextInputPanel::set_text(lua_State* L) {
	get()->set_text(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: selected_text

      (RO) The text currently selected by the user (may be empty).
*/
int LuaTextInputPanel::get_selected_text(lua_State* L) {
	lua_pushstring(L, get()->has_selection() ? get()->get_selected_text().c_str() : "");
	return 1;
}

/* RST
   .. attribute:: password

      (RW) Whether the actual text is displayed with asterisks for password boxes.
*/
int LuaTextInputPanel::get_password(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_password()));
	return 1;
}
int LuaTextInputPanel::set_password(lua_State* L) {
	get()->set_password(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: warning

      (RW) Whether the box is highlighted to indicate an error.
*/
int LuaTextInputPanel::get_warning(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->has_warning()));
	return 1;
}
int LuaTextInputPanel::set_warning(lua_State* L) {
	get()->set_warning(luaL_checkboolean(L, -1));
	return 0;
}

/* RST
   .. attribute:: caret_pos

      (RW) The position of the caret in the text.
*/
int LuaTextInputPanel::get_caret_pos(lua_State* L) {
	lua_pushinteger(L, get()->get_caret_pos());
	return 1;
}
int LuaTextInputPanel::set_caret_pos(lua_State* L) {
	get()->set_caret_pos(luaL_checkuint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: multiline

      (RO) Whether this editbox has multiple lines or just one line.
*/
int LuaTextInputPanel::get_multiline(lua_State* L) {
	lua_pushboolean(L, dynamic_cast<const UI::MultilineEditbox*>(get()) != nullptr ? 1 : 0);
	return 1;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

}  // namespace LuaUi
