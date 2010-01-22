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

#include "coroutine_impl.h"

extern "C" {
#include "pluto/pluto.h"
}

#include "c_utils.h"


int LuaCoroutine_Impl::resume(uint32_t * sleeptime)
{
	int rv = lua_resume(m_L, 0);
	int n = lua_gettop(m_L);
	uint32_t sleep_for = 0;
	if (n == 1)
		sleep_for = luaL_checkint32(m_L, -1);

	if (sleeptime)
		*sleeptime = sleep_for;

	if (rv != 0 && rv != YIELDED)
		return lua_error(m_L);

	return rv;
}


int write_func(lua_State *, const void * p, size_t data, void * ud) {
	Widelands::FileWrite * fw = static_cast<Widelands::FileWrite *>(ud);

	fw->Data(p, data, Widelands::FileWrite::Pos::Null());

	return data;
}
/*int read_func(lua_State* L, void* ud, size_t data, size_t * sz) {
	FileWrite* fw = static_cast<Widelands::FileWrite *>(ud);

	fw->Data(p, data);

	return data;
}*/

int LuaCoroutine_Impl::freeze(Widelands::FileWrite & fw) {
	log("Freezing!\n");
	lua_newtable(m_L);
	log("after new table!\n");
	lua_newtable(m_L);
        // lua_pushvalue(m_L, LUA_GLOBALSINDEX);
	log("after pushvalue!\n");

	// fw.Unsigned32(0xff);
	pluto_persist(m_L, &write_func, &fw);
	log("After pluto_persist!\n");

	return 1;
}

int LuaCoroutine_Impl::unfreeze(Widelands::FileRead & fr) {
	/*log("Unfreezing!\n");
	lua_createtable();
	pluto_unpersist(m_L, func, &fw)*/
}

