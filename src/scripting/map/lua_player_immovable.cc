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

#include "scripting/map/lua_player_immovable.h"

#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
PlayerImmovable
---------------

.. class:: PlayerImmovable

   All Immovables that belong to a Player (Buildings, Flags, ...) are based on
   this Class.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaPlayerImmovable::className[] = "PlayerImmovable";
const MethodType<LuaPlayerImmovable> LuaPlayerImmovable::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPlayerImmovable> LuaPlayerImmovable::Properties[] = {
   PROP_RO(LuaPlayerImmovable, owner),
   PROP_RO(LuaPlayerImmovable, debug_ware_economy),
   PROP_RO(LuaPlayerImmovable, debug_worker_economy),
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
int LuaPlayerImmovable::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

// UNTESTED, for debug only
int LuaPlayerImmovable::get_debug_ware_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(Widelands::wwWARE));
	return 1;
}
int LuaPlayerImmovable::get_debug_worker_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(Widelands::wwWORKER));
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
