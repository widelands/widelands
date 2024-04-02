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

#include "scripting/map/lua_player_slot.h"

#include "scripting/globals.h"
#include "scripting/map/lua_field.h"

namespace LuaMaps {

/* RST
PlayerSlot
----------

.. class:: PlayerSlot

   A player description as it is in the map. This contains information
   about the start position, the name of the player if this map is played
   as scenario and it's tribe. Note that these information can be different
   than the players actually valid in the game as in single player games,
   the player can choose most parameters freely.
*/
const char LuaPlayerSlot::className[] = "PlayerSlot";
const MethodType<LuaPlayerSlot> LuaPlayerSlot::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPlayerSlot> LuaPlayerSlot::Properties[] = {
   PROP_RW(LuaPlayerSlot, tribe_name),
   PROP_RW(LuaPlayerSlot, name),
   PROP_RW(LuaPlayerSlot, starting_field),
   {nullptr, nullptr, nullptr},
};

void LuaPlayerSlot::__persist(lua_State* L) {
	PERS_UINT32("player", player_number_);
}

void LuaPlayerSlot::__unpersist(lua_State* L) {
	UNPERS_UINT32("player", player_number_)
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: tribe_name

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) The name of the tribe suggested for this player in this map.
*/
int LuaPlayerSlot::get_tribe_name(lua_State* L) {  // NOLINT - can not be made const
	lua_pushstring(L, get_egbase(L).map().get_scenario_player_tribe(player_number_));
	return 1;
}
int LuaPlayerSlot::set_tribe_name(lua_State* L) {  // NOLINT - can not be made const
	get_egbase(L).mutable_map()->set_scenario_player_tribe(player_number_, luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: name

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) The name for this player as suggested in this map.
*/
int LuaPlayerSlot::get_name(lua_State* L) {  // NOLINT - can not be made const
	lua_pushstring(L, get_egbase(L).map().get_scenario_player_name(player_number_));
	return 1;
}
int LuaPlayerSlot::set_name(lua_State* L) {  // NOLINT - can not be made const
	get_egbase(L).mutable_map()->set_scenario_player_name(player_number_, luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: starting_field

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) The starting_field for this player as set in the map.
      Note that it is not guaranteed that the HQ of the player is on this
      field as scenarios and starting conditions are free to place the HQ
      wherever it want. This field is only centered when the game starts.
*/
int LuaPlayerSlot::get_starting_field(lua_State* L) {  // NOLINT - can not be made const
	to_lua<LuaField>(L, new LuaField(get_egbase(L).map().get_starting_pos(player_number_)));
	return 1;
}
int LuaPlayerSlot::set_starting_field(lua_State* L) {  // NOLINT - can not be made const
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, -1);
	get_egbase(L).mutable_map()->set_starting_pos(player_number_, c->coords());
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
