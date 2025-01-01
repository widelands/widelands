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

#include "scripting/map/lua_building.h"

#include "economy/flag.h"
#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
Building
--------

.. class:: Building

   This represents a building owned by a player.

   More properties are available through this object's
   :class:`BuildingDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaBuilding::className[] = "Building";
const MethodType<LuaBuilding> LuaBuilding::Methods[] = {
   METHOD(LuaBuilding, dismantle),
   METHOD(LuaBuilding, enhance),
   {nullptr, nullptr},
};
const PropertyType<LuaBuilding> LuaBuilding::Properties[] = {
   PROP_RO(LuaBuilding, flag),
   PROP_RW(LuaBuilding, destruction_blocked),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: flag

      (RO) The flag that belongs to this building (that is to the bottom right
      of it's main location).
*/
// UNTESTED
int LuaBuilding::get_flag(lua_State* L) {
	return upcasted_map_object_to_lua(L, &get(L, get_egbase(L))->base_flag());
}

/* RST
   .. attribute:: destruction_blocked

      (RW) Whether the player is forbidden to dismantle or destroy this building.
*/
int LuaBuilding::get_destruction_blocked(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L))->is_destruction_blocked()));
	return 1;
}
int LuaBuilding::set_destruction_blocked(lua_State* L) {
	get(L, get_egbase(L))->set_destruction_blocked(luaL_checkboolean(L, -1));
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: dismantle([keep_wares = false])

      Instantly turn this building into a dismantlesite.

      :arg keep_wares: Optional: If :const:`false` (default) the wares in this buildings stock
         get destroyed. If :const:`true` the wares in this buildings stock will be preserved.
      :type keep_wares: :const:`bool`
*/
int LuaBuilding::dismantle(lua_State* L) {
	Widelands::Building* bld = get(L, get_egbase(L));
	bld->get_owner()->dismantle_building(bld, lua_gettop(L) > 1 && luaL_checkboolean(L, 2));
	return 0;
}

/* RST
   .. method:: enhance([keep_wares = false])

      .. versionadded:: 1.1

      Instantly enhance this building if there is an enhancement.

      :arg keep_wares: Optional: If :const:`false` (default) the wares in this buildings stock
         get destroyed. If :const:`true` the wares in this buildings stock will be preserved.
      :type keep_wares: :const:`bool`
*/
int LuaBuilding::enhance(lua_State* L) {
	Widelands::Building* bld = get(L, get_egbase(L));
	const Widelands::DescriptionIndex enhancement = bld->descr().enhancement();
	if (enhancement == Widelands::INVALID_INDEX) {
		return 0;
	}
	bld->get_owner()->enhance_building(
	   bld, enhancement, lua_gettop(L) > 1 && luaL_checkboolean(L, 2));
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
}  // namespace LuaMaps
