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

#include "scripting/lua_map.h"

#include "economy/road.h"
#include "economy/waterway.h"
#include "scripting/globals.h"
#include "scripting/map/lua_base_immovable.h"
#include "scripting/map/lua_bob.h"
#include "scripting/map/lua_building.h"
#include "scripting/map/lua_building_description.h"
#include "scripting/map/lua_construction_site.h"
#include "scripting/map/lua_construction_site_description.h"
#include "scripting/map/lua_dismantle_site.h"
#include "scripting/map/lua_dismantle_site_description.h"
#include "scripting/map/lua_economy.h"
#include "scripting/map/lua_ferry_fleet_yard_interface.h"
#include "scripting/map/lua_field.h"
#include "scripting/map/lua_flag.h"
#include "scripting/map/lua_immovable_description.h"
#include "scripting/map/lua_map.h"
#include "scripting/map/lua_map_object.h"
#include "scripting/map/lua_map_object_description.h"
#include "scripting/map/lua_market.h"
#include "scripting/map/lua_market_description.h"
#include "scripting/map/lua_military_site.h"
#include "scripting/map/lua_military_site_description.h"
#include "scripting/map/lua_naval_invasion_base.h"
#include "scripting/map/lua_pinned_note.h"
#include "scripting/map/lua_player_immovable.h"
#include "scripting/map/lua_player_slot.h"
#include "scripting/map/lua_port_dock.h"
#include "scripting/map/lua_production_site.h"
#include "scripting/map/lua_production_site_description.h"
#include "scripting/map/lua_resource_description.h"
#include "scripting/map/lua_road.h"
#include "scripting/map/lua_ship.h"
#include "scripting/map/lua_ship_description.h"
#include "scripting/map/lua_ship_fleet_yard_interface.h"
#include "scripting/map/lua_soldier.h"
#include "scripting/map/lua_soldier_description.h"
#include "scripting/map/lua_terrain_description.h"
#include "scripting/map/lua_training_site.h"
#include "scripting/map/lua_training_site_description.h"
#include "scripting/map/lua_tribe_description.h"
#include "scripting/map/lua_ware_description.h"
#include "scripting/map/lua_warehouse.h"
#include "scripting/map/lua_warehouse_description.h"
#include "scripting/map/lua_worker.h"
#include "scripting/map/lua_worker_description.h"

namespace LuaMaps {

/* RST
:mod:`wl.map`
=============

.. module:: wl.map
   :synopsis: Provides access to Fields and Objects on the map

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.map

*/

// Checks if a field has the desired caps
bool check_has_caps(lua_State* L,
                    const std::string& query,
                    const Widelands::FCoords& f,
                    const Widelands::NodeCaps& caps,
                    const Widelands::Map& map) {
	if (query == "walkable") {
		return (caps & Widelands::MOVECAPS_WALK) != 0;
	}
	if (query == "swimmable") {
		return (caps & Widelands::MOVECAPS_SWIM) != 0;
	}
	if (query == "small") {
		return (caps & Widelands::BUILDCAPS_SMALL) != 0;
	}
	if (query == "medium") {
		return (caps & Widelands::BUILDCAPS_MEDIUM) != 0;
	}
	if (query == "big") {
		return (caps & Widelands::BUILDCAPS_BIG) == Widelands::BUILDCAPS_BIG;
	}
	if (query == "port") {
		return ((caps & Widelands::BUILDCAPS_PORT) != 0) && map.is_port_space(f);
	}
	if (query == "mine") {
		return (caps & Widelands::BUILDCAPS_MINE) != 0;
	}
	if (query == "flag") {
		return (caps & Widelands::BUILDCAPS_FLAG) != 0;
	}
	report_error(L, "Unknown caps queried: %s!", query.c_str());
}

// Pushes a lua table with (name, count) pairs for the given 'ware_amount_container' on the
// stack. The 'type' needs to be WARE or WORKER. Returns 1.
int wares_or_workers_map_to_lua(lua_State* L,
                                const Widelands::Buildcost& ware_amount_map,
                                Widelands::MapObjectType type) {
	lua_newtable(L);
	for (const auto& ware_amount : ware_amount_map) {
		switch (type) {
		case Widelands::MapObjectType::WORKER:
			lua_pushstring(
			   L, get_egbase(L).descriptions().get_worker_descr(ware_amount.first)->name());
			break;
		case Widelands::MapObjectType::WARE:
			lua_pushstring(L, get_egbase(L).descriptions().get_ware_descr(ware_amount.first)->name());
			break;
		default:
			throw wexception("wares_or_workers_map_to_lua needs a ware or worker");
		}
		lua_pushuint32(L, ware_amount.second);
		lua_settable(L, -3);
	}
	return 1;
}

bool SoldierMapDescr::operator<(const SoldierMapDescr& ot) const {
	bool equal_health = health == ot.health;
	bool equal_attack = attack == ot.attack;
	bool equal_defense = defense == ot.defense;
	if (equal_health && equal_attack && equal_defense) {
		return evade < ot.evade;
	}
	if (equal_health && equal_attack) {
		return defense < ot.defense;
	}
	if (equal_health) {
		return attack < ot.attack;
	}
	return health < ot.health;
}

bool SoldierMapDescr::operator==(const SoldierMapDescr& ot) const {
	return (health == ot.health && attack == ot.attack && defense == ot.defense &&
	        evade == ot.evade);
}

using SoldiersMap = std::map<SoldierMapDescr, Widelands::Quantity>;
using WaresWorkersMap = std::map<Widelands::DescriptionIndex, Widelands::Quantity>;
using SoldierAmount = std::pair<SoldierMapDescr, Widelands::Quantity>;
using WorkerAmount = std::pair<Widelands::DescriptionIndex, Widelands::Quantity>;
using PlrInfluence = std::pair<Widelands::PlayerNumber, Widelands::MilitaryInfluence>;
using WaresSet = std::set<Widelands::DescriptionIndex>;
using InputSet = std::set<std::pair<Widelands::DescriptionIndex, Widelands::WareWorker>>;
using WorkersSet = std::set<Widelands::DescriptionIndex>;
using SoldiersList = std::vector<Widelands::Soldier*>;

// Versions of the above macros which accept wares and workers
InputSet
parse_get_input_arguments(lua_State* L, const Widelands::TribeDescr& tribe, bool* return_number) {
	/* takes either "all", a name or an array of names */
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "Wrong number of arguments to get_inputs!");
	}
	*return_number = false;
	InputSet rv;
	if (lua_isstring(L, 2) != 0) {
		std::string what = luaL_checkstring(L, -1);
		if (what == "all") {
			for (const Widelands::DescriptionIndex& i : tribe.wares()) {
				rv.insert(std::make_pair(i, Widelands::wwWARE));
			}
			for (const Widelands::DescriptionIndex& i : tribe.workers()) {
				rv.insert(std::make_pair(i, Widelands::wwWORKER));
			}
		} else {
			/* Only one item requested */
			Widelands::DescriptionIndex index = tribe.ware_index(what);
			if (tribe.has_ware(index)) {
				rv.insert(std::make_pair(index, Widelands::wwWARE));
				*return_number = true;
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					rv.insert(std::make_pair(index, Widelands::wwWORKER));
					*return_number = true;
				} else {
					report_error(L, "Invalid input: <%s>", what.c_str());
				}
			}
		}
	} else {
		/* array of names */
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			std::string what = luaL_checkstring(L, -1);
			Widelands::DescriptionIndex index = tribe.ware_index(what);
			if (tribe.has_ware(index)) {
				rv.insert(std::make_pair(index, Widelands::wwWARE));
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					rv.insert(std::make_pair(index, Widelands::wwWORKER));
				} else {
					report_error(L, "Invalid input: <%s>", what.c_str());
				}
			}
			lua_pop(L, 1);
		}
	}
	return rv;
}

InputMap parse_set_input_arguments(lua_State* L, const Widelands::TribeDescr& tribe) {
	int32_t nargs = lua_gettop(L);
	if (nargs != 2 && nargs != 3) {
		report_error(L, "Wrong number of arguments to set_inputs!");
	}
	InputMap rv;
	if (nargs == 3) {
		/* name amount */
		std::string what = luaL_checkstring(L, 2);
		Widelands::DescriptionIndex index = tribe.ware_index(what);
		if (tribe.has_ware(index)) {
			rv.insert(
			   std::make_pair(std::make_pair(index, Widelands::wwWARE), luaL_checkuint32(L, 3)));
		} else {
			index = tribe.worker_index(what);
			if (tribe.has_worker(index)) {
				rv.insert(
				   std::make_pair(std::make_pair(index, Widelands::wwWORKER), luaL_checkuint32(L, 3)));
			} else {
				report_error(L, "Invalid input: <%s>", what.c_str());
			}
		}
	} else {
		/* array of (name, count) */
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			std::string what = luaL_checkstring(L, -2);
			Widelands::DescriptionIndex index = tribe.ware_index(what);
			if (tribe.has_ware(index)) {
				rv.insert(
				   std::make_pair(std::make_pair(index, Widelands::wwWARE), luaL_checkuint32(L, -1)));
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					rv.insert(std::make_pair(
					   std::make_pair(index, Widelands::wwWORKER), luaL_checkuint32(L, -1)));
				} else {
					report_error(L, "Invalid input: <%s>", what.c_str());
				}
			}
			lua_pop(L, 1);
		}
	}
	return rv;
}

WaresWorkersMap count_wares_on_flag_(Widelands::Flag& f,
                                     const Widelands::Descriptions& descriptions) {
	WaresWorkersMap rv;

	for (const Widelands::WareInstance* ware : f.get_wares()) {
		Widelands::DescriptionIndex i = descriptions.ware_index(ware->descr().name());
		if (rv.count(i) == 0u) {
			rv.insert(Widelands::WareAmount(i, 1));
		} else {
			++rv[i];
		}
	}
	return rv;
}

// Return the valid workers for a Road.
WaresWorkersMap get_valid_workers_for(const Widelands::RoadBase& r) {
	WaresWorkersMap valid_workers;
	if (r.descr().type() == Widelands::MapObjectType::WATERWAY) {
		valid_workers.insert(WorkerAmount(r.owner().tribe().ferry(), 1));
	} else {
		upcast(const Widelands::Road, road, &r);
		assert(road);
		if (road->is_busy()) {
			for (Widelands::DescriptionIndex c : r.owner().tribe().carriers()) {
				valid_workers.insert(WorkerAmount(c, 1));
			}
		} else {
			valid_workers.insert(WorkerAmount(r.owner().tribe().carriers()[0], 1));
		}
	}

	return valid_workers;
}

// Returns the valid workers allowed in 'pi'.
WaresWorkersMap get_valid_workers_for(const Widelands::ProductionSite& ps) {
	WaresWorkersMap rv;
	for (const auto& item : ps.descr().working_positions()) {
		rv.insert(WorkerAmount(item.first, item.second));
	}
	return rv;
}

// Translate the given Workers map into a (string, count) Lua table.
int workers_map_to_lua(lua_State* L, const WaresWorkersMap& valid_workers) {
	lua_newtable(L);
	for (const WaresWorkersMap::value_type& item : valid_workers) {
		lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(item.first)->name());
		lua_pushuint32(L, item.second);
		lua_rawset(L, -3);
	}
	return 1;
}

// Does most of the work of get_workers for player immovables (buildings and roads mainly).
int do_get_workers(lua_State* L,
                   const Widelands::PlayerImmovable& pi,
                   const WaresWorkersMap& valid_workers) {
	const Widelands::TribeDescr& tribe = pi.owner().tribe();

	Widelands::DescriptionIndex worker_index = Widelands::INVALID_INDEX;
	std::vector<Widelands::DescriptionIndex> workers_list;

	RequestedWareWorker parse_output =
	   parse_wares_workers_list(L, tribe, &worker_index, &workers_list, false);

	// c_workers is map (index:count) of all workers at the immovable
	WaresWorkersMap c_workers;
	for (const Widelands::Worker* w : pi.get_workers()) {
		Widelands::DescriptionIndex i = tribe.worker_index(w->descr().name());
		if (c_workers.count(i) == 0u) {
			c_workers.insert(WorkerAmount(i, 1));
		} else {
			++c_workers[i];
		}
	}

	// We return quantity for asked worker
	if (worker_index != Widelands::INVALID_INDEX) {
		if (c_workers.count(worker_index) != 0u) {
			lua_pushuint32(L, c_workers[worker_index]);
		} else {
			lua_pushuint32(L, 0);
		}
	} else {
		// or array of worker:quantity
		if (parse_output == RequestedWareWorker::kAll) {  // 'all' was required, otherwise we return
			                                               // only asked workers
			workers_list.clear();
			for (const WaresWorkersMap::value_type& v : valid_workers) {
				workers_list.push_back(v.first);
			}
		}
		lua_newtable(L);
		for (const Widelands::DescriptionIndex& i : workers_list) {
			Widelands::Quantity cnt = 0;
			if (c_workers.count(i) != 0u) {
				cnt = c_workers[i];
			}
			lua_pushstring(L, tribe.get_worker_descr(i)->name());
			lua_pushuint32(L, cnt);
			lua_settable(L, -3);
		}
	}
	return 1;
}

// Does most of the work of set_workers for player immovables (buildings and roads mainly).
template <typename TLua, typename TMapObject>
static int do_set_workers(lua_State* L, TMapObject* pi, const WaresWorkersMap& valid_workers) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::TribeDescr& tribe = pi->owner().tribe();

	// setpoints is map of index:quantity
	InputMap setpoints;
	parse_wares_workers_counted(
	   L, tribe, &setpoints, false, pi->descr().type() == Widelands::MapObjectType::WATERWAY);

	// c_workers is actual statistics, the map index:quantity
	WaresWorkersMap c_workers;
	for (const Widelands::Worker* w : pi->get_workers()) {
		Widelands::DescriptionIndex i = tribe.worker_index(w->descr().name());
		if (!valid_workers.count(i)) {
			// Ignore workers that will be consumed as inputs
			continue;
		}
		if (!c_workers.count(i)) {
			c_workers.insert(WorkerAmount(i, 1));
		} else {
			++c_workers[i];
		}
		if (!setpoints.count(std::make_pair(i, Widelands::WareWorker::wwWORKER))) {
			setpoints.insert(std::make_pair(std::make_pair(i, Widelands::WareWorker::wwWORKER), 0));
		}
	}

	// The idea is to change as little as possible
	for (const auto& sp : setpoints) {
		const Widelands::DescriptionIndex& index = sp.first.first;
		const Widelands::WorkerDescr* wdes = tribe.get_worker_descr(index);
		if (sp.second != 0 && !valid_workers.count(index)) {
			report_error(
			   L, "<%s> can't be employed at '%s'!", wdes->name().c_str(), pi->descr().name().c_str());
		}

		Widelands::Quantity cur = 0;
		WaresWorkersMap::iterator i = c_workers.find(index);
		if (i != c_workers.end()) {
			cur = i->second;
		}

		int d = sp.second - cur;
		if (d < 0) {
			while (d) {
				for (const Widelands::Worker* w : pi->get_workers()) {
					if (tribe.worker_index(w->descr().name()) == index) {
						const_cast<Widelands::Worker*>(w)->remove(egbase);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			for (; d; --d) {
				if (!TLua::create_new_worker(L, *pi, egbase, wdes)) {
					report_error(L, "No space left for worker '%s' at '%s'", wdes->name().c_str(),
					             pi->descr().name().c_str());
				}
			}
		}
	}
	return 0;
}

int do_set_workers_for_road(lua_State* L,
                            Widelands::RoadBase* pi,
                            const WaresWorkersMap& valid_workers) {
	return do_set_workers<LuaRoad>(L, pi, valid_workers);
}
int do_set_workers_for_productionsite(lua_State* L,
                                      Widelands::ProductionSite* pi,
                                      const WaresWorkersMap& valid_workers) {
	return do_set_workers<LuaProductionSite>(L, pi, valid_workers);
}

// Unpacks the Lua table of the form {health, attack, defense, evade} at the stack index
// 'table_index' into a SoldierMapDescr struct.
SoldierMapDescr
unbox_lua_soldier_description(lua_State* L, int table_index, const Widelands::SoldierDescr& sd) {
	SoldierMapDescr soldier_descr;

	lua_pushuint32(L, 1);
	lua_rawget(L, table_index);
	soldier_descr.health = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.health > sd.get_max_health_level()) {
		report_error(L, "health level (%u) > max health level (%u)",
		             static_cast<unsigned>(soldier_descr.health), sd.get_max_health_level());
	}

	lua_pushuint32(L, 2);
	lua_rawget(L, table_index);
	soldier_descr.attack = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.attack > sd.get_max_attack_level()) {
		report_error(L, "attack level (%u) > max attack level (%u)",
		             static_cast<unsigned>(soldier_descr.attack), sd.get_max_attack_level());
	}

	lua_pushuint32(L, 3);
	lua_rawget(L, table_index);
	soldier_descr.defense = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.defense > sd.get_max_defense_level()) {
		report_error(L, "defense level (%u) > max defense level (%u)",
		             static_cast<unsigned>(soldier_descr.defense), sd.get_max_defense_level());
	}

	lua_pushuint32(L, 4);
	lua_rawget(L, table_index);
	soldier_descr.evade = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.evade > sd.get_max_evade_level()) {
		report_error(L, "evade level (%u) > max evade level (%u)",
		             static_cast<unsigned>(soldier_descr.evade), sd.get_max_evade_level());
	}

	return soldier_descr;
}

// Parser the arguments of set_soldiers() into a setpoint. See the
// documentation in has_soldiers to understand the valid arguments.
SoldiersMap parse_set_soldiers_arguments(lua_State* L,
                                         const Widelands::SoldierDescr& soldier_descr) {
	SoldiersMap rv;
	constexpr Widelands::Quantity max_count = 10000;
	if (lua_gettop(L) > 2) {
		// STACK: cls, descr, count
		const Widelands::Quantity count = std::min(luaL_checkuint32(L, 3), max_count);
		const SoldierMapDescr d = unbox_lua_soldier_description(L, 2, soldier_descr);
		rv.insert(SoldierAmount(d, count));
	} else {
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			const SoldierMapDescr d = unbox_lua_soldier_description(L, 3, soldier_descr);
			const Widelands::Quantity count = std::min(luaL_checkuint32(L, -1), max_count);
			rv.insert(SoldierAmount(d, count));
			lua_pop(L, 1);
		}
	}
	return rv;
}

// Does most of the work of get_soldiers for buildings.
int do_get_soldiers(lua_State* L,
                    const Widelands::SoldierControl& sc,
                    const Widelands::TribeDescr& tribe) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Invalid arguments!");
	}

	if (lua_isstring(L, -1) != 0) {
		if (std::string(luaL_checkstring(L, -1)) == "present") {
			lua_pushuint32(L, sc.present_soldiers().size());
			return 1;
		}
		if (std::string(luaL_checkstring(L, -1)) == "stationed") {
			lua_pushuint32(L, sc.stationed_soldiers().size());
			return 1;
		}
		if (std::string(luaL_checkstring(L, -1)) == "associated") {
			lua_pushuint32(L, sc.associated_soldiers().size());
			return 1;
		}
	}

	const SoldiersList soldiers = sc.stationed_soldiers();
	if (lua_isstring(L, -1) != 0) {
		if (std::string(luaL_checkstring(L, -1)) != "all") {
			report_error(L, "Invalid arguments!");
		}

		// Return All Soldiers
		SoldiersMap hist;
		for (const Widelands::Soldier* s : soldiers) {
			SoldierMapDescr sd(s->get_health_level(), s->get_attack_level(), s->get_defense_level(),
			                   s->get_evade_level());

			SoldiersMap::iterator i = hist.find(sd);
			if (i == hist.end()) {
				hist[sd] = 1;
			} else {
				++i->second;
			}
		}

		// Get this to Lua.
		lua_newtable(L);
		for (const SoldiersMap::value_type& i : hist) {
			lua_createtable(L, 4, 0);
#define PUSHLEVEL(idx, name)                                                                       \
	lua_pushuint32(L, idx);                                                                         \
	lua_pushuint32(L, i.first.name);                                                                \
	lua_rawset(L, -3);
			PUSHLEVEL(1, health)
			PUSHLEVEL(2, attack)
			PUSHLEVEL(3, defense)
			PUSHLEVEL(4, evade)
#undef PUSHLEVEL

			lua_pushuint32(L, i.second);
			lua_rawset(L, -3);
		}
	} else {
		const Widelands::SoldierDescr& soldier_descr =
		   dynamic_cast<const Widelands::SoldierDescr&>(*tribe.get_worker_descr(tribe.soldier()));

		// Only return the number of those requested
		const SoldierMapDescr wanted = unbox_lua_soldier_description(L, 2, soldier_descr);
		Widelands::Quantity rv = 0;
		for (const Widelands::Soldier* s : soldiers) {
			SoldierMapDescr sd(s->get_health_level(), s->get_attack_level(), s->get_defense_level(),
			                   s->get_evade_level());
			if (sd == wanted) {
				++rv;
			}
		}
		lua_pushuint32(L, rv);
	}
	return 1;
}

// Does most of the work of set_soldiers for buildings.
int do_set_soldiers(lua_State* L,
                    const Widelands::Coords& building_position,
                    Widelands::SoldierControl* sc,
                    Widelands::Player* owner) {
	assert(sc != nullptr);
	assert(owner != nullptr);

	const Widelands::TribeDescr& tribe = owner->tribe();
	const Widelands::SoldierDescr& soldier_descr =  //  soldiers
	   dynamic_cast<const Widelands::SoldierDescr&>(*tribe.get_worker_descr(tribe.soldier()));
	SoldiersMap setpoints = parse_set_soldiers_arguments(L, soldier_descr);

	// Get information about current soldiers
	const std::vector<Widelands::Soldier*> curs = sc->stationed_soldiers();
	SoldiersMap hist;
	for (const Widelands::Soldier* s : curs) {
		SoldierMapDescr sd(s->get_health_level(), s->get_attack_level(), s->get_defense_level(),
		                   s->get_evade_level());

		SoldiersMap::iterator i = hist.find(sd);
		if (i == hist.end()) {
			hist[sd] = 1;
		} else {
			++i->second;
		}
		if (setpoints.count(sd) == 0u) {
			setpoints[sd] = 0;
		}
	}

	// Now adjust them
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const SoldiersMap::value_type& sp : setpoints) {
		Widelands::Quantity cur = 0;
		SoldiersMap::iterator i = hist.find(sp.first);
		if (i != hist.end()) {
			cur = i->second;
		}

		int d = sp.second - cur;
		if (d < 0) {
			while (d < 0) {
				for (Widelands::Soldier* s : sc->stationed_soldiers()) {
					SoldierMapDescr is(s->get_health_level(), s->get_attack_level(),
					                   s->get_defense_level(), s->get_evade_level());

					if (is == sp.first) {
						sc->outcorporate_soldier(*s);
						s->remove(egbase);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			for (; d > 0; --d) {
				Widelands::Soldier& soldier = dynamic_cast<Widelands::Soldier&>(
				   soldier_descr.create(egbase, owner, nullptr, building_position));
				soldier.set_level(sp.first.health, sp.first.attack, sp.first.defense, sp.first.evade);
				if (sc->incorporate_soldier(egbase, soldier) != 0) {
					soldier.remove(egbase);
					report_error(L, "No space left for soldier!");
				}
			}
		}
	}
	return 0;
}

// Parses a table of name/count pairs as given from Lua.
void parse_wares_workers(lua_State* L,
                         int table_index,
                         const Widelands::TribeDescr& tribe,
                         InputMap* ware_workers_list,
                         bool is_ware) {
	luaL_checktype(L, table_index, LUA_TTABLE);
	lua_pushnil(L);
	while (lua_next(L, table_index) != 0) {
		const std::string unit_name = luaL_checkstring(L, -2);
		if (is_ware) {
			if (tribe.ware_index(unit_name) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal ware %s", unit_name.c_str());
			}
			ware_workers_list->insert(std::make_pair(
			   std::make_pair(tribe.ware_index(unit_name), Widelands::WareWorker::wwWARE),
			   luaL_checkuint32(L, -1)));
		} else {
			if (tribe.worker_index(unit_name) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal worker %s", unit_name.c_str());
			}
			ware_workers_list->insert(std::make_pair(
			   std::make_pair(tribe.worker_index(unit_name), Widelands::WareWorker::wwWORKER),
			   luaL_checkuint32(L, -1)));
		}
		lua_pop(L, 1);
	}
}

Widelands::BillOfMaterials
parse_wares_as_bill_of_material(lua_State* L, int table_index, const Widelands::TribeDescr& tribe) {
	InputMap input_map;
	parse_wares_workers(L, table_index, tribe, &input_map, true /* is_ware */);
	Widelands::BillOfMaterials result;
	for (const auto& pair : input_map) {
		result.emplace_back(pair.first.first, pair.second);
	}
	return result;
}

const Widelands::TribeDescr* get_tribe_descr(lua_State* L, const std::string& tribename) {
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	if (!descriptions.tribe_exists(tribename)) {
		report_error(L, "Tribe '%s' does not exist", tribename.c_str());
	}
	return descriptions.get_tribe_descr(get_egbase(L).mutable_descriptions()->load_tribe(tribename));
}

std::string soldier_preference_to_string(const Widelands::SoldierPreference p) {
	switch (p) {
	case Widelands::SoldierPreference::kHeroes:
		return "heroes";
	case Widelands::SoldierPreference::kRookies:
		return "rookies";
	case Widelands::SoldierPreference::kAny:
		return "any";
	default:
		NEVER_HERE();
	}
}

Widelands::SoldierPreference string_to_soldier_preference(const std::string& p) {
	if (p == "heroes") {
		return Widelands::SoldierPreference::kHeroes;
	}
	if (p == "rookies") {
		return Widelands::SoldierPreference::kRookies;
	}
	if (p == "any") {
		return Widelands::SoldierPreference::kAny;
	}
	throw wexception("Invalid soldier preference '%s'", p.c_str());
}

// Transforms the given warehouse policy to a string which is used by the lua code
void wh_policy_to_string(lua_State* L, Widelands::StockPolicy p) {
	switch (p) {
	case Widelands::StockPolicy::kNormal:
		lua_pushstring(L, "normal");
		break;
	case Widelands::StockPolicy::kPrefer:
		lua_pushstring(L, "prefer");
		break;
	case Widelands::StockPolicy::kDontStock:
		lua_pushstring(L, "dontstock");
		break;
	case Widelands::StockPolicy::kRemove:
		lua_pushstring(L, "remove");
		break;
	default:
		NEVER_HERE();
	}
}
// Transforms the given string from the lua code to a warehouse policy
Widelands::StockPolicy string_to_wh_policy(lua_State* L, uint32_t index) {
	std::string str = luaL_checkstring(L, index);
	if (str == "normal") {
		return Widelands::StockPolicy::kNormal;
	}
	if (str == "prefer") {
		return Widelands::StockPolicy::kPrefer;
	}
	if (str == "dontstock") {
		return Widelands::StockPolicy::kDontStock;
	}
	if (str == "remove") {
		return Widelands::StockPolicy::kRemove;
	}
	report_error(L, "<%s> is no valid warehouse policy!", str.c_str());
}

Widelands::WarePriority string_to_priority(const std::string& p) {
	if (p == "normal") {
		return Widelands::WarePriority::kNormal;
	}
	if (p == "low") {
		return Widelands::WarePriority::kLow;
	}
	if (p == "very_low") {
		return Widelands::WarePriority::kVeryLow;
	}
	if (p == "high") {
		return Widelands::WarePriority::kHigh;
	}
	if (p == "very_high") {
		return Widelands::WarePriority::kVeryHigh;
	}
	throw wexception("Invalid ware priority '%s'", p.c_str());
}
std::string priority_to_string(const Widelands::WarePriority& priority) {
	if (priority == Widelands::WarePriority::kVeryLow) {
		return "very_low";
	}
	if (priority == Widelands::WarePriority::kLow) {
		return "low";
	}
	if (priority == Widelands::WarePriority::kHigh) {
		return "high";
	}
	if (priority == Widelands::WarePriority::kVeryHigh) {
		return "very_high";
	}
	if (priority == Widelands::WarePriority::kNormal) {
		return "normal";
	}
	NEVER_HERE();
}

/*
 * Upcast the given map object description to a higher type and hand this
 * to Lua. We use this so that scripters always work with the highest class
 * object available.
 */
#define CAST_TO_LUA(klass, lua_klass)                                                              \
	to_lua<lua_klass>(L, new lua_klass(dynamic_cast<const klass*>(descr)))
int upcasted_map_object_descr_to_lua(lua_State* L, const Widelands::MapObjectDescr* const descr) {
	assert(descr != nullptr);

	if (descr->type() >= Widelands::MapObjectType::BUILDING) {
		switch (descr->type()) {
		case Widelands::MapObjectType::CONSTRUCTIONSITE:
			return CAST_TO_LUA(Widelands::ConstructionSiteDescr, LuaConstructionSiteDescription);
		case Widelands::MapObjectType::DISMANTLESITE:
			return CAST_TO_LUA(Widelands::DismantleSiteDescr, LuaDismantleSiteDescription);
		case Widelands::MapObjectType::PRODUCTIONSITE:
			return CAST_TO_LUA(Widelands::ProductionSiteDescr, LuaProductionSiteDescription);
		case Widelands::MapObjectType::MILITARYSITE:
			return CAST_TO_LUA(Widelands::MilitarySiteDescr, LuaMilitarySiteDescription);
		case Widelands::MapObjectType::WAREHOUSE:
			return CAST_TO_LUA(Widelands::WarehouseDescr, LuaWarehouseDescription);
		case Widelands::MapObjectType::MARKET:
			return CAST_TO_LUA(Widelands::MarketDescr, LuaMarketDescription);
		case Widelands::MapObjectType::TRAININGSITE:
			return CAST_TO_LUA(Widelands::TrainingSiteDescr, LuaTrainingSiteDescription);
		default:
			return CAST_TO_LUA(Widelands::BuildingDescr, LuaBuildingDescription);
		}
	} else {
		switch (descr->type()) {
		case Widelands::MapObjectType::WARE:
			return CAST_TO_LUA(Widelands::WareDescr, LuaWareDescription);
		case Widelands::MapObjectType::WORKER:
		case Widelands::MapObjectType::CARRIER:
		case Widelands::MapObjectType::FERRY:
			return CAST_TO_LUA(Widelands::WorkerDescr, LuaWorkerDescription);
		case Widelands::MapObjectType::SOLDIER:
			return CAST_TO_LUA(Widelands::SoldierDescr, LuaSoldierDescription);
		case Widelands::MapObjectType::SHIP:
			return CAST_TO_LUA(Widelands::ShipDescr, LuaShipDescription);
		case Widelands::MapObjectType::IMMOVABLE:
			return CAST_TO_LUA(Widelands::ImmovableDescr, LuaImmovableDescription);
		case Widelands::MapObjectType::RESOURCE:
			return CAST_TO_LUA(Widelands::ResourceDescription, LuaResourceDescription);
		case Widelands::MapObjectType::TERRAIN:
			return CAST_TO_LUA(Widelands::TerrainDescription, LuaTerrainDescription);
		case Widelands::MapObjectType::MAPOBJECT:
		case Widelands::MapObjectType::BATTLE:
		case Widelands::MapObjectType::BOB:
		case Widelands::MapObjectType::CRITTER:
		case Widelands::MapObjectType::FERRY_FLEET:
		case Widelands::MapObjectType::SHIP_FLEET:
		case Widelands::MapObjectType::FLAG:
		case Widelands::MapObjectType::ROAD:
		case Widelands::MapObjectType::WATERWAY:
		case Widelands::MapObjectType::ROADBASE:
		case Widelands::MapObjectType::PORTDOCK:
		case Widelands::MapObjectType::PINNED_NOTE:
		case Widelands::MapObjectType::SHIP_FLEET_YARD_INTERFACE:
		case Widelands::MapObjectType::FERRY_FLEET_YARD_INTERFACE:
		case Widelands::MapObjectType::NAVAL_INVASION_BASE:
			return CAST_TO_LUA(Widelands::MapObjectDescr, LuaMapObjectDescription);
		default:
			verb_log_warn("upcasted_map_object_to_lua: unknown type '%s' to cast to, return general "
			              "MapObjectDescription",
			              Widelands::to_string(descr->type()).c_str());
			return CAST_TO_LUA(Widelands::MapObjectDescr, LuaMapObjectDescription);
		}
	}
}
#undef CAST_TO_LUA

/*
 * Upcast the given map object to a higher type and hand this to
 * Lua. We use this so that scripters always work with the highest class
 * object available.
 */
#define CAST_TO_LUA(k) to_lua<Lua##k>(L, new Lua##k(dynamic_cast<Widelands::k&>(*mo)))
int upcasted_map_object_to_lua(lua_State* L, Widelands::MapObject* mo) {
	if (mo == nullptr) {
		return 0;
	}

	switch (mo->descr().type()) {
	case Widelands::MapObjectType::CRITTER:
		return CAST_TO_LUA(Bob);
	case Widelands::MapObjectType::SHIP:
		return CAST_TO_LUA(Ship);
	// TODO(sirver / Nordfriese): Carrier / Ferry not yet implemented
	case Widelands::MapObjectType::WORKER:
	case Widelands::MapObjectType::CARRIER:
	case Widelands::MapObjectType::FERRY:
		return CAST_TO_LUA(Worker);
	case Widelands::MapObjectType::SOLDIER:
		return CAST_TO_LUA(Soldier);

	case Widelands::MapObjectType::IMMOVABLE:
		return CAST_TO_LUA(BaseImmovable);

	case Widelands::MapObjectType::FLAG:
		return CAST_TO_LUA(Flag);
	case Widelands::MapObjectType::ROAD:
		return CAST_TO_LUA(Road);
	case Widelands::MapObjectType::WATERWAY:
		// TODO(Nordfriese): not yet implemented
		return to_lua<LuaRoad>(L, new LuaRoad(dynamic_cast<Widelands::Waterway&>(*mo)));
	case Widelands::MapObjectType::ROADBASE:
		// TODO(Nordfriese): not yet implemented
		return to_lua<LuaRoad>(L, new LuaRoad(dynamic_cast<Widelands::RoadBase&>(*mo)));
	case Widelands::MapObjectType::PORTDOCK:
		return CAST_TO_LUA(PortDock);

	case Widelands::MapObjectType::BUILDING:
		return CAST_TO_LUA(Building);
	case Widelands::MapObjectType::CONSTRUCTIONSITE:
		return CAST_TO_LUA(ConstructionSite);
	case Widelands::MapObjectType::DISMANTLESITE:
		return CAST_TO_LUA(DismantleSite);
	case Widelands::MapObjectType::WAREHOUSE:
		return CAST_TO_LUA(Warehouse);
	case Widelands::MapObjectType::MARKET:
		return CAST_TO_LUA(Market);
	case Widelands::MapObjectType::PRODUCTIONSITE:
		return CAST_TO_LUA(ProductionSite);
	case Widelands::MapObjectType::MILITARYSITE:
		return CAST_TO_LUA(MilitarySite);
	case Widelands::MapObjectType::TRAININGSITE:
		return CAST_TO_LUA(TrainingSite);
	case Widelands::MapObjectType::PINNED_NOTE:
		return CAST_TO_LUA(PinnedNote);
	case Widelands::MapObjectType::SHIP_FLEET_YARD_INTERFACE:
		return CAST_TO_LUA(ShipFleetYardInterface);
	case Widelands::MapObjectType::FERRY_FLEET_YARD_INTERFACE:
		return CAST_TO_LUA(FerryFleetYardInterface);
	case Widelands::MapObjectType::NAVAL_INVASION_BASE:
		return CAST_TO_LUA(NavalInvasionBase);
	case Widelands::MapObjectType::MAPOBJECT:
	case Widelands::MapObjectType::RESOURCE:
	case Widelands::MapObjectType::TERRAIN:
	case Widelands::MapObjectType::BATTLE:
	case Widelands::MapObjectType::BOB:
	case Widelands::MapObjectType::SHIP_FLEET:
	case Widelands::MapObjectType::FERRY_FLEET:
	case Widelands::MapObjectType::WARE:
	default:
		throw LuaError(
		   format("upcasted_map_object_to_lua: Unknown %i", static_cast<int>(mo->descr().type())));
	}
}
#undef CAST_TO_LUA

// This is used for get_ware/workers functions, when argument can be
// 'all', single ware/worker, or array of ware/workers
RequestedWareWorker parse_wares_workers_list(lua_State* L,
                                             const Widelands::TribeDescr& tribe,
                                             Widelands::DescriptionIndex* single_item,
                                             std::vector<Widelands::DescriptionIndex>* item_list,
                                             bool is_ware) {
	RequestedWareWorker result = RequestedWareWorker::kUndefined;
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "One argument is required for produced_wares_count()");
	}

	/* If we have single string as an argument */
	if (lua_isstring(L, 2) != 0) {

		std::string what = luaL_checkstring(L, -1);
		if (what != "all") {
			result = RequestedWareWorker::kSingle;
			// This is name of ware/worker
			if (is_ware) {
				*single_item = tribe.ware_index(what);
			} else {
				*single_item = tribe.worker_index(what);
			}
			if (*single_item == Widelands::INVALID_INDEX) {
				report_error(L, "Unrecognized ware/worker %s", what.c_str());
			}
		} else {
			// we collect all wares/workers and push it to item_list
			result = RequestedWareWorker::kAll;
			if (is_ware) {
				for (auto idx : tribe.wares()) {
					item_list->push_back(idx);
				}
			} else {
				for (auto idx : tribe.workers()) {
					item_list->push_back(idx);
				}
			}
		}
	} else {
		result = RequestedWareWorker::kList;
		/* we got array of names, and so fill the indexes into item_list */
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			std::string what = luaL_checkstring(L, -1);
			lua_pop(L, 1);
			if (is_ware) {
				item_list->push_back(tribe.ware_index(what));
			} else {
				item_list->push_back(tribe.worker_index(what));
			}
			if (item_list->back() == Widelands::INVALID_INDEX) {
				report_error(L, "Unrecognized ware %s", what.c_str());
			}
		}
	}
	assert((*single_item == Widelands::INVALID_INDEX) != item_list->empty());
	return result;
}

// Very similar to above function, but expects numbers for every received ware/worker
RequestedWareWorker parse_wares_workers_counted(lua_State* L,
                                                const Widelands::TribeDescr& tribe,
                                                InputMap* ware_workers_list,
                                                const bool is_ware,
                                                const bool allow_ferry) {
	RequestedWareWorker result = RequestedWareWorker::kUndefined;
	int32_t nargs = lua_gettop(L);
	if (nargs != 2 && nargs != 3) {
		report_error(L, "Wrong number of arguments to set ware/worker method!");
	}

	// We either received, two items string,int:
	if (nargs == 3) {
		const std::string unit_name = luaL_checkstring(L, 2);
		result = RequestedWareWorker::kSingle;
		if (is_ware) {
			if (tribe.ware_index(unit_name) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal ware %s", unit_name.c_str());
			}
			ware_workers_list->insert(std::make_pair(
			   std::make_pair(tribe.ware_index(unit_name), Widelands::WareWorker::wwWARE),
			   luaL_checkuint32(L, 3)));
		} else {
			const Widelands::DescriptionIndex worker_index = tribe.worker_index(unit_name);
			if (worker_index == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal worker %s", unit_name.c_str());
			}
			if (worker_index == tribe.soldier()) {
				report_error(L, "Do not set soldiers via set_workers(), use set_soldiers() instead");
			}
			if (worker_index == tribe.ferry() && !allow_ferry) {
				report_error(L, "This map object can not contain ferries");
			}
			ware_workers_list->insert(std::make_pair(
			   std::make_pair(worker_index, Widelands::WareWorker::wwWORKER), luaL_checkuint32(L, 3)));
		}
	} else {
		result = RequestedWareWorker::kList;
		// or we got a table with name:quantity
		parse_wares_workers(L, 2, tribe, ware_workers_list, is_ware);
	}
	return result;
}

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
.. currentmodule:: wl.map.MapObject

Common functions
^^^^^^^^^^^^^^^^

Some map objects share the same functions and attributes:

   * :ref:`has_wares`
   * :ref:`has_workers`
   * :ref:`has_soldiers`
   * :ref:`has_inputs`
*/

/* RST
.. _has_wares:

Common properties for objects with ware storage
-----------------------------------------------

Functions for objects which can store wares.
Supported at the time of this writing by :class:`~wl.map.Flag`
and :class:`~wl.map.Warehouse`.

For objects which consume wares, see: :ref:`has_inputs`.
*/

/* RST
.. method:: get_wares(which)

   Gets the number of wares that currently reside here.

   :arg which:  Can be either of:

   * The string :const:`"all"`.
      In this case the function will return a
      :class:`table` of ``{ware_name=amount}`` pairs that gives information
      about all ware information available for this object.
   * A ware name.
      In this case a single integer is returned. No check is made
      if this ware makes sense for this location, you can for example ask a
      :const:`lumberjacks_hut` for the number of :const:`granite` he has
      and he will return 0.
   * An :class:`array` of ware names.
      In this case a :class:`table` of
      ``{ware_name=amount}`` pairs is returned where only the requested wares
      are listed. All other entries are :const:`nil`.

   :returns: :class:`integer` or :class:`table`
*/

/* RST
.. method:: set_wares(which[, amount])

   Sets the wares available in this location. Either takes two arguments,
   a ware name and an amount to set it to. Or it takes a :class:`table` of
   ``{ware_name=amount}`` pairs. Wares are created and added to an economy out
   of thin air.

   Setting some wares does not influence other wares for

   - warehouse

   but sets other wares to empty for

   - flag

   :arg which: Name of ware or a :const:`table` of `{ware_name=amount}`` pairs.
   :type which: :class:`string` or :class:`table`
   :arg amount: This many units will be available after the call.
   :type amount: :class:`integer`
*/

/* RST
.. attribute:: valid_wares

   (RO) A :class:`table` of ``{ware_name=count}`` if storage is somehow
   constrained in this location. For example for a
   :class:`~wl.map.ProductionSite` this is the information what wares
   and how much can be stored as inputs. For unconstrained storage (like
   :class:`~wl.map.Warehouse`) this is :const:`nil`.

   You can use this to quickly fill a building:

   .. code-block:: lua

      if b.valid_wares then b:set_wares(b.valid_wares) end
*/

/* RST
.. _has_inputs:

Common properties for objects requiring production inputs
---------------------------------------------------------

Supported at the time of this writing by
:class:`~wl.map.ProductionSite` and :class:`~wl.map.TrainingSite`.
These functions allows to set workers as inputs. These workers
are consumed by the production or trainings programs. To access
workers that do the work, see: :ref:`has_workers`.
*/

/* RST
.. method:: get_inputs(which)

   Gets the number of wares and workers that currently reside here
   for consumption.

   :arg which:  Can be either of:

   * The string :const:`all`.
      In this case the function will return a
      :class:`table` of ``{ware/worker_name,amount}`` pairs that gives
      information about all ware information available for this object.
   * A ware or worker name.
      In this case a single integer is returned. No check is made
      if this ware/worker makes sense for this location, you can for example ask a
      :const:`lumberjacks_hut` for the number of :const:`granite` he has
      and he will return 0.
   * An :class:`array` of ware and worker names.
      In this case a :class:`table` of
      ``{ware/worker_name=amount}`` pairs is returned where only the requested
      wares/workers are listed. All other entries are :const:`nil`.

   :returns: :class:`integer` or :class:`table`
*/

/* RST
.. method:: set_inputs(which[, amount])

   Sets the wares/workers available in this location which will
   be consumed by the production/training programm. Either takes two arguments,
   a ware/worker name and an amount to set it to. Or it takes a :class:`table` of
   ``{ware/worker_name=amount}`` pairs. Wares are created and added to an
   economy out of thin air.

   Setting a few wares does not influence other wares for

   - production site
   - training site

   :arg which: name of ware/worker or ``{ware/worker_name=amount}`` :class:`table`
   :type which: :class:`string` or :class:`table`
   :arg amount: this many units will be available after the call
   :type amount: :class:`integer`
*/

/* RST
.. attribute:: valid_inputs

   (RO) A :class:`table` of ``{ware/worker_name=amount}`` which describes how
   many wares/workers can be stored here for consumption. For example for a
   :class:`~wl.map.ProductionSite` this is the information what wares/workers
   and can be stored in which amount as inputs.

   You can use this to quickly fill a building:

   .. code-block:: lua

      if b.valid_inputs then b:set_inputs(b.valid_inputs) end
*/

/* RST
.. method:: set_priority(ware, prio [, cs_setting = false])

   Sets the priority for the given ware inputqueue.

   :arg ware: ware name
   :type ware: :class:`string`
   :arg prio: The new priority. One of ``"very_low"``, ``"low"``, ``"normal"``, ``"high"``,
      or ``"very_high"``.
   :type prio: :class:`string`
   :arg cs_setting: Only valid for productionsite-constructionsites. If :const:`true`,
      refers to the settings to apply after construction.
   :type cs_setting: :class:`bool`
*/

/* RST
.. method:: get_priority(ware [, cs_setting = false])

   Returns the priority for the given ware inputqueue. See also :meth:`set_priority`.

   :arg ware: A ware name.
   :type ware: :class:`string`
   :arg cs_setting: Only valid for productionsite-constructionsites. If :const:`true`,
      refers to the settings to apply after construction.
   :type cs_setting: :class:`bool`

   :returns: :class:`string`
*/

/* RST
.. method:: set_desired_fill(item, fill [, cs_setting = false])

   Sets the desired fill for the given ware or worker inputqueue, as if the player had clicked
   the increase/decrease buttons.

   :arg item: Ware or worker name.
   :type item: :class:`string`
   :arg fill: The desired fill.
   :type fill: :class:`integer`
   :arg cs_setting: Only valid for productionsite-constructionsites. If :const:`true`, refers to
      the settings to apply after construction.
   :type cs_setting: :class:`bool`
*/

/* RST
.. method:: get_desired_fill(item, fill [, cs_setting = false])

   Returns the desired fill for the given ware or worker inputqueue. See also
   :meth:`set_desired_fill`.

   :arg item: Ware or worker name.
   :type item: :class:`string`
   :arg cs_setting: Only valid for productionsite-constructionsites. If :const:`true`, refers to
      the settings to apply after construction.
   :type cs_setting: :class:`bool`

   :returns: :class:`integer`
*/

/* RST
.. _has_workers:

Common properties for objects requiring workers
-----------------------------------------------

Supported at the time of this writing by :class:`~wl.map.Road`,
:class:`~wl.map.Warehouse` and :class:`~wl.map.ProductionSite`.
In the case of ProductionSites, these methods allow access to the
workers which do the work instead of workers which are consumed.
For workers which are consumed, see: :ref:`has_inputs`.
*/

/* RST
.. method:: get_workers(which)

   Similar to :meth:`wl.map.MapObject.get_wares`.
*/

/* RST
.. method:: set_workers(which[, amount])

   Similar to :meth:`wl.map.MapObject.set_wares`.

   Note that soldiers must not be set via this method.
   Use :meth:`wl.map.MapObject.set_soldiers` instead.

   Ferries can not be set by this method either, except for waterways.

   Setting some wares does not influence other wares for

   - warehouse

   but sets other wares to empty for

   - road
   - productionsite
*/

/* RST
.. attribute:: valid_workers

   (RO) Similar to :attr:`wl.map.MapObject.valid_wares` but for workers in this
   location.
*/

/* RST
.. _has_soldiers:

Common properties for objects garrisoning soldiers
--------------------------------------------------

Supported at the time of this writing by
:class:`~wl.map.Warehouse`, :class:`~wl.map.MilitarySite` and
:class:`~wl.map.TrainingSite`.
*/

/* RST
.. method:: get_soldiers(descr)

   Gets information about the soldiers in a location.

   :arg descr: Can be either of:

   * A soldier description.
      Returns an :class:`integer` which is the number of soldiers of this
      kind in this building.

      A soldier description is an :class:`array` that contains the level for
      health, attack, defense and evade (in this order). A usage example:

      .. code-block:: lua

         building:get_soldiers({0,0,0,0})

      would return the number of soldiers of level 0 in this location.

   * The string :const:`"all"`.
      In this case a :class:`table` of ``{soldier_descriptions=count}`` is
      returned. Note that the following will not work, because Lua indexes
      tables by identity:

      .. code-block:: lua

         building:set_soldiers({0,0,0,0}, 100)
         building:get_soldiers({0,0,0,0}) -- works, returns 100
         building:get_soldiers("all")[{0,0,0,0}] -- works not, this is nil

         -- Following is a working way to check for a {0,0,0,0} soldier
         for descr,count in pairs(building:get_soldiers("all")) do
            if descr[1] == 0 and descr[2] == 0 and
               descr[3] == 0 and descr[4] == 0 then
                  print(count)
            end
         end

   * The string :const:`"present"`.
      .. versionadded:: 1.3

      Returns an :class:`integer` which is the number of soldiers present
      in this building.

   * The string :const:`"stationed"`.
      .. versionadded:: 1.3

      Returns an :class:`integer` which is the number of soldiers stationed
      in this building. Stationed soldiers include present soldiers and the
      soldiers who left this building to fight.

   * The string :const:`"associated"`.
      .. versionadded:: 1.3

      Returns an :class:`integer` which is the number of soldiers associated
      to this building. Associated soldiers include stationed soldiers and
      soldiers who are coming to this building.

   :returns: Number of soldiers that match **descr** or the :class:`table`
      containing all soldiers
   :rtype: :class:`integer` or :class:`table`.
*/

/* RST
.. method:: set_soldiers(which[, amount])

   Analogous to :meth:`wl.map.MapObject.set_workers`, but for soldiers. Instead of
   a name an :class:`array` is used to define the soldier. See
   below for an example.

   Setting some soldiers sets all others to empty for

   - military site
   - training site
   - warehouse

   :arg which: Either a :class:`table` of ``{description=count}`` pairs or one
      description. In that case amount has to be specified as well.
   :type which: :class:`table` or :class:`array`.

   Usage example:

   .. code-block:: lua

      building:set_soldiers({0,0,0,0}, 100)

   would add 100 level 0 soldiers. While

   .. code-block:: lua

      building:set_soldiers({
        [{0,0,0,0}] = 10,
        [{1,2,3,4}] = 5,
      })

   would set 10 level 0 soldier and 5 soldiers with hit point level 1,
   attack level 2, defense level 3 and evade level 4 (as long as this is
   legal for the players tribe).
*/

/* RST
.. attribute:: max_soldiers

   (RO) The maximum number of soldiers that can be inside this building at
   one time. If it is not constrained, like for :class:`~wl.map.Warehouse`,
   this will be :const:`nil`.
*/

/* RST
.. currentmodule:: wl.map

Module Classes
^^^^^^^^^^^^^^

*/

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

const static struct luaL_Reg wlmap[] = {{nullptr, nullptr}};

void luaopen_wlmap(lua_State* L) {
	lua_getglobal(L, "wl");    // S: wl_table
	lua_pushstring(L, "map");  // S: wl_table "map"
	luaL_newlib(L, wlmap);     // S: wl_table "map" wl.map_table
	lua_settable(L, -3);       // S: wl_table
	lua_pop(L, 1);             // S:

	register_class<LuaMap>(L, "map");
	register_class<LuaTribeDescription>(L, "map");
	register_class<LuaMapObjectDescription>(L, "map");

	register_class<LuaImmovableDescription>(L, "map", true);
	add_parent<LuaImmovableDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaBuildingDescription>(L, "map", true);
	add_parent<LuaBuildingDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaConstructionSiteDescription>(L, "map", true);
	add_parent<LuaConstructionSiteDescription, LuaBuildingDescription>(L);
	add_parent<LuaConstructionSiteDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaDismantleSiteDescription>(L, "map", true);
	add_parent<LuaDismantleSiteDescription, LuaBuildingDescription>(L);
	add_parent<LuaDismantleSiteDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaProductionSiteDescription>(L, "map", true);
	add_parent<LuaProductionSiteDescription, LuaBuildingDescription>(L);
	add_parent<LuaProductionSiteDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMilitarySiteDescription>(L, "map", true);
	add_parent<LuaMilitarySiteDescription, LuaBuildingDescription>(L);
	add_parent<LuaMilitarySiteDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTrainingSiteDescription>(L, "map", true);
	add_parent<LuaTrainingSiteDescription, LuaProductionSiteDescription>(L);
	add_parent<LuaTrainingSiteDescription, LuaBuildingDescription>(L);
	add_parent<LuaTrainingSiteDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWarehouseDescription>(L, "map", true);
	add_parent<LuaWarehouseDescription, LuaBuildingDescription>(L);
	add_parent<LuaWarehouseDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMarketDescription>(L, "map", true);
	add_parent<LuaMarketDescription, LuaBuildingDescription>(L);
	add_parent<LuaMarketDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaShipDescription>(L, "map", true);
	add_parent<LuaShipDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWareDescription>(L, "map", true);
	add_parent<LuaWareDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWorkerDescription>(L, "map", true);
	add_parent<LuaWorkerDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaSoldierDescription>(L, "map", true);
	add_parent<LuaSoldierDescription, LuaWorkerDescription>(L);
	add_parent<LuaSoldierDescription, LuaMapObjectDescription>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaResourceDescription>(L, "map");
	register_class<LuaTerrainDescription>(L, "map");

	register_class<LuaField>(L, "map");
	register_class<LuaPlayerSlot>(L, "map");
	register_class<LuaEconomy>(L, "map");
	register_class<LuaMapObject>(L, "map");

	register_class<LuaBob>(L, "map", true);
	add_parent<LuaBob, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWorker>(L, "map", true);
	add_parent<LuaWorker, LuaBob>(L);
	add_parent<LuaWorker, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaSoldier>(L, "map", true);
	add_parent<LuaSoldier, LuaWorker>(L);
	add_parent<LuaSoldier, LuaBob>(L);
	add_parent<LuaSoldier, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaShip>(L, "map", true);
	add_parent<LuaShip, LuaBob>(L);
	add_parent<LuaShip, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaBaseImmovable>(L, "map", true);
	add_parent<LuaBaseImmovable, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaPlayerImmovable>(L, "map", true);
	add_parent<LuaPlayerImmovable, LuaBaseImmovable>(L);
	add_parent<LuaPlayerImmovable, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaBuilding>(L, "map", true);
	add_parent<LuaBuilding, LuaPlayerImmovable>(L);
	add_parent<LuaBuilding, LuaBaseImmovable>(L);
	add_parent<LuaBuilding, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaPortDock>(L, "map", true);
	add_parent<LuaPortDock, LuaPlayerImmovable>(L);
	add_parent<LuaPortDock, LuaBaseImmovable>(L);
	add_parent<LuaPortDock, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaFlag>(L, "map", true);
	add_parent<LuaFlag, LuaPlayerImmovable>(L);
	add_parent<LuaFlag, LuaBaseImmovable>(L);
	add_parent<LuaFlag, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaRoad>(L, "map", true);
	add_parent<LuaRoad, LuaPlayerImmovable>(L);
	add_parent<LuaRoad, LuaBaseImmovable>(L);
	add_parent<LuaRoad, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaConstructionSite>(L, "map", true);
	add_parent<LuaConstructionSite, LuaBuilding>(L);
	add_parent<LuaConstructionSite, LuaPlayerImmovable>(L);
	add_parent<LuaConstructionSite, LuaBaseImmovable>(L);
	add_parent<LuaConstructionSite, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaDismantleSite>(L, "map", true);
	add_parent<LuaDismantleSite, LuaBuilding>(L);
	add_parent<LuaDismantleSite, LuaPlayerImmovable>(L);
	add_parent<LuaDismantleSite, LuaBaseImmovable>(L);
	add_parent<LuaDismantleSite, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaWarehouse>(L, "map", true);
	add_parent<LuaWarehouse, LuaBuilding>(L);
	add_parent<LuaWarehouse, LuaPlayerImmovable>(L);
	add_parent<LuaWarehouse, LuaBaseImmovable>(L);
	add_parent<LuaWarehouse, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMarket>(L, "map", true);
	add_parent<LuaMarket, LuaBuilding>(L);
	add_parent<LuaMarket, LuaPlayerImmovable>(L);
	add_parent<LuaMarket, LuaBaseImmovable>(L);
	add_parent<LuaMarket, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaProductionSite>(L, "map", true);
	add_parent<LuaProductionSite, LuaBuilding>(L);
	add_parent<LuaProductionSite, LuaPlayerImmovable>(L);
	add_parent<LuaProductionSite, LuaBaseImmovable>(L);
	add_parent<LuaProductionSite, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaMilitarySite>(L, "map", true);
	add_parent<LuaMilitarySite, LuaBuilding>(L);
	add_parent<LuaMilitarySite, LuaPlayerImmovable>(L);
	add_parent<LuaMilitarySite, LuaBaseImmovable>(L);
	add_parent<LuaMilitarySite, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaTrainingSite>(L, "map", true);
	add_parent<LuaTrainingSite, LuaProductionSite>(L);
	add_parent<LuaTrainingSite, LuaBuilding>(L);
	add_parent<LuaTrainingSite, LuaPlayerImmovable>(L);
	add_parent<LuaTrainingSite, LuaBaseImmovable>(L);
	add_parent<LuaTrainingSite, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaPinnedNote>(L, "map", true);
	add_parent<LuaPinnedNote, LuaBob>(L);
	add_parent<LuaPinnedNote, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaNavalInvasionBase>(L, "map", true);
	add_parent<LuaNavalInvasionBase, LuaBob>(L);
	add_parent<LuaNavalInvasionBase, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaShipFleetYardInterface>(L, "map", true);
	add_parent<LuaShipFleetYardInterface, LuaBob>(L);
	add_parent<LuaShipFleetYardInterface, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table

	register_class<LuaFerryFleetYardInterface>(L, "map", true);
	add_parent<LuaFerryFleetYardInterface, LuaBob>(L);
	add_parent<LuaFerryFleetYardInterface, LuaMapObject>(L);
	lua_pop(L, 1);  // Pop the meta table
}
}  // namespace LuaMaps
