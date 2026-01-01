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

#include "scripting/map/lua_soldier.h"

#include "scripting/globals.h"

namespace LuaMaps {

/* RST
Soldier
-------

.. class:: Soldier

   All soldiers that are on the map are represented by this class.

   More properties are available through this object's
   :class:`SoldierDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaSoldier::className[] = "Soldier";
const MethodType<LuaSoldier> LuaSoldier::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaSoldier> LuaSoldier::Properties[] = {
   PROP_RO(LuaSoldier, attack_level),   PROP_RO(LuaSoldier, defense_level),
   PROP_RO(LuaSoldier, health_level),   PROP_RO(LuaSoldier, evade_level),
   PROP_RW(LuaSoldier, current_health), {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: attack_level

      (RO) The current attack level of this soldier
*/
// UNTESTED
int LuaSoldier::get_attack_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_attack_level());
	return 1;
}

/* RST
   .. attribute:: defense_level

      (RO) The current defense level of this soldier
*/
// UNTESTED
int LuaSoldier::get_defense_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_defense_level());
	return 1;
}

/* RST
   .. attribute:: health_level

      (RO) The current health level of this soldier
*/
// UNTESTED
int LuaSoldier::get_health_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_health_level());
	return 1;
}

/* RST
   .. attribute:: evade_level

      (RO) The current evade level of this soldier
*/
// UNTESTED
int LuaSoldier::get_evade_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_evade_level());
	return 1;
}

/* RST
   .. attribute:: current_health

      (RW) This soldier's current number of hitpoints left.
*/
int LuaSoldier::get_current_health(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_current_health());
	return 1;
}
int LuaSoldier::set_current_health(lua_State* L) {
	Widelands::Soldier& s = *get(L, get_egbase(L));

	const uint32_t ch = luaL_checkuint32(L, -1);
	if (ch == 0) {
		report_error(L, "Soldier.current_health must be greater than 0");
	}

	const uint32_t maxhealth =
	   s.descr().get_base_health() + s.get_health_level() * s.descr().get_health_incr_per_level();
	if (ch > maxhealth) {
		report_error(
		   L, "Soldier.current_health %u must not be greater than %u for %s with health level %u", ch,
		   maxhealth, s.descr().name().c_str(), s.get_health_level());
	}

	s.set_current_health(ch);
	return 0;
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
