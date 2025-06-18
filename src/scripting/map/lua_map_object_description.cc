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

#include "scripting/map/lua_map_object_description.h"

namespace LuaMaps {

/* RST
MapObjectDescription
--------------------

.. class:: MapObjectDescription

   A static description of a map object, so it can be used without
   having to access an actual object on the map. This class contains the
   properties that are common to all objects a tribe has, such as buildings or
   wares. E.g. the tribal encyclopedia is built upon this class.

   To access the static descriptions of this class one can use anything that return
   description objects. See e.g. some of the attributes of :class:`~wl.Descriptions`
   or :class:`~wl.bases.EditorGameBase`.

   Accessing the descriptions of this class during a game is done via
   the class :class:`MapObject` and the attribute :attr:`MapObject.descr`.
*/

const char LuaMapObjectDescription::className[] = "MapObjectDescription";
const MethodType<LuaMapObjectDescription> LuaMapObjectDescription::Methods[] = {
   METHOD(LuaMapObjectDescription, helptexts),
   METHOD(LuaMapObjectDescription, has_attribute),
   {nullptr, nullptr},
};
const PropertyType<LuaMapObjectDescription> LuaMapObjectDescription::Properties[] = {
   PROP_RO(LuaMapObjectDescription, descname),
   PROP_RO(LuaMapObjectDescription, icon_name),
   PROP_RO(LuaMapObjectDescription, name),
   PROP_RO(LuaMapObjectDescription, type_name),
   {nullptr, nullptr, nullptr},
};

// Only base classes can be persisted.
void LuaMapObjectDescription::__persist(lua_State* /* L */) {
	NEVER_HERE();
}

void LuaMapObjectDescription::__unpersist(lua_State* /* L */) {
	NEVER_HERE();
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: descname

      (RO) The map object's localized name as :class:`string`
*/

int LuaMapObjectDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: icon_name

      (RO) The filename for the menu icon as :class:`string`
*/
int LuaMapObjectDescription::get_icon_name(lua_State* L) {
	lua_pushstring(L, get()->icon_filename());
	return 1;
}

/* RST
   .. attribute:: name

      (RO) The map object's internal name as :class:`string`
*/

int LuaMapObjectDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: type_name

      (RO) The map object's type as :class:`string`. Map object types are
      organized in a hierarchy, where an element that's lower in the
      hierarchy has all the properties of the higher-placed types,
      as well as its own additional properties. Any map object's
      description that isn't linked below can be accessed via its
      higher types, e.g. a ``bob`` is a
      :class:`general map object <MapObjectDescription>`, and a
      ``carrier`` is a :class:`worker <WorkerDescription>` as well as a
      general map object.
      Some types do not have any static properties besides those
      defined in their parent type's description class, and are therefore represented
      by their parent class. Possible values are:

      * **Bobs:** Bobs are map objects that can move around the map.
        Bob types are:

        * :const:`bob`, the abstract base type for all bobs. For properties see
          :class:`MapObjectDescription`.
        * :const:`critter`, animals that aren't controlled by any tribe. For properties see
          :class:`MapObjectDescription`.
        * :class:`ship <ShipDescription>`, a sea-going vessel
          belonging to a tribe that can ferry wares or an expedition.
        * :class:`worker <WorkerDescription>`, a worker belonging to
          a tribe.
        * :const:`carrier`, a specialized worker for carrying items along a road. For properties
          see :class:`WorkerDescription`.
        * :const:`ferry`, a specialized carrier for carrying items along a waterway. For properties
          see :class:`WorkerDescription`.
        * :class:`soldier <SoldierDescription>`, a specialized worker
          that will fight for its tribe.

      * **Wares:** :class:`ware <WareDescription>`, a ware used by
        buildings to produce other wares, workers or ships
      * **Immovables:** Immovables are map objects that have a fixed
        position on the map, like buildings or trees. Immovable types are:

        * :class:`immovable <ImmovableDescription>` General immovables
          that can belong to a tribe (e.g. a wheat field) or to the
          world (e.g. trees or rocks).

        * **Buildings:** Buildings always belong to a tribe. Building
          types are:

          * :class:`building <BuildingDescription>`, the base class
            for all buildings
          * :class:`constructionsite <ConstructionSiteDescription>`,
            an actual building is being constructed here,
          * :class:`dismantlesite <DismantleSiteDescription>`, an
            actual building is being dismantled here,
          * :class:`warehouse <WarehouseDescription>`, a warehouse
            can store wares and workers. Headquarters and ports are
            special types of warehouses, but they belong to the same
            class,
          * :class:`militarysite <MilitarySiteDescription>`, a
            building manned by soldiers to expand a tribe's territory,
          * :class:`productionsite <ProductionSiteDescription>`, the
            most common type of building, which can produce wares,
          * :class:`trainingsite <TrainingSiteDescription>`, a
            specialized productionsite for improving soldiers.

        * **Other Immovables:** Specialized immovables that aren't buildings.

          * :const:`flag`, a flag that can hold wares for transport. For properties see
            :class:`MapObjectDescription`.
          * :const:`roadbase`, the abstract base type for roads and waterways. For properties see
            :class:`MapObjectDescription`.
          * :const:`road`, a road connecting two flags. For properties see
            :class:`MapObjectDescription`.
          * :const:`waterway`, a waterway connecting two flags. For properties see
            :class:`MapObjectDescription`.
          * :const:`portdock`, a 'parking space' on water terrain where ships can
            load/unload wares and workers. A portdock is invisible to the player and one is
            automatically placed next to each port building. For properties see
            :class:`MapObjectDescription`.

      * **Abstract:** These types are abstract map objects that are used by the engine and are
        not visible on the map. They are mentioned here only for completeness; no Lua
        interface to access such objects or descriptions currently exists.

        * :const:`battle`, holds information about two soldiers in a fight,
        * :const:`naval_invasion_base`, links a naval invasion of a port space.
        * :const:`ship_fleet`, holds information for managing ships and ports,
        * :const:`ferry_fleet`, holds information for managing ferries and waterways.
        * :const:`ship_fleet_yard_interface`, links a shipyard to a ship fleet.
        * :const:`ferry_fleet_yard_interface`, links a ferry yard to a ferry fleet.
        * :const:`pinned_note`, a textual note pinned to a field by the player.

      Example to fetch some information from a tribe's description:

      .. code-block:: lua

         -- get tribe description
         local tribe_descr = wl.Game():get_tribe_description("barbarians")

         -- get building descriptions of this tribe
         local buildings = tribe_descr.buildings

         -- iterate over all building descriptions
         for i, building in ipairs(buildings) do
            print(building.type_name, building.name)

            -- filter military sites
            if building.type_name == "militarysite" do
               print(building.max_number_of_soldiers)
            end
         end
*/
int LuaMapObjectDescription::get_type_name(lua_State* L) {
	lua_pushstring(L, to_string(get()->type()));
	return 1;
}

/*
 ==========================================================
 METHODS
 ==========================================================
 */

/* RST
   .. method:: has_attribute(attribute_name)

      .. versionchanged:: 1.3
         Only available for :class:`ImmovableDescription` and :class:`CritterDescr` in 1.2.1 and older.

      Returns :const:`true` if the immovable has the attribute, :const:`false` otherwise.

      :arg attribute_name: The attribute that we are checking for.
      :type attribute_name: :class:`string`

*/
int LuaMapObjectDescription::has_attribute(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::MapObjectDescr::AttributeIndex attribute_id =
	   Widelands::MapObjectDescr::get_attribute_id(luaL_checkstring(L, 2));
	lua_pushboolean(L, static_cast<int>(get()->has_attribute(attribute_id)));
	return 1;
}

/* RST
   .. method:: helptexts(tribename)

      (RO) Returns a :class:`table` of helptexts if it exists for the given tribe, an
      empty :class:`table` otherwise.

      Keys are ``lore``, ``lore_author``, ``purpose``, ``note``, ``performance``, all of them
      optional. The :class:`table` may contain other keys as well.

      :arg tribename: The tribe for which we want to fetch the helptext.
      :type tribename: :class:`string`
*/
int LuaMapObjectDescription::helptexts(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	std::string tribename = luaL_checkstring(L, 2);
	lua_newtable(L);
	if (get()->has_helptext(tribename)) {
		for (const auto& item : get()->get_helptexts(tribename)) {
			if (!item.second.empty()) {
				lua_pushstring(L, item.first);
				lua_pushstring(L, item.second);
				lua_settable(L, -3);
			}
		}
	}
	return 1;
}

}  // namespace LuaMaps
