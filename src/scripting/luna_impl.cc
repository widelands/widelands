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

#include <string>

#include "log.h"

#include "c_utils.h"
#include "luna.h"

#include "luna_impl.h"

/*
 * =======================================
 * Private Functions
 * =======================================
 */
static void m_instantiate_new_lua_class(lua_State * L) {
	lua_getfield(L, -1, "module");
	std::string module = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "class");
	std::string klass = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	// get this classes instantiator
	lua_getglobal(L, "wl"); // table wl
	lua_getfield(L, -1, module.c_str()); // table wl module
	std::string instantiator = "__" + klass;
	lua_getfield(L, -1, instantiator.c_str()); // table wl module func

	// TODO: check if this is a function
	lua_call(L, 0, 1);
}

static LunaClass ** m_get_new_empty_user_data(lua_State * L) {
	m_instantiate_new_lua_class(L);

	lua_pushint32(L, 0); // table wl module lua_obj int
	lua_gettable(L, -2); // table wl module lua_obj obj

	LunaClass ** obj = static_cast<LunaClass ** >(lua_touserdata(L, - 1));
	lua_pop(L, 4);

	return obj;
}

/*
 * =======================================
 * Public Function
 * =======================================
 */

/*
 * Expects a table on the stack. Calls
 * the instantiator for this object and fills it with
 * information from the table via its __unpersist function
 */
int luna_restore_object(lua_State * L) {
	LunaClass ** obj = m_get_new_empty_user_data(L);

	(*obj)->__unpersist(L);
	(*obj)->__finish_unpersist(L);

	return 1;
}

