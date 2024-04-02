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

#include "scripting/map/lua_worker_description.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
WorkerDescription
-----------------

.. class:: WorkerDescription

   A static description of a tribe's worker. See the parent class for more properties.
*/
const char LuaWorkerDescription::className[] = "WorkerDescription";
const MethodType<LuaWorkerDescription> LuaWorkerDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaWorkerDescription> LuaWorkerDescription::Properties[] = {
   PROP_RO(LuaWorkerDescription, becomes),           PROP_RO(LuaWorkerDescription, buildcost),
   PROP_RO(LuaWorkerDescription, employers),         PROP_RO(LuaWorkerDescription, buildable),
   PROP_RO(LuaWorkerDescription, needed_experience), {nullptr, nullptr, nullptr},
};

void LuaWorkerDescription::__persist(lua_State* L) {
	const Widelands::WorkerDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaWorkerDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_worker_index(name);
	set_description_pointer(descriptions.get_worker_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: becomes

      (RO) The :class:`WorkerDescription` of the worker this one will level up
      to or :const:`nil` if it never levels up.
*/
int LuaWorkerDescription::get_becomes(lua_State* L) {
	const Widelands::DescriptionIndex becomes_index = get()->becomes();
	if (becomes_index == Widelands::INVALID_INDEX) {
		lua_pushnil(L);
		return 1;
	}
	return to_lua<LuaWorkerDescription>(
	   L, new LuaWorkerDescription(get_egbase(L).descriptions().get_worker_descr(becomes_index)));
}

/* RST
   .. attribute:: buildcost

      (RO) A list of building requirements, e.g. for an atlateans woodcutter this is
      ``{"atlanteans_carrier","saw"}``.
*/
int LuaWorkerDescription::get_buildcost(lua_State* L) {
	lua_newtable(L);
	if (get()->is_buildable()) {
		int index = 1;
		for (const auto& buildcost_pair : get()->buildcost()) {
			lua_pushint32(L, index++);
			lua_pushstring(L, buildcost_pair.first);
			lua_settable(L, -3);
		}
	}
	return 1;
}

/* RST
   .. attribute:: employers

      (RO) An :class:`array` with :class:`BuildingDescription` with buildings where
      this worker can be employed.
*/
int LuaWorkerDescription::get_employers(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const Widelands::DescriptionIndex& building_index : get()->employers()) {
		lua_pushint32(L, index++);
		upcasted_map_object_descr_to_lua(
		   L, get_egbase(L).descriptions().get_building_descr(building_index));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: buildable

      (RO) Returns :const:`true` if this worker is buildable.
*/
int LuaWorkerDescription::get_buildable(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->is_buildable()));
	return 1;
}

/* RST
   .. attribute:: needed_experience

      (RO) The experience the worker needs to reach this level.
*/
int LuaWorkerDescription::get_needed_experience(lua_State* L) {
	lua_pushinteger(L, get()->get_needed_experience());
	return 1;
}

}  // namespace LuaMaps
