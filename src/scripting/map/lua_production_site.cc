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

#include "scripting/map/lua_production_site.h"

#include "economy/input_queue.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

/* RST
ProductionSite
--------------

.. class:: ProductionSite

   Every building that produces anything.

   See also:
      * :ref:`has_inputs`
      * :ref:`has_workers`

   More properties are available through this object's
   :class:`ProductionSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaProductionSite::className[] = "ProductionSite";
const MethodType<LuaProductionSite> LuaProductionSite::Methods[] = {
   METHOD(LuaProductionSite, set_inputs),
   METHOD(LuaProductionSite, get_inputs),
   METHOD(LuaProductionSite, get_workers),
   METHOD(LuaProductionSite, set_workers),
   METHOD(LuaProductionSite, get_priority),
   METHOD(LuaProductionSite, set_priority),
   METHOD(LuaProductionSite, get_desired_fill),
   METHOD(LuaProductionSite, set_desired_fill),
   METHOD(LuaProductionSite, toggle_start_stop),

   {nullptr, nullptr},
};
const PropertyType<LuaProductionSite> LuaProductionSite::Properties[] = {
   PROP_RO(LuaProductionSite, valid_workers),
   PROP_RO(LuaProductionSite, valid_inputs),
   PROP_RO(LuaProductionSite, is_stopped),
   PROP_RO(LuaProductionSite, productivity),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// documented in parent class
int LuaProductionSite::get_valid_inputs(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::ProductionSite* ps = get(L, egbase);

	lua_newtable(L);
	for (const auto& input_ware : ps->descr().input_wares()) {
		const Widelands::WareDescr* descr = egbase.descriptions().get_ware_descr(input_ware.first);
		lua_pushstring(L, descr->name());
		lua_pushuint32(L, input_ware.second);
		lua_rawset(L, -3);
	}
	for (const auto& input_worker : ps->descr().input_workers()) {
		const Widelands::WorkerDescr* descr =
		   egbase.descriptions().get_worker_descr(input_worker.first);
		lua_pushstring(L, descr->name());
		lua_pushuint32(L, input_worker.second);
		lua_rawset(L, -3);
	}
	return 1;
}

// documented in parent class
int LuaProductionSite::get_valid_workers(lua_State* L) {
	Widelands::ProductionSite* ps = get(L, get_egbase(L));
	return workers_map_to_lua(L, get_valid_workers_for(*ps));
}

/* RST
   .. attribute:: is_stopped

      (RO) Returns whether this productionsite is currently active or stopped

      :returns: :const:`false` if the productionsite has been started,
         :const:`true` if it has been stopped.

*/
int LuaProductionSite::get_is_stopped(lua_State* L) {
	Widelands::ProductionSite* ps = get(L, get_egbase(L));
	lua_pushboolean(L, static_cast<int>(ps->is_stopped()));
	return 1;
}

/* RST
   .. attribute:: productivity

      (RO) Returns the building's current productivity percentage

      :returns: A number between 0 and 100.
*/
int LuaProductionSite::get_productivity(lua_State* L) {
	lua_pushinteger(L, get(L, get_egbase(L))->get_actual_statistics());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
// documented in parent class
int LuaProductionSite::set_inputs(lua_State* L) {
	Widelands::ProductionSite* ps = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = ps->owner().tribe();
	InputMap setpoints = parse_set_input_arguments(L, tribe);

	InputSet valid_inputs;
	for (const auto& input_ware : ps->descr().input_wares()) {
		valid_inputs.insert(std::make_pair(input_ware.first, Widelands::wwWARE));
	}
	for (const auto& input_worker : ps->descr().input_workers()) {
		valid_inputs.insert(std::make_pair(input_worker.first, Widelands::wwWORKER));
	}
	for (const auto& sp : setpoints) {
		if (valid_inputs.count(sp.first) == 0u) {
			report_error(L, "<%s> can't be stored in this building: %s!",
			             sp.first.second == Widelands::wwWARE ?
			                tribe.get_ware_descr(sp.first.first)->name().c_str() :
			                tribe.get_worker_descr(sp.first.first)->name().c_str(),
			             ps->descr().name().c_str());
		}
		Widelands::InputQueue& iq = ps->inputqueue(sp.first.first, sp.first.second, nullptr);
		if (sp.second > iq.get_max_size()) {
			report_error(
			   L, "Not enough space for %u inputs, only for %i", sp.second, iq.get_max_size());
		}
		iq.set_filled(sp.second);
	}

	return 0;
}

// documented in parent class
int LuaProductionSite::get_inputs(lua_State* L) {
	Widelands::ProductionSite* ps = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = ps->owner().tribe();

	bool return_number = false;
	InputSet input_set = parse_get_input_arguments(L, tribe, &return_number);

	InputSet valid_inputs;
	for (const auto& input_ware : ps->descr().input_wares()) {
		valid_inputs.insert(std::make_pair(input_ware.first, Widelands::wwWARE));
	}
	for (const auto& input_worker : ps->descr().input_workers()) {
		valid_inputs.insert(std::make_pair(input_worker.first, Widelands::wwWORKER));
	}

	if (input_set.size() == tribe.get_nrwares() + tribe.get_nrworkers()) {  // Want all returned
		input_set = valid_inputs;
	}

	if (!return_number) {
		lua_newtable(L);
	}

	for (const auto& input : input_set) {
		uint32_t cnt = 0;
		if (valid_inputs.count(input) != 0u) {
			cnt = ps->inputqueue(input.first, input.second, nullptr).get_filled();
		}

		if (return_number) {  // this is the only thing the customer wants to know
			lua_pushuint32(L, cnt);
			break;
		}
		lua_pushstring(L, input.second == Widelands::wwWARE ?
		                     tribe.get_ware_descr(input.first)->name() :
		                     tribe.get_worker_descr(input.first)->name());
		lua_pushuint32(L, cnt);
		lua_settable(L, -3);
	}
	return 1;
}

// documented in parent class
int LuaProductionSite::get_workers(lua_State* L) {
	Widelands::ProductionSite* ps = get(L, get_egbase(L));
	return do_get_workers(L, *ps, get_valid_workers_for(*ps));
}

// documented in parent class
int LuaProductionSite::set_workers(lua_State* L) {
	Widelands::ProductionSite* ps = get(L, get_egbase(L));
	return do_set_workers_for_productionsite(L, ps, get_valid_workers_for(*ps));
}

/* RST
   .. method:: toggle_start_stop()

      If :attr:`ProductionSite.is_stopped`, sends a command to start this productionsite.
      Otherwise, sends a command to stop this productionsite.
*/
int LuaProductionSite::toggle_start_stop(lua_State* L) {
	Widelands::Game& game = get_game(L);
	Widelands::ProductionSite* ps = get(L, game);
	game.send_player_start_stop_building(*ps);
	return 1;
}

// documented in parent class
int LuaProductionSite::get_priority(lua_State* L) {
	lua_pushstring(
	   L, priority_to_string(
	         get(L, get_egbase(L))
	            ->get_priority(Widelands::wwWARE, get_egbase(L).descriptions().safe_ware_index(
	                                                 luaL_checkstring(L, 2)))));
	return 1;
}
int LuaProductionSite::set_priority(lua_State* L) {
	try {
		get(L, get_egbase(L))
		   ->set_priority(Widelands::wwWARE,
		                  get_egbase(L).descriptions().safe_ware_index(luaL_checkstring(L, 2)),
		                  string_to_priority(luaL_checkstring(L, 3)));
	} catch (const WException& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}
int LuaProductionSite::get_desired_fill(lua_State* L) {
	const std::string item = luaL_checkstring(L, 2);
	const bool is_ware = get_egbase(L).descriptions().ware_exists(item);
	lua_pushuint32(L, get(L, get_egbase(L))
	                     ->inputqueue(is_ware ? get_egbase(L).descriptions().safe_ware_index(item) :
	                                            get_egbase(L).descriptions().safe_worker_index(item),
	                                  is_ware ? Widelands::wwWARE : Widelands::wwWORKER, nullptr)
	                     .get_max_fill());
	return 1;
}
int LuaProductionSite::set_desired_fill(lua_State* L) {
	const std::string item = luaL_checkstring(L, 2);
	const bool is_ware = get_egbase(L).descriptions().ware_exists(item);
	get(L, get_egbase(L))
	   ->inputqueue(is_ware ? get_egbase(L).descriptions().safe_ware_index(item) :
	                          get_egbase(L).descriptions().safe_worker_index(item),
	                is_ware ? Widelands::wwWARE : Widelands::wwWORKER, nullptr)
	   .set_max_fill(luaL_checkuint32(L, 3));
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

bool LuaProductionSite::create_new_worker(lua_State* /* L */,
                                          Widelands::ProductionSite& ps,
                                          Widelands::EditorGameBase& egbase,
                                          const Widelands::WorkerDescr* wdes) {
	return ps.warp_worker(egbase, *wdes);
}

}  // namespace LuaMaps
