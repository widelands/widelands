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

#include "scripting/map/lua_ferry_fleet_yard_interface.h"

#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
FerryFleetYardInterface
-----------------------

.. class:: FerryFleetYardInterface

   .. versionadded:: 1.2

   This represents an interface between a ferry yard and a ferry fleet.

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaFerryFleetYardInterface::className[] = "FerryFleetYardInterface";
const MethodType<LuaFerryFleetYardInterface> LuaFerryFleetYardInterface::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaFerryFleetYardInterface> LuaFerryFleetYardInterface::Properties[] = {
   PROP_RO(LuaFerryFleetYardInterface, building),
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
int LuaFerryFleetYardInterface::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

/* RST
   .. attribute:: building

      (RO) The ferry yard this interface belongs to.
*/
int LuaFerryFleetYardInterface::get_building(lua_State* L) {
	upcasted_map_object_to_lua(L, get(L, get_egbase(L))->get_building());
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
