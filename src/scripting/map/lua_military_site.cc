/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#include "scripting/map/lua_military_site.h"

#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
MilitarySite
--------------

.. class:: MilitarySite

   Military buildings with stationed soldiers.

   See also: :ref:`has_soldiers`

   More properties are available through this object's
   :class:`MilitarySiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaMilitarySite::className[] = "MilitarySite";
const MethodType<LuaMilitarySite> LuaMilitarySite::Methods[] = {
   METHOD(LuaMilitarySite, get_soldiers),
   METHOD(LuaMilitarySite, set_soldiers),
   {nullptr, nullptr},
};
const PropertyType<LuaMilitarySite> LuaMilitarySite::Properties[] = {
   PROP_RO(LuaMilitarySite, max_soldiers),
   PROP_RW(LuaMilitarySite, soldier_preference),
   PROP_RW(LuaMilitarySite, capacity),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

// documented in parent class
int LuaMilitarySite::get_max_soldiers(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->max_soldier_capacity());
	return 1;
}

/* RST
   .. attribute:: capacity

      (RW) The number of soldiers meant to be stationed here.
*/
int LuaMilitarySite::set_capacity(lua_State* L) {
	get(L, get_egbase(L))->mutable_soldier_control()->set_soldier_capacity(luaL_checkuint32(L, -1));
	return 0;
}
int LuaMilitarySite::get_capacity(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/* RST
   .. attribute:: soldier_preference

      (RW) ``"heroes"`` if this site prefers heroes; ``"rookies"`` for rookies;
         or ``"any"`` for no predilection.
*/
int LuaMilitarySite::get_soldier_preference(lua_State* L) {
	lua_pushstring(
	   L, soldier_preference_to_string(get(L, get_egbase(L))->get_soldier_preference()).c_str());
	return 1;
}
int LuaMilitarySite::set_soldier_preference(lua_State* L) {
	try {
		get(L, get_egbase(L))
		   ->set_soldier_preference(string_to_soldier_preference(luaL_checkstring(L, -1)));
	} catch (const WException& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaMilitarySite::get_soldiers(lua_State* L) {
	Widelands::MilitarySite* ms = get(L, get_egbase(L));
	return do_get_soldiers(L, *ms->soldier_control(), ms->owner().tribe());
}

// documented in parent class
int LuaMilitarySite::set_soldiers(lua_State* L) {
	Widelands::MilitarySite* ms = get(L, get_egbase(L));
	return do_set_soldiers(L, ms->get_position(), ms->mutable_soldier_control(), ms->get_owner());
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
