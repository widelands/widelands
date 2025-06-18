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

#include "scripting/map/lua_immovable_description.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/terrain_affinity.h"
#include "scripting/globals.h"
#include "scripting/map/lua_terrain_description.h"

namespace LuaMaps {

/* RST
ImmovableDescription
--------------------

.. class:: ImmovableDescription

   A static description of a :class:`base immovable <BaseImmovable>`. See also
   :class:`MapObjectDescription` for more properties.
*/
const char LuaImmovableDescription::className[] = "ImmovableDescription";
const MethodType<LuaImmovableDescription> LuaImmovableDescription::Methods[] = {
   METHOD(LuaImmovableDescription, probability_to_grow),
   {nullptr, nullptr},
};
const PropertyType<LuaImmovableDescription> LuaImmovableDescription::Properties[] = {
   PROP_RO(LuaImmovableDescription, species), PROP_RO(LuaImmovableDescription, buildcost),
   PROP_RO(LuaImmovableDescription, becomes), PROP_RO(LuaImmovableDescription, terrain_affinity),
   PROP_RO(LuaImmovableDescription, size),    {nullptr, nullptr, nullptr},
};

void LuaImmovableDescription::__persist(lua_State* L) {
	const Widelands::ImmovableDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaImmovableDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_immovable_index(name);
	set_description_pointer(descriptions.get_immovable_descr(idx));
}

/* RST
   .. attribute:: species

         (RO) The localized species name of the immovable, or an empty string if it has none.
*/
int LuaImmovableDescription::get_species(lua_State* L) {
	lua_pushstring(L, get()->species());
	return 1;
}

/* RST
   .. attribute:: buildcost

      (RO) A :class:`table` of ``{ware=amount}`` pairs, describing the build cost for the
      immovable.
*/
int LuaImmovableDescription::get_buildcost(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->buildcost(), Widelands::MapObjectType::WARE);
}

/* RST
   .. attribute:: becomes

      (RO) An :class:`array` of map object names that this immovable can turn into, e.g.
      ``{"atlanteans_ship"}`` or ``{"deadtree2","fallentree"}``.
*/
int LuaImmovableDescription::get_becomes(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const auto& target : get()->becomes()) {
		lua_pushuint32(L, ++counter);
		lua_pushstring(L, target.second);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: terrain_affinity

      (RO) A :class:`table` containing numbers labeled as pickiness,
      preferred_fertility, preferred_humidity, and preferred_temperature,
      or :const:`nil` if the immovable has no terrain affinity.

      E.g. for a beech this will be:

      .. code-block:: lua

         {
            preferred_humidity = 400,
            preferred_temperature = 110,
            preferred_fertility = 600,
            pickiness = 60
         }
*/
int LuaImmovableDescription::get_terrain_affinity(lua_State* L) {
	if (get()->has_terrain_affinity()) {
		const Widelands::TerrainAffinity& affinity = get()->terrain_affinity();
		lua_newtable(L);
		lua_pushstring(L, "pickiness");
		lua_pushuint32(L, affinity.pickiness());
		lua_settable(L, -3);
		lua_pushstring(L, "preferred_fertility");
		lua_pushuint32(L, affinity.preferred_fertility());
		lua_settable(L, -3);
		lua_pushstring(L, "preferred_humidity");
		lua_pushuint32(L, affinity.preferred_humidity());
		lua_settable(L, -3);
		lua_pushstring(L, "preferred_temperature");
		lua_pushuint32(L, affinity.preferred_temperature());
		lua_settable(L, -3);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: size

      (RO) The size of this immovable. Can be either of

      * :const:`none` -- Example: mushrooms. Immovables will be destroyed when
         something else is built on this field.
      * :const:`small` -- Example: trees, flags or small sized buildings
      * :const:`medium` -- Example: Medium sized buildings
      * :const:`big` -- Example: Big sized buildings or rocks
*/
int LuaImmovableDescription::get_size(lua_State* L) {
	try {
		lua_pushstring(L, Widelands::BaseImmovable::size_to_string(get()->get_size()));
	} catch (std::exception&) {
		report_error(L, "Unknown size %i in LuaImmovableDescription::get_size: %s", get()->get_size(),
		             get()->name().c_str());
	}
	return 1;
}

/*
 ==========================================================
 METHODS
 ==========================================================
 */

/* RST
   .. method:: probability_to_grow(terrain_description)

      Returns a :class:`double` describing the probability that this immovable will grow on the
      given terrain. Returns :const:`nil` if this immovable has no terrain affinity.

      Note that floating-point arithmetic is platform-dependent. Using :class:`double`
      values to make any decisions in the script logic might result in desyncs.

      :arg terrain_description: The terrain that we are checking the probability for.
      :type terrain_description: :class:`wl.map.TerrainDescription`

*/
int LuaImmovableDescription::probability_to_grow(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	if (get()->has_terrain_affinity()) {
		const Widelands::TerrainDescription* terrain =
		   (*get_user_class<LuaMaps::LuaTerrainDescription>(L, 2))->get();
		lua_pushdouble(L, Widelands::probability_to_grow(get()->terrain_affinity(), *terrain) /
		                     static_cast<double>(Widelands::TerrainAffinity::kPrecisionFactor));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

}  // namespace LuaMaps
