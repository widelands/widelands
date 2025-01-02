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

#include "scripting/map/lua_ship_description.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
ShipDescription
-----------------

.. class:: ShipDescription

   A static description of a tribe's ship. See also
   :class:`MapObjectDescription` for more properties.
*/
const char LuaShipDescription::className[] = "ShipDescription";
const MethodType<LuaShipDescription> LuaShipDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaShipDescription> LuaShipDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaShipDescription::__persist(lua_State* L) {
	const Widelands::ShipDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaShipDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_ship_index(name);
	set_description_pointer(descriptions.get_ship_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

}  // namespace LuaMaps
