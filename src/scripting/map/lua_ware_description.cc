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

#include "scripting/map/lua_ware_description.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
WareDescription
---------------

.. class:: WareDescription

   A static description of a ware. See the parent class for more properties.
*/
const char LuaWareDescription::className[] = "WareDescription";
const MethodType<LuaWareDescription> LuaWareDescription::Methods[] = {
   METHOD(LuaWareDescription, consumers),
   METHOD(LuaWareDescription, is_construction_material),
   METHOD(LuaWareDescription, producers),
   {nullptr, nullptr},
};
const PropertyType<LuaWareDescription> LuaWareDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaWareDescription::__persist(lua_State* L) {
	const Widelands::WareDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaWareDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_ware_index(name);
	set_description_pointer(descriptions.get_ware_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. method:: consumers(tribename)

      (RO) Returns an :class:`array` with :class:`~wl.map.BuildingDescription` with buildings that
      need this ware for their production. Loads the tribe if it hasn't been loaded yet.

      :arg tribename: The name of the tribe that this ware gets checked for.
      :type tribename: :class:`string`

*/
int LuaWareDescription::consumers(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::TribeDescr* tribe = get_tribe_descr(L, luaL_checkstring(L, 2));

	lua_newtable(L);
	int index = 1;
	for (const Widelands::DescriptionIndex& building_index : get()->consumers()) {
		if (tribe->has_building(building_index)) {
			lua_pushint32(L, index++);
			upcasted_map_object_descr_to_lua(
			   L, get_egbase(L).descriptions().get_building_descr(building_index));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
   .. method:: is_construction_material(tribename)

      (RO) Returns :const:`true` if this ware is used by the tribe's construction sites.

      :arg tribename: The name of the tribe that this ware gets checked for.
      :type tribename: :class:`string`

*/
int LuaWareDescription::is_construction_material(lua_State* L) {
	std::string tribename = luaL_checkstring(L, -1);
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	if (descriptions.tribe_exists(tribename)) {
		const Widelands::DescriptionIndex& ware_index = descriptions.safe_ware_index(get()->name());
		int tribeindex = descriptions.tribe_index(tribename);
		lua_pushboolean(
		   L, static_cast<int>(
		         descriptions.get_tribe_descr(tribeindex)->is_construction_material(ware_index)));
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

/* RST
   .. method:: producers(tribename)

      (RO) Returns an :class:`array` with :class:`BuildingDescription` with buildings that
      can produce this ware. Loads the tribe if it hasn't been loaded yet.

      :arg tribename: The name of the tribe that this ware gets checked for.
      :type tribename: :class:`string`

*/
int LuaWareDescription::producers(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::TribeDescr* tribe = get_tribe_descr(L, luaL_checkstring(L, 2));

	lua_newtable(L);
	int index = 1;
	for (const Widelands::DescriptionIndex& building_index : get()->producers()) {
		if (tribe->has_building(building_index)) {
			lua_pushint32(L, index++);
			upcasted_map_object_descr_to_lua(
			   L, get_egbase(L).descriptions().get_building_descr(building_index));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

}  // namespace LuaMaps
