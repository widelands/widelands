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

#include "scripting/map/lua_bob.h"

#include "scripting/globals.h"
#include "scripting/map/lua_field.h"

namespace LuaMaps {

/* RST
Bob
---

.. class:: Bob

   This is the base class for all Bobs in widelands.

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaBob::className[] = "Bob";
const MethodType<LuaBob> LuaBob::Methods[] = {
   METHOD(LuaBob, has_caps),
   {nullptr, nullptr},
};
const PropertyType<LuaBob> LuaBob::Properties[] = {
   PROP_RO(LuaBob, field),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: field

      (RO) The field the bob is located on.
*/
// UNTESTED
int LuaBob::get_field(lua_State* L) {

	Widelands::EditorGameBase& egbase = get_egbase(L);

	Widelands::Coords coords = get(L, egbase)->get_position();

	return to_lua<LuaMaps::LuaField>(L, new LuaMaps::LuaField(coords.x, coords.y));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
   .. method:: has_caps(swim_or_walk)

      (RO) Whether this bob can swim or walk.

      :arg swim_or_walk: Can be either of :const:`"swims"` or :const:`"walks"`.
      :type swim_or_walk: :class:`string`

      :returns: :const:`true` if this bob is able to **swim_or_walk**, otherwise :const:`false`.
*/
// UNTESTED
int LuaBob::has_caps(lua_State* L) {
	std::string query = luaL_checkstring(L, 2);

	uint32_t movecaps = get(L, get_egbase(L))->descr().movecaps();

	if (query == "swims") {
		lua_pushboolean(L, movecaps & Widelands::MOVECAPS_SWIM);
	} else if (query == "walks") {
		lua_pushboolean(L, movecaps & Widelands::MOVECAPS_WALK);
	} else {
		report_error(L, "Unknown caps queried: %s!", query.c_str());
	}

	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
