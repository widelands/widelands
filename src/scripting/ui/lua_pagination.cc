/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#include "scripting/ui/lua_pagination.h"

namespace LuaUi {

/* RST
Pagination
----------

.. class:: Pagination

   .. versionadded:: 1.3

   This represents a pagination control that allows users to switch to different pages of
   a paginated UI structure and to change the pagesize.
*/
const char LuaPagination::className[] = "Pagination";
const MethodType<LuaPagination> LuaPagination::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPagination> LuaPagination::Properties[] = {
   PROP_RW(LuaPagination, current_page), PROP_RW(LuaPagination, pagesize),
   PROP_RO(LuaPagination, no_of_pages),  PROP_RO(LuaPagination, no_of_items),
   {nullptr, nullptr, nullptr},
};

/*
 * Properties
 */
/* RST
   .. attribute:: current_page

      (RW) The current page (in range from 1 to :attr:`no_of_pages`).
*/
int LuaPagination::get_current_page(lua_State* L) {
	lua_pushinteger(L, get()->get_current_page());
	return 1;
}
int LuaPagination::set_current_page(lua_State* L) {
	get()->set_page(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: pagesize

      (RW) The maximum number of items on a page.
*/
int LuaPagination::get_pagesize(lua_State* L) {
	lua_pushinteger(L, get()->get_pagesize());
	return 1;
}
int LuaPagination::set_pagesize(lua_State* L) {
	get()->set_pagesize(luaL_checkint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: no_of_pages

      (RO) The total number of pages with the current :attr:`pagesize` and number of items.
*/
int LuaPagination::get_no_of_pages(lua_State* L) {
	lua_pushinteger(L, get()->get_nr_pages());
	return 1;
}

/* RST
   .. attribute:: no_of_items

      (RO) The total number of items in the parental UI element.
*/
int LuaPagination::get_no_of_items(lua_State* L) {
	lua_pushinteger(L, get()->get_nr_items());
	return 1;
}

/*
 * Lua Functions
 */

/*
 * C Functions
 */

}  // namespace LuaUi
