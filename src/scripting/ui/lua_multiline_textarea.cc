/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#include "scripting/ui/lua_multiline_textarea.h"

namespace LuaUi {

/* RST
MultilineTextarea
-----------------

.. class:: MultilineTextarea

   .. versionadded:: 1.2

   A static text area with multiple lines that can not be modified by the user.
*/
const char LuaMultilineTextarea::className[] = "MultilineTextarea";
const MethodType<LuaMultilineTextarea> LuaMultilineTextarea::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMultilineTextarea> LuaMultilineTextarea::Properties[] = {
   PROP_RW(LuaMultilineTextarea, text),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */

/* RST
   .. attribute:: text

      (RW) The text currently shown by this area.
*/
int LuaMultilineTextarea::get_text(lua_State* L) {
	lua_pushstring(L, get()->get_text().c_str());
	return 1;
}
int LuaMultilineTextarea::set_text(lua_State* L) {
	get()->set_text(luaL_checkstring(L, -1));
	return 0;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

}  // namespace LuaUi
