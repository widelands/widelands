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

#include "scripting/map/lua_soldier_description.h"

namespace LuaMaps {

/* RST
SoldierDescription
--------------------

.. class:: SoldierDescription

   A static description of a tribe's soldier, so it can be used in help files
   without having to access an actual instance of the worker on the map.
   See the parent classes for more properties.
*/
const char LuaSoldierDescription::className[] = "SoldierDescription";
const MethodType<LuaSoldierDescription> LuaSoldierDescription::Methods[] = {
   METHOD(LuaSoldierDescription, get_health_level_image_filepath),
   METHOD(LuaSoldierDescription, get_attack_level_image_filepath),
   METHOD(LuaSoldierDescription, get_defense_level_image_filepath),
   METHOD(LuaSoldierDescription, get_evade_level_image_filepath),
   {nullptr, nullptr},
};
const PropertyType<LuaSoldierDescription> LuaSoldierDescription::Properties[] = {
   PROP_RO(LuaSoldierDescription, max_health_level),
   PROP_RO(LuaSoldierDescription, max_attack_level),
   PROP_RO(LuaSoldierDescription, max_defense_level),
   PROP_RO(LuaSoldierDescription, max_evade_level),
   PROP_RO(LuaSoldierDescription, base_health),
   PROP_RO(LuaSoldierDescription, base_min_attack),
   PROP_RO(LuaSoldierDescription, base_max_attack),
   PROP_RO(LuaSoldierDescription, base_defense),
   PROP_RO(LuaSoldierDescription, base_evade),
   PROP_RO(LuaSoldierDescription, health_incr_per_level),
   PROP_RO(LuaSoldierDescription, attack_incr_per_level),
   PROP_RO(LuaSoldierDescription, defense_incr_per_level),
   PROP_RO(LuaSoldierDescription, evade_incr_per_level),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: max_health_level

      (RO) The maximum health level that the soldier can have.
*/
int LuaSoldierDescription::get_max_health_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_health_level());
	return 1;
}

/* RST
   .. attribute:: max_attack_level

      (RO) The maximum attack level that the soldier can have.
*/
int LuaSoldierDescription::get_max_attack_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_attack_level());
	return 1;
}

/* RST
   .. attribute:: max_defense_level

      (RO) The maximum defense level that the soldier can have.
*/
int LuaSoldierDescription::get_max_defense_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_defense_level());
	return 1;
}

/* RST
   .. attribute:: max_evade_level

      (RO) The maximum evade level that the soldier can have.
*/
int LuaSoldierDescription::get_max_evade_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_evade_level());
	return 1;
}

/* RST
   .. attribute:: base_health

      (RO) The health points that the soldier starts with
*/
int LuaSoldierDescription::get_base_health(lua_State* L) {
	lua_pushinteger(L, get()->get_base_health());
	return 1;
}

/* RST
   .. attribute:: base_min_attack

      (RO) The minimum random attack points that get added to a soldier's attack
*/
int LuaSoldierDescription::get_base_min_attack(lua_State* L) {
	lua_pushinteger(L, get()->get_base_min_attack());
	return 1;
}

/* RST
   .. attribute:: base_max_attack

      (RO) The maximum random attack points that get added to a soldier's attack
*/
int LuaSoldierDescription::get_base_max_attack(lua_State* L) {
	lua_pushinteger(L, get()->get_base_max_attack());
	return 1;
}

/* RST
   .. attribute:: base_defense

      (RO) The defense % that the soldier starts with
*/
int LuaSoldierDescription::get_base_defense(lua_State* L) {
	lua_pushinteger(L, get()->get_base_defense());
	return 1;
}

/* RST
   .. attribute:: base_evade

      (RO) The evade % that the soldier starts with
*/
int LuaSoldierDescription::get_base_evade(lua_State* L) {
	lua_pushinteger(L, get()->get_base_evade());
	return 1;
}

/* RST
   .. attribute:: health_incr_per_level

      (RO) The health points that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_health_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_health_incr_per_level());
	return 1;
}

/* RST
   .. attribute:: attack_incr_per_level

      (RO) The attack points that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_attack_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_attack_incr_per_level());
	return 1;
}

/* RST
   .. attribute:: defense_incr_per_level

      (RO) The defense % that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_defense_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_defense_incr_per_level());
	return 1;
}

/* RST
   .. attribute:: evade_incr_per_level

      (RO) The evade % that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_evade_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_evade_incr_per_level());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: get_health_level_image_filepath(level)

      .. versionadded:: 1.3

      Get the filepath for this soldier's "health" attribute icon at the specified training level.

      :arg level: The soldier training attribute level whose icon to query
      :type level: :class:`integer`

      :returns: The image file path.
*/
int LuaSoldierDescription::get_health_level_image_filepath(lua_State* L) {
	lua_pushstring(L, get()->get_health_level_image_filepath(luaL_checkinteger(L, 2)).c_str());
	return 1;
}

/* RST
   .. method:: get_attack_level_image_filepath(level)

      .. versionadded:: 1.3

      Get the filepath for this soldier's "attack" attribute icon at the specified training level.

      :arg level: The soldier training attribute level whose icon to query
      :type level: :class:`integer`

      :returns: The image file path.
*/
int LuaSoldierDescription::get_attack_level_image_filepath(lua_State* L) {
	lua_pushstring(L, get()->get_attack_level_image_filepath(luaL_checkinteger(L, 2)).c_str());
	return 1;
}

/* RST
   .. method:: get_defense_level_image_filepath(level)

      .. versionadded:: 1.3

      Get the filepath for this soldier's "defense" attribute icon at the specified training level.

      :arg level: The soldier training attribute level whose icon to query
      :type level: :class:`integer`

      :returns: The image file path.
*/
int LuaSoldierDescription::get_defense_level_image_filepath(lua_State* L) {
	lua_pushstring(L, get()->get_defense_level_image_filepath(luaL_checkinteger(L, 2)).c_str());
	return 1;
}

/* RST
   .. method:: get_evade_level_image_filepath(level)

      .. versionadded:: 1.3

      Get the filepath for this soldier's "evade" attribute icon at the specified training level.

      :arg level: The soldier training attribute level whose icon to query
      :type level: :class:`integer`

      :returns: The image file path.
*/
int LuaSoldierDescription::get_evade_level_image_filepath(lua_State* L) {
	lua_pushstring(L, get()->get_evade_level_image_filepath(luaL_checkinteger(L, 2)).c_str());
	return 1;
}

}  // namespace LuaMaps
