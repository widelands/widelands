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

#include "scripting/map/lua_naval_invasion_base.h"

#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
NavalInvasionBase
-----------------

.. class:: NavalInvasionBase

   .. versionadded:: 1.2

   This represents a naval invasion in progress.

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaNavalInvasionBase::className[] = "NavalInvasionBase";
const MethodType<LuaNavalInvasionBase> LuaNavalInvasionBase::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaNavalInvasionBase> LuaNavalInvasionBase::Properties[] = {
   PROP_RO(LuaNavalInvasionBase, owner),
   PROP_RO(LuaNavalInvasionBase, soldiers),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: owner

      (RO) The :class:`wl.game.Player` who owns this object.
*/
int LuaNavalInvasionBase::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

/* RST
   .. attribute:: soldiers

      (RO) An :class:`array` with every :class:`~wl.map.Soldier`
      currently stationed on this invasion base.
*/
int LuaNavalInvasionBase::get_soldiers(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const auto& soldiers = get(L, egbase)->get_soldiers();
	lua_newtable(L);
	uint32_t cidx = 1;
	for (const auto& ptr : soldiers) {
		lua_pushuint32(L, cidx++);
		upcasted_map_object_to_lua(L, ptr.get(egbase));
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
