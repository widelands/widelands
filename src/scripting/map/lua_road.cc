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

#include "scripting/map/lua_road.h"

#include "economy/road.h"
#include "economy/waterway.h"
#include "logic/map_objects/tribes/carrier.h"
#include "scripting/globals.h"
#include "scripting/map/lua_flag.h"

namespace LuaMaps {

/* RST
Road
----

.. class:: Road

   A road connecting two flags in the economy of this Player.
   Waterways are currently treated like roads in scripts; however,
   there are significant differences. You can check whether an
   instance of Road is a road or waterway using :attr:`road_type`.

   See also: :ref:`has_workers`.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaRoad::className[] = "Road";
const MethodType<LuaRoad> LuaRoad::Methods[] = {
   METHOD(LuaRoad, get_workers),
   METHOD(LuaRoad, set_workers),
   {nullptr, nullptr},
};
const PropertyType<LuaRoad> LuaRoad::Properties[] = {
   PROP_RO(LuaRoad, length),        PROP_RO(LuaRoad, start_flag), PROP_RO(LuaRoad, end_flag),
   PROP_RO(LuaRoad, valid_workers), PROP_RO(LuaRoad, road_type),  {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: length

      (RO) The length of the roads in number of edges.
*/
int LuaRoad::get_length(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_path().get_nsteps());
	return 1;
}

/* RST
   .. attribute:: start_flag

      (RO) The flag were this road starts.
*/
int LuaRoad::get_start_flag(lua_State* L) {
	return to_lua<LuaFlag>(
	   L, new LuaFlag(get(L, get_egbase(L))->get_flag(Widelands::RoadBase::FlagStart)));
}

/* RST
   .. attribute:: end_flag

      (RO) The flag were this road ends.
*/
int LuaRoad::get_end_flag(lua_State* L) {
	return to_lua<LuaFlag>(
	   L, new LuaFlag(get(L, get_egbase(L))->get_flag(Widelands::RoadBase::FlagEnd)));
}

/* RST
   .. attribute:: road_type

      (RO) Type of road. Can be any either of:

      * ``"normal"``
      * ``"busy"``
      * ``"waterway"``
*/
int LuaRoad::get_road_type(lua_State* L) {
	Widelands::RoadBase* r = get(L, get_egbase(L));
	if (r->descr().type() == Widelands::MapObjectType::WATERWAY) {
		lua_pushstring(L, "waterway");
	} else if (upcast(Widelands::Road, road, r)) {
		lua_pushstring(L, road->is_busy() ? "busy" : "normal");
	} else {
		report_error(L, "Unknown road type! Please report as a bug!");
	}
	return 1;
}

// documented in parent class
int LuaRoad::get_valid_workers(lua_State* L) {
	Widelands::RoadBase* road = get(L, get_egbase(L));
	return workers_map_to_lua(L, get_valid_workers_for(*road));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaRoad::get_workers(lua_State* L) {
	Widelands::RoadBase* road = get(L, get_egbase(L));
	return do_get_workers(L, *road, get_valid_workers_for(*road));
}

int LuaRoad::set_workers(lua_State* L) {
	Widelands::RoadBase* road = get(L, get_egbase(L));
	return do_set_workers_for_road(L, road, get_valid_workers_for(*road));
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

bool LuaRoad::create_new_worker(lua_State* L,
                                Widelands::RoadBase& rb,
                                Widelands::EditorGameBase& egbase,
                                const Widelands::WorkerDescr* wdes) {
	Widelands::Road* r = dynamic_cast<Widelands::Road*>(&rb);
	const bool is_busy = (r != nullptr) && r->is_busy();
	if (is_busy) {
		// Busy roads have space for 2 carriers
		if (rb.get_workers().size() == 2) {
			return false;  // No space
		}
	} else if (!rb.get_workers().empty()) {
		// Normal roads and waterways have space for 1 carrier
		return false;  // No space
	}

	// Determine Idle position.
	Widelands::Flag& start = rb.get_flag(Widelands::RoadBase::FlagStart);
	Widelands::Coords idle_position = start.get_position();
	const Widelands::Path& path = rb.get_path();
	Widelands::Path::StepVector::size_type idle_index = rb.get_idle_index();
	for (Widelands::Path::StepVector::size_type i = 0; i < idle_index; ++i) {
		egbase.map().get_neighbour(idle_position, path[i], &idle_position);
	}

	// Ensure the position is free - e.g. we want carrier + carrier2 for busy road, not 2x carrier!
	for (Widelands::Worker* existing : rb.get_workers()) {
		if (existing->descr().name() == wdes->name()) {
			report_error(L, "Road already has worker <%s> assigned at (%d, %d)", wdes->name().c_str(),
			             idle_position.x, idle_position.y);
		}
	}

	Widelands::Carrier& carrier =
	   dynamic_cast<Widelands::Carrier&>(wdes->create(egbase, rb.get_owner(), &rb, idle_position));

	if (upcast(Widelands::Game, game, &egbase)) {
		carrier.start_task_road(*game);
	}

	rb.assign_carrier(carrier, 0);
	return true;
}

}  // namespace LuaMaps
