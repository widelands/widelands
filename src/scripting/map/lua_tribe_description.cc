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

#include "scripting/map/lua_tribe_description.h"

#include <cstring>

#include "logic/map_objects/tribes/ship.h"
#include "scripting/globals.h"
#include "scripting/map/lua_immovable_description.h"
#include "scripting/map/lua_ware_description.h"

namespace LuaMaps {

/* RST
TribeDescription
--------------------
.. class:: TribeDescription

   A static description of a tribe.
   This class contains information about which buildings, wares, workers etc. a tribe uses.
*/
const char LuaTribeDescription::className[] = "TribeDescription";
const MethodType<LuaTribeDescription> LuaTribeDescription::Methods[] = {
   METHOD(LuaTribeDescription, has_building),
   METHOD(LuaTribeDescription, has_ware),
   METHOD(LuaTribeDescription, has_worker),
   {nullptr, nullptr},
};
const PropertyType<LuaTribeDescription> LuaTribeDescription::Properties[] = {
   PROP_RO(LuaTribeDescription, buildings),
   PROP_RO(LuaTribeDescription, builder),
   PROP_RO(LuaTribeDescription, carriers),
   PROP_RO(LuaTribeDescription, carrier),
   PROP_RO(LuaTribeDescription, carrier2),
   PROP_RO(LuaTribeDescription, ferry),
   PROP_RO(LuaTribeDescription, descname),
   PROP_RO(LuaTribeDescription, geologist),
   PROP_RO(LuaTribeDescription, immovables),
   PROP_RO(LuaTribeDescription, resource_indicators),
   PROP_RO(LuaTribeDescription, name),
   PROP_RO(LuaTribeDescription, port),
   PROP_RO(LuaTribeDescription, ship),
   PROP_RO(LuaTribeDescription, soldier),
   PROP_RO(LuaTribeDescription, wares),
   PROP_RO(LuaTribeDescription, workers),
   PROP_RO(LuaTribeDescription, directory),
   PROP_RO(LuaTribeDescription, collectors_points_table),
   {nullptr, nullptr, nullptr},
};

void LuaTribeDescription::__persist(lua_State* L) {
	const Widelands::TribeDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaTribeDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	Widelands::Descriptions& d = *get_egbase(L).mutable_descriptions();
	if (!Widelands::tribe_exists(name, d.all_tribes())) {
		report_error(L, "Tribe '%s' does not exist", name.c_str());
	}
	set_description_pointer(d.get_tribe_descr(d.load_tribe(name)));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: buildings

      (RO) An :class:`array` of :class:`BuildingDescription` with all the buildings that the tribe
      can use, casted to their appropriate subclasses.
*/
int LuaTribeDescription::get_buildings(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (Widelands::DescriptionIndex building : tribe.buildings()) {
		lua_pushinteger(L, ++counter);
		upcasted_map_object_descr_to_lua(L, tribe.get_building_descr(building));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: builder

      (RO) The internal name of the builder type that this tribe uses as
      :class:`string`.
*/
int LuaTribeDescription::get_builder(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(get()->builder())->name());
	return 1;
}

/* RST
   .. attribute:: carriers

      .. versionadded:: 1.1

      (RO) An :class:`array` of the internal names of the carrier types that this tribe uses as
      :class:`string`.
*/

int LuaTribeDescription::get_carriers(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (Widelands::DescriptionIndex c : tribe.carriers()) {
		lua_pushinteger(L, ++counter);
		lua_pushstring(L, tribe.get_worker_descr(c)->name());
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: carrier

      .. deprecated:: 1.1 Use :attr:`carriers` instead.

      (RO) The internal name of the carrier type that this tribe uses as
      :class:`string`.
*/

int LuaTribeDescription::get_carrier(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(get()->carriers()[0])->name());
	return 1;
}

/* RST
   .. attribute:: carrier2

      .. deprecated:: 1.1 Use :attr:`carriers` instead.

      (RO) The internal name of the secondary carrier type that this tribe uses as
      :class:`string`.
*/

int LuaTribeDescription::get_carrier2(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(get()->carriers()[1])->name());
	return 1;
}

/* RST
   .. attribute:: ferry

      (RO) The internal name of the ferry type that this tribe uses as
      :class:`string`.
*/

int LuaTribeDescription::get_ferry(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(get()->ferry())->name());
	return 1;
}

/* RST
   .. attribute:: descname

      (RO) The localized name of the tribe as :class:`string`
*/

int LuaTribeDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: geologist

      (RO) The internal name of the geologist type that this tribe uses as
      :class:`string`.
*/

int LuaTribeDescription::get_geologist(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(get()->geologist())->name());
	return 1;
}

/* RST
   .. attribute:: immovables

      (RO) An :class:`array` of :class:`ImmovableDescription` with all the immovables that the
      tribe can use.
*/
int LuaTribeDescription::get_immovables(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (Widelands::DescriptionIndex immovable : tribe.immovables()) {
		lua_pushinteger(L, ++counter);
		to_lua<LuaImmovableDescription>(
		   L, new LuaImmovableDescription(tribe.get_immovable_descr(immovable)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: resource_indicators

      (RO) The :class:`table` ``resource_indicators`` as defined in the tribe's ``units.lua``.
      See `data/tribes/initializations/atlanteans/units.lua` for more information
      on the table structure.
*/
int LuaTribeDescription::get_resource_indicators(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	for (const auto& resilist : tribe.resource_indicators()) {
		lua_pushstring(L, resilist.first);
		lua_newtable(L);
		for (const auto& resi : resilist.second) {
			lua_pushinteger(L, resi.first);
			lua_pushstring(L, tribe.get_immovable_descr(resi.second)->name());
			lua_settable(L, -3);
		}
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: collectors_points_table

      (RO) The ``collectors_points_table`` as defined in the tribe's ``units.lua``.
      See `data/tribes/initializations/atlanteans/units.lua` for more information
      on the table structure.
*/
int LuaTribeDescription::get_collectors_points_table(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	int index = 0;
	for (const auto& pair : tribe.collectors_points_table()) {
		lua_pushinteger(L, ++index);
		lua_newtable(L);
		lua_pushstring(L, "ware");
		lua_pushstring(L, pair.first);
		lua_settable(L, -3);
		lua_pushstring(L, "points");
		lua_pushinteger(L, pair.second);
		lua_settable(L, -3);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: name

      (RO) The internal name of the tribe as :class:`string`.
*/

int LuaTribeDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: directory

      (RO) The path of the tribe's initialization scripts as :class:`string`.
*/

int LuaTribeDescription::get_directory(lua_State* L) {
	std::string path = get()->basic_info().script;
	path = path.substr(0, path.size() - strlen(FileSystem::fs_filename(path.c_str())));
	lua_pushstring(L, path.c_str());
	return 1;
}

/* RST
   .. attribute:: port

      (RO) The internal name of the port type that this tribe uses as :class:`string`.
*/

int LuaTribeDescription::get_port(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_building_descr(get()->port())->name());
	return 1;
}

/* RST
   .. attribute:: ship

      (RO) The internal name of the ship type that this tribe uses as :class:`string`.
*/

int LuaTribeDescription::get_ship(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_ship_descr(get()->ship())->name());
	return 1;
}

/* RST
   .. attribute:: soldier

      (RO) The internal name of the soldier type that this tribe uses as :class:`string`.
*/

int LuaTribeDescription::get_soldier(lua_State* L) {
	lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(get()->soldier())->name());
	return 1;
}

/* RST
   .. attribute:: wares

      (RO) An :class:`array` of :class:`WareDescription` with all the wares that the tribe can use.
*/
int LuaTribeDescription::get_wares(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (Widelands::DescriptionIndex ware : tribe.wares()) {
		lua_pushinteger(L, ++counter);
		to_lua<LuaWareDescription>(L, new LuaWareDescription(tribe.get_ware_descr(ware)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: workers

      (RO) an :class:`array` of :class:`WorkerDescription` with all the workers that the tribe
      can use, casted to their appropriate subclasses.
*/
int LuaTribeDescription::get_workers(lua_State* L) {
	const Widelands::TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (Widelands::DescriptionIndex worker : tribe.workers()) {
		lua_pushinteger(L, ++counter);
		upcasted_map_object_descr_to_lua(L, tribe.get_worker_descr(worker));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. method:: has_building(buildingname)

      Returns :const:`true` if **buildingname** is a building and the tribe can use it.

      :returns: :const:`true` or :const:`false`
      :rtype: :class:`bool`
*/
int LuaTribeDescription::has_building(lua_State* L) {
	const std::string buildingname = luaL_checkstring(L, 2);
	const Widelands::DescriptionIndex index =
	   get_egbase(L).descriptions().building_index(buildingname);
	lua_pushboolean(L, static_cast<int>(get()->has_building(index)));
	return 1;
}

/* RST
   .. method:: has_ware(warename)

      Returns :const:`true` if **warename** is a ware and the tribe uses it.

      :returns: :const:`true` or :const:`false`
      :rtype: :class:`bool`
*/
int LuaTribeDescription::has_ware(lua_State* L) {
	const std::string warename = luaL_checkstring(L, 2);
	const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(warename);
	lua_pushboolean(L, static_cast<int>(get()->has_ware(index)));
	return 1;
}

/* RST
   .. method:: has_worker(workername)

      Returns :const:`true` if **workername** is a worker and the tribe can use it.

      :returns: :const:`true` or :const:`false`
      :rtype: :class:`bool`
*/
int LuaTribeDescription::has_worker(lua_State* L) {
	const std::string workername = luaL_checkstring(L, 2);
	const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(workername);
	lua_pushboolean(L, static_cast<int>(get()->has_worker(index)));
	return 1;
}

}  // namespace LuaMaps
