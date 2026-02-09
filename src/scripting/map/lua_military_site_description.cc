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

#include "scripting/map/lua_military_site_description.h"

namespace LuaMaps {

/* RST
MilitarySiteDescription
-----------------------

.. class:: MilitarySiteDescription

   A static description of a tribe's militarysite.

   A militarysite can garrison and heal soldiers, and it will expand your territory.
   See the parent classes for more properties.
*/
const char LuaMilitarySiteDescription::className[] = "MilitarySiteDescription";
const MethodType<LuaMilitarySiteDescription> LuaMilitarySiteDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMilitarySiteDescription> LuaMilitarySiteDescription::Properties[] = {
   PROP_RO(LuaMilitarySiteDescription, heal_per_second),
   PROP_RO(LuaMilitarySiteDescription, max_number_of_soldiers),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: heal_per_second

      (RO) The number of health healed per second by the militarysite.
*/
int LuaMilitarySiteDescription::get_heal_per_second(lua_State* L) {
	lua_pushinteger(L, get()->get_heal_per_second());
	return 1;
}

/* RST
   .. attribute:: max_number_of_soldiers

      (RO) The number of soldiers that can be garrisoned at the militarysite.
*/
int LuaMilitarySiteDescription::get_max_number_of_soldiers(lua_State* L) {
	lua_pushinteger(L, get()->get_max_number_of_soldiers());
	return 1;
}

}  // namespace LuaMaps
