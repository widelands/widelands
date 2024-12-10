/*
 * Copyright (C) 2006-2024 by the Widelands Development Team
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

#include "scripting/map/lua_terrain_description.h"

#include "logic/map_objects/descriptions.h"
#include "scripting/globals.h"
#include "scripting/map/lua_resource_description.h"

namespace LuaMaps {

/* RST
TerrainDescription
--------------------
.. class:: TerrainDescription

   A static description of a terrain.
*/
const char LuaTerrainDescription::className[] = "TerrainDescription";
const MethodType<LuaTerrainDescription> LuaTerrainDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTerrainDescription> LuaTerrainDescription::Properties[] = {
   PROP_RO(LuaTerrainDescription, name),
   PROP_RO(LuaTerrainDescription, descname),
   PROP_RO(LuaTerrainDescription, default_resource),
   PROP_RO(LuaTerrainDescription, default_resource_amount),
   PROP_RO(LuaTerrainDescription, fertility),
   PROP_RO(LuaTerrainDescription, humidity),
   PROP_RO(LuaTerrainDescription, representative_image),
   PROP_RO(LuaTerrainDescription, temperature),
   PROP_RO(LuaTerrainDescription, valid_resources),
   {nullptr, nullptr, nullptr},
};

void LuaTerrainDescription::__persist(lua_State* L) {
	const Widelands::TerrainDescription* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaTerrainDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& desc = get_egbase(L).descriptions();
	set_description_pointer(desc.get_terrain_descr(desc.safe_terrain_index(name)));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: name

      (RO) The internal name of this terrain as :class:`string`.
*/

int LuaTerrainDescription::get_name(lua_State* L) {  // NOLINT - can not be made const
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: descname

      (RO) The localized name of this terrain as :class:`string`.
*/

int LuaTerrainDescription::get_descname(lua_State* L) {  // NOLINT - can not be made const
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: default_resource

      (RO) The :class:`wl.map.ResourceDescription` for the default resource provided by this
      terrain, or :const:`nil` if the terrain has no default resource.
*/

int LuaTerrainDescription::get_default_resource(lua_State* L) {  // NOLINT - can not be made const
	Widelands::DescriptionIndex res_index = get()->get_default_resource();
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	if (res_index != Widelands::kNoResource && res_index < descriptions.nr_resources()) {
		to_lua<LuaMaps::LuaResourceDescription>(
		   L, new LuaMaps::LuaResourceDescription(descriptions.get_resource_descr(res_index)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: default_resource_amount

      (RO) The amount of the default resource provided by this terrain as :class:`integer`.
*/

int LuaTerrainDescription::get_default_resource_amount(  // NOLINT - can not be made const
   lua_State* L) {
	lua_pushinteger(L, get()->get_default_resource_amount());
	return 1;
}

/* RST
   .. attribute:: fertility

      (RO) The fertility value for this terrain.

      See also: :attr:`ImmovableDescription.terrain_affinity`
*/

int LuaTerrainDescription::get_fertility(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, get()->fertility());
	return 1;
}

/* RST
   .. attribute:: humidity

      (RO) The humidity value for this terrain.

      See also: :attr:`ImmovableDescription.terrain_affinity`
*/

int LuaTerrainDescription::get_humidity(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, get()->humidity());
	return 1;
}

/* RST
   .. attribute:: representative_image

      (RO) The file path to a representative image as :class:`string`.
*/
int LuaTerrainDescription::get_representative_image(  // NOLINT - can not be made const
   lua_State* L) {
	lua_pushstring(L, get()->texture_paths().front());
	return 1;
}

/* RST
   .. attribute:: temperature

      (RO) The temperature value for this terrain.

      See also: :attr:`~ImmovableDescription.terrain_affinity`
*/

int LuaTerrainDescription::get_temperature(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, get()->temperature());
	return 1;
}

/* RST
   .. attribute:: valid_resources

      (RO) A list of :class:`wl.map.ResourceDescription` with all valid resources for this
      terrain.
*/

int LuaTerrainDescription::get_valid_resources(lua_State* L) {  // NOLINT - can not be made const
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	lua_newtable(L);
	int index = 1;
	for (Widelands::DescriptionIndex res_index : get()->valid_resources()) {
		if (res_index != Widelands::kNoResource && res_index < descriptions.nr_resources()) {
			lua_pushint32(L, index++);
			to_lua<LuaMaps::LuaResourceDescription>(
			   L, new LuaMaps::LuaResourceDescription(descriptions.get_resource_descr(res_index)));
			lua_settable(L, -3);
		}
	}
	return 1;
}

/*
 ==========================================================
 METHODS
 ==========================================================
 */

}  // namespace LuaMaps
