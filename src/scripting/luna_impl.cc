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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "scripting/luna_impl.h"

#include <string>

#include "log.h"
#include "scripting/c_utils.h"
#include "scripting/luna.h"


/*
 * =======================================
 * Private Functions
 * =======================================
 */
static void m_instantiate_new_lua_class(lua_State * L) {
	assert(lua_gettop(L) == 0);  // S:

	std::string module, klass;
	UNPERS_STRING("module", module);
	UNPERS_STRING("class", klass);

	// get this classes instantiator
	lua_getglobal(L, "wl"); //  S: wl
	if (module != "")
		lua_getfield(L, -1, module.c_str()); // S: wl module
	else
		lua_pushvalue(L, -1); // S: wl wl

	const std::string instantiator = "__" + klass;
	lua_getfield(L, -1, instantiator.c_str()); // S: wl module func

	// Hopefully this is a function!
	luaL_checktype(L, -1, LUA_TFUNCTION);

	lua_call(L, 0, 1);  // S: wl module luna_obj

	lua_remove(L, -2); // S: wl luna_obj
	lua_remove(L, -2); // S: luna_obj

	assert(lua_gettop(L) == 1);
}

static LunaClass** m_get_new_empty_user_data(lua_State * L) {
	assert(lua_gettop(L) == 0);  // S:

	m_instantiate_new_lua_class(L); // S: luna_obj

	lua_pushint32(L, 0); // luna_obj int
	lua_gettable(L, -2); // luna_obj userdata

	LunaClass** obj = static_cast<LunaClass**>(lua_touserdata(L, -1));
	lua_pop(L, 1); // luna_obj

	return obj;
}

/*
 * =======================================
 * Public Function
 * =======================================
 */

/*
 * This is the closure that is used to persist our own classes. It will be called
 * by eris while unpersisting. It will call the instantiator for this object
 * and fill it with information from the table (the upvalue for the closure)
 * via its __unpersist function.
 */
int luna_unpersisting_closure(lua_State * L) {
	assert(lua_gettop(L) == 0);

	LunaClass ** obj = m_get_new_empty_user_data(L); // S: luna_obj

	(*obj)->__unpersist(L);

	return 1;
}
