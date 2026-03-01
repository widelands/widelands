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

#include "scripting/map/lua_warehouse_description.h"

namespace LuaMaps {

/* RST
WarehouseDescription
--------------------

.. class:: WarehouseDescription

   A static description of a tribe's warehouse. Note that headquarters are also warehouses.
   A warehouse keeps people, animals and wares. See the parent classes for more properties.
*/
const char LuaWarehouseDescription::className[] = "WarehouseDescription";
const MethodType<LuaWarehouseDescription> LuaWarehouseDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaWarehouseDescription> LuaWarehouseDescription::Properties[] = {
   PROP_RO(LuaWarehouseDescription, heal_per_second),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: heal_per_second

      (RO) The number of health healed per second by the warehouse.
*/
int LuaWarehouseDescription::get_heal_per_second(lua_State* L) {
	lua_pushinteger(L, get()->get_heal_per_second());
	return 1;
}

}  // namespace LuaMaps
