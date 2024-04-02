/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#include "scripting/map/lua_base_immovable.h"

#include "scripting/globals.h"
#include "scripting/map/lua_field.h"

namespace LuaMaps {

/* RST
BaseImmovable
-------------

.. class:: BaseImmovable

   This is the base class for all immovables in Widelands.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :attr:`MapObject.descr`.
*/
const char LuaBaseImmovable::className[] = "BaseImmovable";
const MethodType<LuaBaseImmovable> LuaBaseImmovable::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaBaseImmovable> LuaBaseImmovable::Properties[] = {
   PROP_RO(LuaBaseImmovable, fields),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: fields

      (RO) An :class:`array` of :class:`wl.map.Field` that is occupied by this
      Immovable. If the immovable occupies more than one field (roads or big
      buildings for example) the first entry in this list will be the main field.
*/
int LuaBaseImmovable::get_fields(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);

	Widelands::BaseImmovable::PositionList pl = get(L, egbase)->get_positions(egbase);

	lua_createtable(L, pl.size(), 0);
	uint32_t idx = 1;
	for (const Widelands::Coords& coords : pl) {
		lua_pushuint32(L, idx++);
		to_lua<LuaField>(L, new LuaField(coords.x, coords.y));
		lua_rawset(L, -3);
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

}  // namespace LuaMaps
