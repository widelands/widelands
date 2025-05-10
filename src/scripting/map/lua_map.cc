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

#include "scripting/map/lua_map.h"

#include "economy/road.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/worker.h"
#include "scripting/globals.h"
#include "scripting/map/lua_field.h"
#include "scripting/map/lua_player_slot.h"

namespace LuaMaps {

/* RST
Map
---

.. class:: Map

   Access to the map and its objects. You cannot instantiate this directly,
   instead access it via ``wl.Game().map``.
*/
const char LuaMap::className[] = "Map";
const MethodType<LuaMap> LuaMap::Methods[] = {
   METHOD(LuaMap, count_conquerable_fields),
   METHOD(LuaMap, count_terrestrial_fields),
   METHOD(LuaMap, count_owned_valuable_fields),
   METHOD(LuaMap, place_immovable),
   METHOD(LuaMap, get_field),
   METHOD(LuaMap, wrap_field),
   METHOD(LuaMap, recalculate),
   METHOD(LuaMap, recalculate_seafaring),
   METHOD(LuaMap, set_port_space),
   METHOD(LuaMap, sea_route_exists),
   METHOD(LuaMap, find_ocean_fields),
   {nullptr, nullptr},
};
const PropertyType<LuaMap> LuaMap::Properties[] = {
   PROP_RO(LuaMap, allows_seafaring),
   PROP_RO(LuaMap, number_of_port_spaces),
   PROP_RO(LuaMap, port_spaces),
   PROP_RO(LuaMap, width),
   PROP_RO(LuaMap, height),
   PROP_RO(LuaMap, player_slots),
   PROP_RW(LuaMap, waterway_max_length),
   {nullptr, nullptr, nullptr},
};

void LuaMap::__persist(lua_State* /* L */) {
}

void LuaMap::__unpersist(lua_State* /* L */) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: allows_seafaring

      (RO) Whether the map currently allows seafaring.

      :returns: :const:`true` if there are at least two port spaces that can be reached from each
         other.
*/
int LuaMap::get_allows_seafaring(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get_egbase(L).map().allows_seafaring()));
	return 1;
}
/* RST
   .. attribute:: number_of_port_spaces

      (RO) The amount of port spaces on the map.

      :returns: An :class:`integer` with the number of port spaces.
*/
int LuaMap::get_number_of_port_spaces(lua_State* L) {
	lua_pushuint32(L, get_egbase(L).map().get_port_spaces().size());
	return 1;
}

/* RST
   .. attribute:: port_spaces

      (RO) A list of coordinates for all port spaces on the map.

      :returns: A :class:`table` of port space coordinates,
        like this: ``{{x=0,y=2},{x=54,y=23}}``.
*/
int LuaMap::get_port_spaces(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const Widelands::Coords& space : get_egbase(L).map().get_port_spaces()) {
		lua_pushinteger(L, ++counter);
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushint32(L, space.x);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushint32(L, space.y);
		lua_settable(L, -3);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: width

      (RO) The width of the map in fields.
*/
int LuaMap::get_width(lua_State* L) {
	lua_pushuint32(L, get_egbase(L).map().get_width());
	return 1;
}
/* RST
   .. attribute:: height

      (RO) The height of the map in fields.
*/
int LuaMap::get_height(lua_State* L) {
	lua_pushuint32(L, get_egbase(L).map().get_height());
	return 1;
}

/* RST
   .. attribute:: waterway_max_length

      .. versionadded:: 1.2

      (RW) The waterway length limit on this map.
*/
int LuaMap::get_waterway_max_length(lua_State* L) {
	lua_pushuint32(L, get_egbase(L).map().get_waterway_max_length());
	return 1;
}
int LuaMap::set_waterway_max_length(lua_State* L) {
	get_egbase(L).mutable_map()->set_waterway_max_length(luaL_checkuint32(L, -1));
	return 0;
}

/* RST
   .. attribute:: player_slots

      (RO) This is an :class:`array` that contains a :class:`~wl.map.PlayerSlot`
      for each player defined in the map.

      Use :attr:`wl.bases.PlayerBase.number` as index to get this player's description as suggested
      by the map's author.
*/
int LuaMap::get_player_slots(lua_State* L) {
	const Widelands::Map& map = get_egbase(L).map();

	lua_createtable(L, map.get_nrplayers(), 0);
	for (Widelands::PlayerNumber i = 0; i < map.get_nrplayers(); i++) {
		lua_pushuint32(L, i + 1);
		to_lua<LuaMaps::LuaPlayerSlot>(L, new LuaMaps::LuaPlayerSlot(i + 1));
		lua_settable(L, -3);
	}

	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: count_conquerable_fields()

      (RO) Counts all reachable fields that a player could build on.

      **Note:** The fields are only calculated afresh when this is called for the first time.

      :returns: An integer with the amount of fields.
*/
int LuaMap::count_conquerable_fields(lua_State* L) {
	lua_pushinteger(L, get_egbase(L).mutable_map()->count_all_conquerable_fields());
	return 1;
}

/* RST
   .. method:: count_terrestrial_fields()

      (RO) Counts all fields that are not swimmable.

      **Note:** The fields are only calculated afresh when this is called for the first time.

      :returns: An integer with the amount of fields.
*/
int LuaMap::count_terrestrial_fields(lua_State* L) {
	lua_pushinteger(
	   L, get_egbase(L).mutable_map()->count_all_fields_excluding_caps(Widelands::MOVECAPS_SWIM));
	return 1;
}

/* RST
   .. method:: count_owned_valuable_fields([immovable_attribute])

      (RO) Counts the number of owned valuable fields for all players.

      :arg immovable_attribute: Optional: If this is set, only count fields that have an
        immovable with the given attribute.
      :type immovable_attribute: :class:`string`

      :returns: A :class:`table` mapping player numbers to their number of owned fields.
*/
int LuaMap::count_owned_valuable_fields(lua_State* L) {
	if (lua_gettop(L) > 2) {
		report_error(L, "Does not take more than one argument.");
	}
	const std::string attribute = lua_gettop(L) == 2 ? luaL_checkstring(L, -1) : "";

	lua_newtable(L);
	for (const auto& fieldinfo : get_egbase(L).map().count_owned_valuable_fields(attribute)) {
		lua_pushinteger(L, fieldinfo.first);
		lua_pushinteger(L, fieldinfo.second);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. method:: find_ocean_fields(number)

      Returns an :class:`array` with the given number of Fields so that every field is swimmable,
      and from each field a sea route to any port space exists.

      :arg number: The number of fields to find.
      :type number: :class:`integer`

      :returns: :class:`array` of :class:`wl.map.Field`
*/
int LuaMap::find_ocean_fields(lua_State* L) {
	upcast(Widelands::Game, game, &get_egbase(L));
	assert(game);
	const Widelands::Map& map = game->map();

	std::vector<LuaMaps::LuaField*> result;
	for (uint32_t i = luaL_checkuint32(L, 2); i > 0;) {
		const uint32_t x = game->logic_rand() % map.get_width();
		const uint32_t y = game->logic_rand() % map.get_height();
		Widelands::Coords field(x, y);
		bool success = false;
		if ((map[field].maxcaps() & Widelands::MOVECAPS_SWIM) != 0) {
			for (Widelands::Coords port : map.get_port_spaces()) {
				for (const Widelands::Coords& c : map.find_portdock(port, false)) {
					Widelands::Path p;
					if (map.findpath(
					       field, c, 0, p, Widelands::CheckStepDefault(Widelands::MOVECAPS_SWIM)) >= 0) {
						success = true;
						break;
					}
				}
				if (success) {
					break;
				}
			}
		}
		if (success) {
			result.push_back(new LuaMaps::LuaField(x, y));
			--i;
		}
	}

	lua_newtable(L);
	int counter = 0;
	for (auto& f : result) {
		lua_pushinteger(L, ++counter);
		to_lua<LuaMaps::LuaField>(L, f);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. method:: place_immovable(name, field)

      Creates an immovable on a given field. If there is already an immovable on
      the field, an error is reported.

      :arg name: The name of the immovable to create
      :type name: :class:`string`
      :arg field: The immovable is created on this field.
      :type field: :class:`wl.map.Field`

      :returns: The created immovable.
*/
int LuaMap::place_immovable(lua_State* const L) {

	const std::string objname = luaL_checkstring(L, 2);
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, 3);

	// Check if the map is still free here
	if (Widelands::BaseImmovable const* const imm = c->fcoords(L).field->get_immovable()) {
		if (imm->get_size() >= Widelands::BaseImmovable::SMALL) {
			report_error(L, "Node is no longer free!");
		}
	}

	// Load it, place it, return it
	try {
		Widelands::EditorGameBase& egbase = get_egbase(L);
		Widelands::DescriptionIndex const imm_idx =
		   egbase.mutable_descriptions()->load_immovable(objname);
		Widelands::BaseImmovable* m =
		   &egbase.create_immovable(c->coords(), imm_idx, nullptr /* owner */);
		return LuaMaps::upcasted_map_object_to_lua(L, m);
	} catch (const Widelands::GameDataError&) {
		report_error(L, "Unknown immovable <%s>", objname.c_str());
	}
}

/* RST
   .. method:: get_field(x, y)

      Returns a :class:`wl.map.Field` object of the given coordinates.
      The coordinates must be in range from 0 (inclusive) to the map's width/height (exclusive).

      :see also: :meth:`wrap_field`
*/
int LuaMap::get_field(lua_State* L) {
	uint32_t x = luaL_checkuint32(L, 2);
	uint32_t y = luaL_checkuint32(L, 3);

	const Widelands::Map& map = get_egbase(L).map();

	if (x >= static_cast<uint32_t>(map.get_width())) {
		report_error(L, "x coordinate out of range!");
	}
	if (y >= static_cast<uint32_t>(map.get_height())) {
		report_error(L, "y coordinate out of range!");
	}

	return do_get_field(L, x, y);
}

/* RST
   .. method:: wrap_field(x, y)

      .. versionadded:: 1.2

      Returns a :class:`wl.map.Field` object of the given coordinates.
      If the coordinates are out of bounds, they will wrap around.

      :see also: :meth:`get_field`
*/
int LuaMap::wrap_field(lua_State* L) {
	int32_t x = luaL_checkint32(L, 2);
	int32_t y = luaL_checkint32(L, 3);
	Widelands::Coords c(x, y);

	const Widelands::Map& map = get_egbase(L).map();
	map.normalize_coords(c);

	return do_get_field(L, c.x, c.y);
}

/* RST
   .. method:: recalculate()

      This map recalculates the whole map state: Height of fields, buildcaps,
      whether the map allows seafaring and so on. You only need to call this
      function if you changed :attr:`~wl.map.Field.raw_height` in any way.
*/
// TODO(unknown): do we really want this function?
int LuaMap::recalculate(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	egbase.mutable_map()->recalc_whole_map(egbase);
	return 0;
}

/* RST
   .. method:: recalculate_seafaring()

      This method recalculates whether the map allows seafaring.
      You only need to call this function if you have been changing terrains to/from
      water and wanted to defer recalculating whether the map allows seafaring.
*/
int LuaMap::recalculate_seafaring(lua_State* L) {
	get_egbase(L).mutable_map()->recalculate_allows_seafaring();
	return 0;
}

/* RST
   .. method:: set_port_space(x, y, allowed)

      Sets whether a port space is allowed at the coordinates (x, y).
      Returns :const:`false` if the port space couldn't be set.

      :arg x: The x coordinate of the port space to set/unset.
      :type x: :class:`integer`
      :arg y: The y coordinate of the port space to set/unset.
      :type y: :class:`integer`
      :arg allowed: Whether building a port will be allowed here.
      :type allowed: :class:`bool`

      :returns: :const:`true` on success, or :const:`false` otherwise
      :rtype: :class:`bool`
*/
int LuaMap::set_port_space(lua_State* L) {
	const int x = luaL_checkint32(L, 2);
	const int y = luaL_checkint32(L, 3);
	const bool allowed = luaL_checkboolean(L, 4);
	const bool success = get_egbase(L).mutable_map()->set_port_space(
	   get_egbase(L), Widelands::Coords(x, y), allowed, false, true);
	lua_pushboolean(L, static_cast<int>(success));
	return 1;
}

/* RST
   .. method:: sea_route_exists(field, port)

      Returns whether a sea route exists from the given field to the given port space.

      :arg field: The field where to start
      :type field: :class:`wl.map.Field`
      :arg port: The port space to find
      :type port: :class:`wl.map.Field`

      :returns: :const:`true` if a sea route exists, or :const:`false` otherwise
      :rtype: :class:`bool`
*/
int LuaMap::sea_route_exists(lua_State* L) {
	const Widelands::Map& map = get_egbase(L).map();
	const Widelands::FCoords f_start = (*get_user_class<LuaMaps::LuaField>(L, 2))->fcoords(L);
	const Widelands::FCoords f_port = (*get_user_class<LuaMaps::LuaField>(L, 3))->fcoords(L);
	for (const Widelands::Coords& c : map.find_portdock(f_port, false)) {
		Widelands::Path p;
		if (map.findpath(f_start, c, 0, p, Widelands::CheckStepDefault(Widelands::MOVECAPS_SWIM)) >=
		    0) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	lua_pushboolean(L, 0);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

int LuaMap::do_get_field(lua_State* L, const uint32_t x, const uint32_t y) {
	constexpr const char* kAllFieldsName = "__all_field_instances";
	static const uint32_t kMaxSize = Widelands::kMapDimensions.back();

	lua_getglobal(L, kAllFieldsName);  // S: args... - allfields or nil
	if (lua_isnil(L, -1)) {
		lua_newtable(L);                   // S: args... - empty table
		lua_setglobal(L, kAllFieldsName);  // S: args...
		lua_getglobal(L, kAllFieldsName);  // S: args... - allfields
	}
	// S: args... - allfields

	// The index is 1 greater since Lua doesn't like the 0 index.
	const uint32_t index = x * kMaxSize + y + 1;
	lua_geti(L, -1, index);  // S: args... - allfields - field or nil

	if (lua_isnil(L, -1)) {
		// Create the field.
		lua_pop(L, 1);             // S: args... - allfields
		lua_pushuint32(L, index);  // S: args... - allfields - index
		to_lua<LuaMaps::LuaField>(L, new LuaMaps::LuaField(x, y));
		lua_settable(L, -3);     // S: args... - allfields
		lua_geti(L, -1, index);  // S: args... - allfields - field
	}

	lua_remove(L, -2);  // S: args... - field
	assert((*get_user_class<LuaMaps::LuaField>(L, -1))->coords() == Widelands::Coords(x, y));
	return 1;
}

}  // namespace LuaMaps
