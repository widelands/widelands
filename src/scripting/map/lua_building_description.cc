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

#include "scripting/map/lua_building_description.h"

#include "logic/map_objects/descriptions.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
BuildingDescription
-------------------

.. class:: BuildingDescription

   A static description of a tribe's building.
   This class contains the properties that are common to all buildings.
   Further properties are implemented in the subclasses.
   See the parent classes for more properties.
*/
const char LuaBuildingDescription::className[] = "BuildingDescription";
const MethodType<LuaBuildingDescription> LuaBuildingDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaBuildingDescription> LuaBuildingDescription::Properties[] = {
   PROP_RO(LuaBuildingDescription, buildcost),
   PROP_RO(LuaBuildingDescription, buildable),
   PROP_RO(LuaBuildingDescription, conquers),
   PROP_RO(LuaBuildingDescription, destructible),
   PROP_RO(LuaBuildingDescription, enhanced),
   PROP_RO(LuaBuildingDescription, enhanced_from),
   PROP_RO(LuaBuildingDescription, enhancement_cost),
   PROP_RO(LuaBuildingDescription, enhancement),
   PROP_RO(LuaBuildingDescription, is_mine),
   PROP_RO(LuaBuildingDescription, is_port),
   PROP_RO(LuaBuildingDescription, size),
   PROP_RO(LuaBuildingDescription, returns_on_dismantle),
   PROP_RO(LuaBuildingDescription, enhancement_returns_on_dismantle),
   PROP_RO(LuaBuildingDescription, vision_range),
   PROP_RO(LuaBuildingDescription, workarea_radius),
   {nullptr, nullptr, nullptr},
};

void LuaBuildingDescription::__persist(lua_State* L) {
	const Widelands::BuildingDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaBuildingDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_building_index(name);
	set_description_pointer(descriptions.get_building_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: buildcost

      (RO) A :class:`table` of ``{ware=build_cost}`` for the building.
*/
int LuaBuildingDescription::get_buildcost(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->buildcost(), Widelands::MapObjectType::WARE);
}

/* RST
   .. attribute:: buildable

      (RO) :const:`true` if the building can be built.
*/
int LuaBuildingDescription::get_buildable(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_buildable()));
	return 1;
}

/* RST
   .. attribute:: conquers

      (RO) The conquer range of the building as an :class:`int`.
*/
int LuaBuildingDescription::get_conquers(lua_State* L) {
	lua_pushinteger(L, get()->get_conquers());
	return 1;
}

/* RST
   .. attribute:: destructible

      (RO) :const:`true` if the building is destructible.
*/
int LuaBuildingDescription::get_destructible(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_destructible()));
	return 1;
}

/* RST
   .. attribute:: enhanced

      (RO) :const:`true` if the building is enhanced from another building.
*/
int LuaBuildingDescription::get_enhanced(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_enhanced()));
	return 1;
}

/* RST
   .. attribute:: enhanced_from

      (RO) The :class:`~wl.map.BuildingDescription` that this was enhanced from, or :const:`nil`
      if this isn't an enhanced building.
*/
int LuaBuildingDescription::get_enhanced_from(lua_State* L) {
	if (get()->is_enhanced()) {
		const Widelands::DescriptionIndex& enhanced_from = get()->enhanced_from();
		Widelands::EditorGameBase& egbase = get_egbase(L);
		assert(egbase.descriptions().building_exists(enhanced_from));
		return upcasted_map_object_descr_to_lua(
		   L, egbase.descriptions().get_building_descr(enhanced_from));
	}
	lua_pushnil(L);
	return 0;
}

/* RST
   .. attribute:: enhancement_cost

      (RO) A :class:`table` of ``{warename=cost}`` for enhancing to this building type.
*/
int LuaBuildingDescription::get_enhancement_cost(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->enhancement_cost(), Widelands::MapObjectType::WARE);
}

/* RST
   .. attribute:: enhancement

      (RO) The :class:`~wl.map.BuildingDescription` that this building can enhance to.
*/
int LuaBuildingDescription::get_enhancement(lua_State* L) {
	const Widelands::DescriptionIndex enhancement = get()->enhancement();
	if (enhancement == Widelands::INVALID_INDEX) {
		return 0;
	}
	return upcasted_map_object_descr_to_lua(
	   L, get_egbase(L).descriptions().get_building_descr(enhancement));
}

/* RST
   .. attribute:: is_mine

      (RO) :const:`true` if the building is a mine.
*/
int LuaBuildingDescription::get_is_mine(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_ismine()));
	return 1;
}

/* RST
   .. attribute:: is_port

      (RO) :const:`true` if the building is a port.
*/
int LuaBuildingDescription::get_is_port(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->get_isport()));
	return 1;
}

/* RST
   .. attribute:: size

      (RO) The size of this building as a :class:`string`. Can be either of

      * :const:`"small"` -- Small sized buildings
      * :const:`"medium"` -- Medium sized buildings
      * :const:`"big"` -- Big sized buildings
*/
int LuaBuildingDescription::get_size(lua_State* L) {
	try {
		lua_pushstring(L, Widelands::BaseImmovable::size_to_string(get()->get_size()));
	} catch (std::exception&) {
		report_error(L, "Unknown size %i in LuaBuildingDescription::get_size: %s", get()->get_size(),
		             get()->name().c_str());
	}
	return 1;
}

/* RST
   .. attribute:: returns_on_dismantle

      (RO) A :class:`table` of ``{warename=amount}`` pairs returned upon dismantling.
*/
int LuaBuildingDescription::get_returns_on_dismantle(lua_State* L) {
	return wares_or_workers_map_to_lua(
	   L, get()->returns_on_dismantle(), Widelands::MapObjectType::WARE);
}

/* RST
   .. attribute:: enhancement_returns_on_dismantle

      (RO) A :class:`table` of ``{warename=amount}`` pairs returned upon dismantling an enhanced
      building.
*/
int LuaBuildingDescription::get_enhancement_returns_on_dismantle(lua_State* L) {
	return wares_or_workers_map_to_lua(
	   L, get()->enhancement_returns_on_dismantle(), Widelands::MapObjectType::WARE);
}

/* RST
   .. attribute:: vision_range

      (RO) The vision_range of the building as an :class:`integer`.
*/
int LuaBuildingDescription::get_vision_range(lua_State* L) {
	lua_pushinteger(L, get()->vision_range());
	return 1;
}

/* RST
   .. attribute:: workarea_radius

      (RO) The first workarea_radius of the building as an :class:`integer`,
      :const:`nil` in case bulding has no workareas.
*/
int LuaBuildingDescription::get_workarea_radius(lua_State* L) {
	const WorkareaInfo& workareaInfo = get()->workarea_info();
	if (!workareaInfo.empty()) {
		lua_pushinteger(L, workareaInfo.begin()->first);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

}  // namespace LuaMaps
