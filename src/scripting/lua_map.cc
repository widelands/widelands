/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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

#include <cstring>

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/input_queue.h"
#include "economy/waterway.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/world/critter.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/widelands_geometry.h"
#include "scripting/factory.h"
#include "scripting/globals.h"
#include "scripting/lua_errors.h"
#include "scripting/lua_game.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelfunctions.h"

namespace LuaMaps {

/* RST
:mod:`wl.map`
=============

.. module:: wl.map
   :synopsis: Provides access to Fields and Objects on the map

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.map

*/

namespace {

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

struct SoldierMapDescr {
	SoldierMapDescr(uint8_t init_health,
	                uint8_t init_attack,
	                uint8_t init_defense,
	                uint8_t init_evade)
	   : health(init_health), attack(init_attack), defense(init_defense), evade(init_evade) {
	}
	SoldierMapDescr() = default;

	uint8_t health = 0;
	uint8_t attack = 0;
	uint8_t defense = 0;
	uint8_t evade = 0;

	bool operator<(const SoldierMapDescr& ot) const {
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
	bool operator==(const SoldierMapDescr& ot) const {
		return (health == ot.health && attack == ot.attack && defense == ot.defense &&
		        evade == ot.evade);
	}
};

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

// Sort functor to sort the owners claiming a field by their influence.
int sort_claimers(const PlrInfluence& first, const PlrInfluence& second) {
	return static_cast<int>(first.second > second.second);
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
int do_set_workers(lua_State* L, TMapObject* pi, const WaresWorkersMap& valid_workers) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::TribeDescr& tribe = pi->owner().tribe();

	// setpoints is map of index:quantity
	InputMap setpoints;
	parse_wares_workers_counted(L, tribe, &setpoints, false);

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
		report_error(L, "health level (%i) > max health level (%i)", soldier_descr.health,
		             sd.get_max_health_level());
	}

	lua_pushuint32(L, 2);
	lua_rawget(L, table_index);
	soldier_descr.attack = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.attack > sd.get_max_attack_level()) {
		report_error(L, "attack level (%i) > max attack level (%i)", soldier_descr.attack,
		             sd.get_max_attack_level());
	}

	lua_pushuint32(L, 3);
	lua_rawget(L, table_index);
	soldier_descr.defense = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.defense > sd.get_max_defense_level()) {
		report_error(L, "defense level (%i) > max defense level (%i)", soldier_descr.defense,
		             sd.get_max_defense_level());
	}

	lua_pushuint32(L, 4);
	lua_rawget(L, table_index);
	soldier_descr.evade = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.evade > sd.get_max_evade_level()) {
		report_error(L, "evade level (%i) > max evade level (%i)", soldier_descr.evade,
		             sd.get_max_evade_level());
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
		if (is_ware) {
			if (tribe.ware_index(luaL_checkstring(L, -2)) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal ware %s", luaL_checkstring(L, -2));
			}
			ware_workers_list->insert(
			   std::make_pair(std::make_pair(tribe.ware_index(luaL_checkstring(L, -2)),
			                                 Widelands::WareWorker::wwWARE),
			                  luaL_checkuint32(L, -1)));
		} else {
			if (tribe.worker_index(luaL_checkstring(L, -2)) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal worker %s", luaL_checkstring(L, -2));
			}
			ware_workers_list->insert(
			   std::make_pair(std::make_pair(tribe.worker_index(luaL_checkstring(L, -2)),
			                                 Widelands::WareWorker::wwWORKER),
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
		result.push_back(std::make_pair(pair.first.first, pair.second));
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
	}
	NEVER_HERE();
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

}  // namespace

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
	case Widelands::MapObjectType::MAPOBJECT:
	case Widelands::MapObjectType::RESOURCE:
	case Widelands::MapObjectType::TERRAIN:
	case Widelands::MapObjectType::BATTLE:
	case Widelands::MapObjectType::BOB:
	case Widelands::MapObjectType::SHIP_FLEET:
	case Widelands::MapObjectType::FERRY_FLEET:
	case Widelands::MapObjectType::WARE:
	case Widelands::MapObjectType::PINNED_NOTE:
	case Widelands::MapObjectType::SHIP_FLEET_YARD_INTERFACE:
	case Widelands::MapObjectType::FERRY_FLEET_YARD_INTERFACE:
		throw LuaError(
		   format("upcasted_map_object_to_lua: Unknown %i", static_cast<int>(mo->descr().type())));
	}
	NEVER_HERE();
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
                                                bool is_ware) {
	RequestedWareWorker result = RequestedWareWorker::kUndefined;
	int32_t nargs = lua_gettop(L);
	if (nargs != 2 && nargs != 3) {
		report_error(L, "Wrong number of arguments to set ware/worker method!");
	}

	// We either received, two items string,int:
	if (nargs == 3) {
		result = RequestedWareWorker::kSingle;
		if (is_ware) {
			if (tribe.ware_index(luaL_checkstring(L, 2)) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal ware %s", luaL_checkstring(L, 2));
			}
			ware_workers_list->insert(std::make_pair(
			   std::make_pair(tribe.ware_index(luaL_checkstring(L, 2)), Widelands::WareWorker::wwWARE),
			   luaL_checkuint32(L, 3)));
		} else {
			if (tribe.worker_index(luaL_checkstring(L, 2)) == Widelands::INVALID_INDEX) {
				report_error(L, "Illegal worker %s", luaL_checkstring(L, 2));
			}
			ware_workers_list->insert(
			   std::make_pair(std::make_pair(tribe.worker_index(luaL_checkstring(L, 2)),
			                                 Widelands::WareWorker::wwWORKER),
			                  luaL_checkuint32(L, 3)));
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

   :returns: Number of soldiers that match **descr** or the :class:`table`
      containing all soldiers
   :rtype: :class:`integer` or :class:`table`.
*/

/* RST
.. method:: set_soldiers(which[, amount])

   Analogous to :meth:`wl.map.MapObject.set_workers`, but for soldiers. Instead of
   a name an :class:`array` is used to define the soldier. See
   below for an example.

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

   would add 10 level 0 soldier and 5 soldiers with hit point level 1,
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
	if (lua_gettop(L) > 3 && !lua_isnil(L, 4)) {
		// TODO(GunChleoc): Compatibility, remove after v1.0
		log_warn("Found deprecated parameter '%s' in place_immovable call, placing '%s'",
		         luaL_checkstring(L, 4), objname.c_str());
	}

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

/* RST
ImmovableDescription
--------------------

.. class:: ImmovableDescription

   A static description of a :class:`base immovable <BaseImmovable>`. See also
   :class:`MapObjectDescription` for more properties.
*/
const char LuaImmovableDescription::className[] = "ImmovableDescription";
const MethodType<LuaImmovableDescription> LuaImmovableDescription::Methods[] = {
   METHOD(LuaImmovableDescription, has_attribute),
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
   .. method:: has_attribute(attribute_name)

      Returns :const:`true` if the immovable has the attribute, :const:`false` otherwise.

      :arg attribute_name: The attribute that we are checking for.
      :type attribute_name: :class:`string`

*/
int LuaImmovableDescription::has_attribute(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::MapObjectDescr::AttributeIndex attribute_id =
	   Widelands::MapObjectDescr::get_attribute_id(luaL_checkstring(L, 2));
	lua_pushboolean(L, static_cast<int>(get()->has_attribute(attribute_id)));
	return 1;
}

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
   .. attribute:: vision range

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

/* RST
ConstructionSiteDescription
---------------------------

.. class:: ConstructionSiteDescription

   A static description of a tribe's constructionsite. See the parent classes for more properties.
*/
const char LuaConstructionSiteDescription::className[] = "ConstructionSiteDescription";
const MethodType<LuaConstructionSiteDescription> LuaConstructionSiteDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaConstructionSiteDescription> LuaConstructionSiteDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/* RST
DismantleSiteDescription
---------------------------

.. class:: DismantleSiteDescription

   A static description of a tribe's dismantlesite. See the parent classes for more properties.
*/
const char LuaDismantleSiteDescription::className[] = "DismantleSiteDescription";
const MethodType<LuaDismantleSiteDescription> LuaDismantleSiteDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaDismantleSiteDescription> LuaDismantleSiteDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/* RST
ProductionSiteDescription
-------------------------

.. class:: ProductionSiteDescription

   A static description of a tribe's productionsite.

   See the parent classes for more properties.
*/
const char LuaProductionSiteDescription::className[] = "ProductionSiteDescription";
const MethodType<LuaProductionSiteDescription> LuaProductionSiteDescription::Methods[] = {
   METHOD(LuaProductionSiteDescription, consumed_wares_workers),
   METHOD(LuaProductionSiteDescription, produced_wares),
   METHOD(LuaProductionSiteDescription, recruited_workers),
   {nullptr, nullptr},
};
const PropertyType<LuaProductionSiteDescription> LuaProductionSiteDescription::Properties[] = {
   PROP_RO(LuaProductionSiteDescription, inputs),
   PROP_RO(LuaProductionSiteDescription, collected_bobs),
   PROP_RO(LuaProductionSiteDescription, collected_immovables),
   PROP_RO(LuaProductionSiteDescription, collected_resources),
   PROP_RO(LuaProductionSiteDescription, created_bobs),
   PROP_RO(LuaProductionSiteDescription, created_immovables),
   PROP_RO(LuaProductionSiteDescription, created_resources),
   PROP_RO(LuaProductionSiteDescription, output_ware_types),
   PROP_RO(LuaProductionSiteDescription, output_worker_types),
   PROP_RO(LuaProductionSiteDescription, production_programs),
   PROP_RO(LuaProductionSiteDescription, supported_productionsites),
   PROP_RO(LuaProductionSiteDescription, supported_by_productionsites),
   PROP_RO(LuaProductionSiteDescription, working_positions),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: inputs

      (RO) An :class:`array` with :class:`WareDescription` containing the wares that
      the productionsite needs for its production.
*/
int LuaProductionSiteDescription::get_inputs(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& input_ware : get()->input_wares()) {
		lua_pushint32(L, index++);
		const Widelands::WareDescr* descr =
		   get_egbase(L).descriptions().get_ware_descr(input_ware.first);
		to_lua<LuaWareDescription>(L, new LuaWareDescription(descr));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: collected_bobs

      (RO) An :class:`array` of :class:`MapObjectDescription` containing the bobs that
      this building will collect from the map.
      For example, a Hunters's Hut will hunt some critters for meat.

      **Note:** At the moment, only critters are supported here, because we don't
      have any other use case.
*/
int LuaProductionSiteDescription::get_collected_bobs(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const std::string& critter_name : get()->collected_bobs()) {
		lua_pushint32(L, index++);
		const Widelands::CritterDescr* critter =
		   egbase.descriptions().get_critter_descr(critter_name);
		assert(critter != nullptr);
		to_lua<LuaMapObjectDescription>(L, new LuaMapObjectDescription(critter));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: collected_immovables

      (RO) An :class:`array` of :class:`ImmovableDescription` containing the immovables that
      this building will collect from the map.
      For example, a Woodcutters's House will cut down trees to obtain logs, and the
      Fruit Collector's House will harvest fruit from berry bushes.
*/
int LuaProductionSiteDescription::get_collected_immovables(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const std::string& immovable_name : get()->collected_immovables()) {
		lua_pushint32(L, index++);
		const Widelands::ImmovableDescr* immovable = egbase.descriptions().get_immovable_descr(
		   egbase.descriptions().immovable_index(immovable_name));
		assert(immovable != nullptr);
		to_lua<LuaImmovableDescription>(L, new LuaImmovableDescription(immovable));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: collected_resources

      (RO) An :class:`array` of :class:`ResourceDescription` containing the resources that
      this building will collect from the map, along with the maximum percentage mined and the
      chance to still find some more after depletion. E.g. for a Fisher's Hut this will be:

      .. code-block:: lua

       {
            {
               resource = <resource description for fish>,
               yield = 100,
               when_empty = 0
            }
         }

      and for a Barbarian Coal Mine this will be:

      .. code-block:: lua

         {
            {
               resource = <resource description for coal>,
               yield = 33.33,
               when_empty = 5
            }
         }
*/
int LuaProductionSiteDescription::get_collected_resources(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const auto& resource_info : get()->collected_resources()) {
		lua_pushint32(L, index++);
		lua_newtable(L);
		lua_pushstring(L, "resource");
		const Widelands::ResourceDescription* resource = egbase.descriptions().get_resource_descr(
		   egbase.descriptions().resource_index(resource_info.first));
		assert(resource != nullptr);
		to_lua<LuaResourceDescription>(L, new LuaResourceDescription(resource));
		lua_rawset(L, -3);
		lua_pushstring(L, "yield");
		lua_pushnumber(L, resource_info.second.max_percent / 100.0);
		lua_settable(L, -3);
		lua_pushstring(L, "when_empty");
		lua_pushnumber(L, resource_info.second.depleted_chance / 100.0);
		lua_settable(L, -3);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: created_immovables

      (RO) An :class:`array` of :class:`ImmovableDescription` containing the immovables that
      this building will place on the map.
      For example, a Foresters's House will create trees, and the Berry Farm some berry bushes.
*/
int LuaProductionSiteDescription::get_created_immovables(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const std::string& immovable_name : get()->created_immovables()) {
		lua_pushint32(L, index++);
		const Widelands::ImmovableDescr* immovable = egbase.descriptions().get_immovable_descr(
		   egbase.descriptions().immovable_index(immovable_name));
		assert(immovable != nullptr);
		to_lua<LuaImmovableDescription>(L, new LuaImmovableDescription(immovable));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: created_bobs

      (RO) An :class:`array` of :class:`MapObjectDescription` containing the bobs that
      this building will place on the map.
      For example, a Gamekeepers's Hut will create some critters, and the Shipyard a Ship.
*/
int LuaProductionSiteDescription::get_created_bobs(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const std::string& bobname : get()->created_bobs()) {
		lua_pushint32(L, index++);
		const Widelands::CritterDescr* critter = egbase.descriptions().get_critter_descr(bobname);
		if (critter != nullptr) {
			to_lua<LuaMapObjectDescription>(
			   L,
			   new LuaMapObjectDescription(dynamic_cast<const Widelands::MapObjectDescr*>(critter)));
		} else {
			const Widelands::ShipDescr* ship =
			   egbase.descriptions().get_ship_descr(egbase.descriptions().ship_index(bobname));
			if (ship != nullptr) {
				to_lua<LuaMapObjectDescription>(
				   L,
				   new LuaMapObjectDescription(dynamic_cast<const Widelands::MapObjectDescr*>(ship)));
			} else {
				const Widelands::WorkerDescr* worker =
				   egbase.descriptions().get_worker_descr(egbase.descriptions().worker_index(bobname));
				if (worker != nullptr) {
					to_lua<LuaWorkerDescription>(L, new LuaWorkerDescription(worker));
				} else {
					report_error(L, "Unknown bob type %s", bobname.c_str());
				}
			}
		}
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: created_resources

      (RO) An :class:`array` of :class:`ResourceDescription` containing the resources that
      this building will place on the map.
      For example, a Fishbreeder's House will create the resource fish.
*/
int LuaProductionSiteDescription::get_created_resources(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	Widelands::EditorGameBase& egbase = get_egbase(L);
	for (const std::string& resource_name : get()->created_resources()) {
		lua_pushint32(L, index++);
		const Widelands::ResourceDescription* resource = egbase.descriptions().get_resource_descr(
		   egbase.descriptions().resource_index(resource_name));
		assert(resource != nullptr);
		to_lua<LuaResourceDescription>(L, new LuaResourceDescription(resource));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: output_ware_types

      (RO) An :class:`array` of :class:`WareDescription` containing the wares that
      the productionsite can produce.
*/
int LuaProductionSiteDescription::get_output_ware_types(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& ware_index : get()->output_ware_types()) {
		lua_pushint32(L, index++);
		const Widelands::WareDescr* descr = get_egbase(L).descriptions().get_ware_descr(ware_index);
		to_lua<LuaWareDescription>(L, new LuaWareDescription(descr));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: output_worker_types

      (RO) An :class:`array` of :class:`WorkerDescription` containing the workers that
      the productionsite can produce.
*/
int LuaProductionSiteDescription::get_output_worker_types(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& worker_index : get()->output_worker_types()) {
		lua_pushint32(L, index++);
		const Widelands::WorkerDescr* descr =
		   get_egbase(L).descriptions().get_worker_descr(worker_index);
		to_lua<LuaWorkerDescription>(L, new LuaWorkerDescription(descr));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: production_programs

      (RO) An :class:`array` with the production program names as string. See
      :ref:`production site programs <productionsite_programs>`.
*/
int LuaProductionSiteDescription::get_production_programs(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& program : get()->programs()) {
		lua_pushint32(L, index++);
		lua_pushstring(L, program.first);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: supported_productionsites

      (RO) An :class:`array` with :class:`ProductionSiteDescription` containing the buildings that
      will collect the bobs, immovables or resources from the map that this building will place on
      it. For example, a Forester's House will support a Woodcutter's House, because it places trees
      on the map.
*/
int LuaProductionSiteDescription::get_supported_productionsites(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	for (const auto& site : get()->supported_productionsites()) {
		lua_pushint32(L, index++);
		const Widelands::ProductionSiteDescr* descr =
		   dynamic_cast<const Widelands::ProductionSiteDescr*>(
		      descriptions.get_building_descr(descriptions.safe_building_index(site)));
		to_lua<LuaProductionSiteDescription>(L, new LuaProductionSiteDescription(descr));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: supported_by_productionsites

      (RO) An :class:`array` with :class:`ProductionSiteDescription` containing the buildings that
      place bobs, immovables or resources on the map that this building will collect.
      For example, a Woodcutter's House is supported by a Forester's House, because it needs trees
      to fell.
*/
int LuaProductionSiteDescription::get_supported_by_productionsites(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	const Widelands::Descriptions& decriptions = get_egbase(L).descriptions();
	for (const auto& site : get()->supported_by_productionsites()) {
		lua_pushint32(L, index++);
		const Widelands::ProductionSiteDescr* descr =
		   dynamic_cast<const Widelands::ProductionSiteDescr*>(
		      decriptions.get_building_descr(decriptions.safe_building_index(site)));
		to_lua<LuaProductionSiteDescription>(L, new LuaProductionSiteDescription(descr));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: working_positions

      (RO) An :class:`array` with :class:`WorkerDescription` containing the workers that
      can work here with their multiplicity, i.e. for an Atlantean mine this
      would be ``{miner,miner,miner}``.
*/
int LuaProductionSiteDescription::get_working_positions(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& positions_pair : get()->working_positions()) {
		int amount = positions_pair.second;
		while (amount > 0) {
			lua_pushint32(L, index++);
			const Widelands::WorkerDescr* descr =
			   get_egbase(L).descriptions().get_worker_descr(positions_pair.first);
			to_lua<LuaWorkerDescription>(L, new LuaWorkerDescription(descr));
			lua_settable(L, -3);
			--amount;
		}
	}
	return 1;
}

/* RST
   .. method:: consumed_wares_workers(program_name)

      Returns an :class:`array` of ``{{ware_name,ware_amount}}`` for the wares consumed by this
      production program. Multiple entries of ``{ware_name,ware_amount}`` are alternatives (OR
      logic)).

      :arg program_name: The name of the production program that we want to get the consumed wares
         for. See :ref:`production site programs <productionsite_programs>`.
      :type program_name: :class:`string`

      E.g. this will return for an Atlantean coalmine and the corresponding program:

      .. code-block:: Lua

         {
           1, {smoked_meat, 2}, {smoked_fish, 2}  -- 2 smoked_meat OR 2 smoked_fish
           2, {atlanteans_bread, 2}               -- AND 2 atlanteans_bread
         }
*/
int LuaProductionSiteDescription::consumed_wares_workers(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const Widelands::ProductionProgram& program = *programs.at(program_name);
		lua_newtable(L);
		int counter = 0;
		for (const auto& group : program.consumed_wares_workers()) {
			lua_pushuint32(L, ++counter);
			lua_newtable(L);
			for (const auto& entry : group.first) {
				const Widelands::DescriptionIndex& index = entry.first;
				if (entry.second == Widelands::wwWARE) {
					lua_pushstring(L, get_egbase(L).descriptions().get_ware_descr(index)->name());
				} else {
					lua_pushstring(L, get_egbase(L).descriptions().get_worker_descr(index)->name());
				}
				lua_pushuint32(L, group.second);
				lua_settable(L, -3);
			}
			lua_settable(L, -3);
		}
	}
	return 1;
}

/* RST
   .. method:: produced_wares(program_name)

      Returns a :class:`table` of ``{ware_name=ware_amount}`` for the wares produced by this
      production program. See :ref:`production site programs <productionsite_programs>`.

      :arg program_name: The name of the production program that we want to get the produced wares
         for.
      :type program_name: :class:`string`

*/
int LuaProductionSiteDescription::produced_wares(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const Widelands::ProductionProgram& program = *programs.at(program_name);
		return wares_or_workers_map_to_lua(
		   L, program.produced_wares(), Widelands::MapObjectType::WARE);
	}
	return 1;
}

/* RST
   .. method:: recruited_workers(program_name)

      Returns a :class:`table` of ``{worker_name=worker_amount}`` for the workers recruited
      by this production program. See :ref:`production site programs <productionsite_programs>`.

      :arg program_name: the name of the production program that we want to get the recruited
         workers for.
      :type program_name: :class:`string`

*/
int LuaProductionSiteDescription::recruited_workers(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const Widelands::ProductionProgram& program = *programs.at(program_name);
		return wares_or_workers_map_to_lua(
		   L, program.recruited_workers(), Widelands::MapObjectType::WORKER);
	}
	return 1;
}

/* RST
MilitarySiteDescription
-----------------------

.. class:: MilitarySiteDescription

   A static description of a tribe's militarysite.

   A militarysite can garrison and heal soldiers, and it will expand your territory.
   See the parent classes for more properties.
*/
const char LuaMilitarySiteDescription::className[] = "MilitarySiteDescription";
const MethodType<LuaMilitarySiteDescription> LuaMilitarySiteDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMilitarySiteDescription> LuaMilitarySiteDescription::Properties[] = {
   PROP_RO(LuaMilitarySiteDescription, heal_per_second),
   PROP_RO(LuaMilitarySiteDescription, max_number_of_soldiers),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: heal_per_second

      (RO) The number of health healed per second by the militarysite.
*/
int LuaMilitarySiteDescription::get_heal_per_second(lua_State* L) {
	lua_pushinteger(L, get()->get_heal_per_second());
	return 1;
}

/* RST
   .. attribute:: max_number_of_soldiers

      (RO) The number of soldiers that can be garrisoned at the militarysite.
*/
int LuaMilitarySiteDescription::get_max_number_of_soldiers(lua_State* L) {
	lua_pushinteger(L, get()->get_max_number_of_soldiers());
	return 1;
}

/* RST
TrainingSiteDescription
-----------------------

.. class:: TrainingSiteDescription

   A static description of a tribe's trainingsite.

   A training site can train some or all of a soldier's properties (attack, defense, evade and
   health). See the parent classes for more properties.
*/
const char LuaTrainingSiteDescription::className[] = "TrainingSiteDescription";
const MethodType<LuaTrainingSiteDescription> LuaTrainingSiteDescription::Methods[] = {
   METHOD(LuaTrainingSiteDescription, trained_soldiers),
   {nullptr, nullptr},
};
const PropertyType<LuaTrainingSiteDescription> LuaTrainingSiteDescription::Properties[] = {
   PROP_RO(LuaTrainingSiteDescription, max_attack),
   PROP_RO(LuaTrainingSiteDescription, max_defense),
   PROP_RO(LuaTrainingSiteDescription, max_evade),
   PROP_RO(LuaTrainingSiteDescription, max_health),
   PROP_RO(LuaTrainingSiteDescription, max_number_of_soldiers),
   PROP_RO(LuaTrainingSiteDescription, min_attack),
   PROP_RO(LuaTrainingSiteDescription, min_defense),
   PROP_RO(LuaTrainingSiteDescription, min_evade),
   PROP_RO(LuaTrainingSiteDescription, min_health),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: max_attack

      (RO) The number of attack points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_attack(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_attack()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kAttack));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_defense

      (RO) The number of defense points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_defense(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_defense()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kDefense));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_evade

      (RO) The number of evade points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_evade(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_evade()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kEvade));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_health

      (RO) The number of health points that a soldier can train.
*/
int LuaTrainingSiteDescription::get_max_health(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_health()) {
		lua_pushinteger(L, descr->get_max_level(Widelands::TrainingAttribute::kHealth));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_number_of_soldiers

      (RO) The number of soldiers that can be garrisoned at the trainingsite.
*/
int LuaTrainingSiteDescription::get_max_number_of_soldiers(lua_State* L) {
	lua_pushinteger(L, get()->get_max_number_of_soldiers());
	return 1;
}

/* RST
   .. attribute:: min_attack

      (RO) The number of attack points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_attack(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_attack()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kAttack));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_defense

      (RO) The number of defense points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_defense(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_defense()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kDefense));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_evade

      (RO) The number of evade points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_evade(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_evade()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kEvade));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_health

      (RO) The number of health points that a soldier starts training with.
*/
int LuaTrainingSiteDescription::get_min_health(lua_State* L) {
	const Widelands::TrainingSiteDescr* descr = get();
	if (descr->get_train_health()) {
		lua_pushinteger(L, descr->get_min_level(Widelands::TrainingAttribute::kHealth));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. method:: trained_soldiers(program_name)

Returns a :class:`table` with following entries [1] = the trained skill, [2] = the starting level,
      [3] = the resulting level trained by this production program.
      See :ref:`production site programs <productionsite_programs>`.

      :arg program_name: the name of the production program that we want to get the trained
         soldiers for.
      :type program_name: :class:`string`

*/
int LuaTrainingSiteDescription::trained_soldiers(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const Widelands::ProductionProgram& program = *programs.at(program_name);
		lua_newtable(L);
		lua_pushint32(L, 1);
		lua_pushstring(L, program.trained_attribute());
		lua_settable(L, -3);
		lua_pushint32(L, 2);
		lua_pushint32(L, program.train_from_level());
		lua_settable(L, -3);
		lua_pushint32(L, 3);
		lua_pushint32(L, program.train_to_level());
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
WarehouseDescription
--------------------

.. class:: WarehouseDescription

   A static description of a tribe's warehouse. Note that headquarters are also warehouses.
   A warehouse keeps people, animals and wares. See the parent classes for more properties.
*/
const char LuaWarehouseDescription::className[] = "WarehouseDescription";
const MethodType<LuaWarehouseDescription> LuaWarehouseDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaWarehouseDescription> LuaWarehouseDescription::Properties[] = {
   PROP_RO(LuaWarehouseDescription, heal_per_second),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: heal_per_second

      (RO) The number of health healed per second by the warehouse.
*/
int LuaWarehouseDescription::get_heal_per_second(lua_State* L) {
	lua_pushinteger(L, get()->get_heal_per_second());
	return 1;
}

// TODO(kaputtnik): Readd RST once this get fully implemented
/*
MarketDescription
-----------------

.. class:: MarketDescription

   A static description of a tribe's market. A Market is used for
   trading over land with other players. See the parent classes for more
   properties.
*/
// TODO(sirver,trading): Expose the properties of MarketDescription here once
// the interface settles.
const char LuaMarketDescription::className[] = "MarketDescription";
const MethodType<LuaMarketDescription> LuaMarketDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMarketDescription> LuaMarketDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
ShipDescription
-----------------

.. class:: ShipDescription

   A static description of a tribe's ship. See also
   :class:`MapObjectDescription` for more properties.
*/
const char LuaShipDescription::className[] = "ShipDescription";
const MethodType<LuaShipDescription> LuaShipDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaShipDescription> LuaShipDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaShipDescription::__persist(lua_State* L) {
	const Widelands::ShipDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaShipDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_ship_index(name);
	set_description_pointer(descriptions.get_ship_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
WareDescription
---------------

.. class:: WareDescription

   A static description of a ware. See the parent class for more properties.
*/
const char LuaWareDescription::className[] = "WareDescription";
const MethodType<LuaWareDescription> LuaWareDescription::Methods[] = {
   METHOD(LuaWareDescription, consumers),
   METHOD(LuaWareDescription, is_construction_material),
   METHOD(LuaWareDescription, producers),
   {nullptr, nullptr},
};
const PropertyType<LuaWareDescription> LuaWareDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaWareDescription::__persist(lua_State* L) {
	const Widelands::WareDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaWareDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	Widelands::DescriptionIndex idx = descriptions.safe_ware_index(name);
	set_description_pointer(descriptions.get_ware_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. method:: consumers(tribename)

      (RO) Returns an :class:`array` with :class:`~wl.map.BuildingDescription` with buildings that
      need this ware for their production. Loads the tribe if it hasn't been loaded yet.

      :arg tribename: The name of the tribe that this ware gets checked for.
      :type tribename: :class:`string`

*/
int LuaWareDescription::consumers(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::TribeDescr* tribe = get_tribe_descr(L, luaL_checkstring(L, 2));

	lua_newtable(L);
	int index = 1;
	for (const Widelands::DescriptionIndex& building_index : get()->consumers()) {
		if (tribe->has_building(building_index)) {
			lua_pushint32(L, index++);
			upcasted_map_object_descr_to_lua(
			   L, get_egbase(L).descriptions().get_building_descr(building_index));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
   .. method:: is_construction_material(tribename)

      (RO) Returns :const:`true` if this ware is used by the tribe's construction sites.

      :arg tribename: The name of the tribe that this ware gets checked for.
      :type tribename: :class:`string`

*/
int LuaWareDescription::is_construction_material(lua_State* L) {
	std::string tribename = luaL_checkstring(L, -1);
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	if (descriptions.tribe_exists(tribename)) {
		const Widelands::DescriptionIndex& ware_index = descriptions.safe_ware_index(get()->name());
		int tribeindex = descriptions.tribe_index(tribename);
		lua_pushboolean(
		   L, static_cast<int>(
		         descriptions.get_tribe_descr(tribeindex)->is_construction_material(ware_index)));
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

/* RST
   .. method:: producers(tribename)

      (RO) Returns an :class:`array` with :class:`BuildingDescription` with buildings that
      can produce this ware. Loads the tribe if it hasn't been loaded yet.

      :arg tribename: The name of the tribe that this ware gets checked for.
      :type tribename: :class:`string`

*/
int LuaWareDescription::producers(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::TribeDescr* tribe = get_tribe_descr(L, luaL_checkstring(L, 2));

	lua_newtable(L);
	int index = 1;
	for (const Widelands::DescriptionIndex& building_index : get()->producers()) {
		if (tribe->has_building(building_index)) {
			lua_pushint32(L, index++);
			upcasted_map_object_descr_to_lua(
			   L, get_egbase(L).descriptions().get_building_descr(building_index));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

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

/* RST
SoldierDescription
--------------------

.. class:: SoldierDescription

   A static description of a tribe's soldier, so it can be used in help files
   without having to access an actual instance of the worker on the map.
   See the parent classes for more properties.
*/
const char LuaSoldierDescription::className[] = "SoldierDescription";
const MethodType<LuaSoldierDescription> LuaSoldierDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaSoldierDescription> LuaSoldierDescription::Properties[] = {
   PROP_RO(LuaSoldierDescription, max_health_level),
   PROP_RO(LuaSoldierDescription, max_attack_level),
   PROP_RO(LuaSoldierDescription, max_defense_level),
   PROP_RO(LuaSoldierDescription, max_evade_level),
   PROP_RO(LuaSoldierDescription, base_health),
   PROP_RO(LuaSoldierDescription, base_min_attack),
   PROP_RO(LuaSoldierDescription, base_max_attack),
   PROP_RO(LuaSoldierDescription, base_defense),
   PROP_RO(LuaSoldierDescription, base_evade),
   PROP_RO(LuaSoldierDescription, health_incr_per_level),
   PROP_RO(LuaSoldierDescription, attack_incr_per_level),
   PROP_RO(LuaSoldierDescription, defense_incr_per_level),
   PROP_RO(LuaSoldierDescription, evade_incr_per_level),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: max_health_level

      (RO) The maximum health level that the soldier can have.
*/
int LuaSoldierDescription::get_max_health_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_health_level());
	return 1;
}

/* RST
   .. attribute:: max_attack_level

      (RO) The maximum attack level that the soldier can have.
*/
int LuaSoldierDescription::get_max_attack_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_attack_level());
	return 1;
}

/* RST
   .. attribute:: max_defense_level

      (RO) The maximum defense level that the soldier can have.
*/
int LuaSoldierDescription::get_max_defense_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_defense_level());
	return 1;
}

/* RST
   .. attribute:: max_evade_level

      (RO) The maximum evade level that the soldier can have.
*/
int LuaSoldierDescription::get_max_evade_level(lua_State* L) {
	lua_pushinteger(L, get()->get_max_evade_level());
	return 1;
}

/* RST
   .. attribute:: base_health

      (RO) The health points that the soldier starts with
*/
int LuaSoldierDescription::get_base_health(lua_State* L) {
	lua_pushinteger(L, get()->get_base_health());
	return 1;
}

/* RST
   .. attribute:: base_min_attack

      (RO) The minimum random attack points that get added to a soldier's attack
*/
int LuaSoldierDescription::get_base_min_attack(lua_State* L) {
	lua_pushinteger(L, get()->get_base_min_attack());
	return 1;
}

/* RST
   .. attribute:: base_max_attack

      (RO) The maximum random attack points that get added to a soldier's attack
*/
int LuaSoldierDescription::get_base_max_attack(lua_State* L) {
	lua_pushinteger(L, get()->get_base_max_attack());
	return 1;
}

/* RST
   .. attribute:: base_defense

      (RO) The defense % that the soldier starts with
*/
int LuaSoldierDescription::get_base_defense(lua_State* L) {
	lua_pushinteger(L, get()->get_base_defense());
	return 1;
}

/* RST
   .. attribute:: base_evade

      (RO) The evade % that the soldier starts with
*/
int LuaSoldierDescription::get_base_evade(lua_State* L) {
	lua_pushinteger(L, get()->get_base_evade());
	return 1;
}

/* RST
   .. attribute:: health_incr_per_level

      (RO) The health points that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_health_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_health_incr_per_level());
	return 1;
}

/* RST
   .. attribute:: attack_incr_per_level

      (RO) The attack points that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_attack_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_attack_incr_per_level());
	return 1;
}

/* RST
   .. attribute:: defense_incr_per_level

      (RO) The defense % that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_defense_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_defense_incr_per_level());
	return 1;
}

/* RST
   .. attribute:: evade_incr_per_level

      (RO) The evade % that the soldier will gain with each level.
*/
int LuaSoldierDescription::get_evade_incr_per_level(lua_State* L) {
	lua_pushinteger(L, get()->get_evade_incr_per_level());
	return 1;
}

/* RST
ResourceDescription
--------------------
.. class:: ResourceDescription

   A static description of a resource.
*/
const char LuaResourceDescription::className[] = "ResourceDescription";
const MethodType<LuaResourceDescription> LuaResourceDescription::Methods[] = {
   METHOD(LuaResourceDescription, editor_image),
   {nullptr, nullptr},
};
const PropertyType<LuaResourceDescription> LuaResourceDescription::Properties[] = {
   PROP_RO(LuaResourceDescription, name),
   PROP_RO(LuaResourceDescription, descname),
   PROP_RO(LuaResourceDescription, is_detectable),
   PROP_RO(LuaResourceDescription, max_amount),
   PROP_RO(LuaResourceDescription, representative_image),
   {nullptr, nullptr, nullptr},
};

void LuaResourceDescription::__persist(lua_State* L) {
	const Widelands::ResourceDescription* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaResourceDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Widelands::Descriptions& descriptions = get_egbase(L).descriptions();
	const Widelands::ResourceDescription* descr =
	   descriptions.get_resource_descr(descriptions.safe_resource_index(name));
	set_description_pointer(descr);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: name

      (RO) The internal name of this resource as :class:`string`.
*/

int LuaResourceDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: descname

      (RO) The display name of this resource as :class:`string`.
*/

int LuaResourceDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: is_detectable

      (RO) :const:`true` if geologists can find this resource.
*/

int LuaResourceDescription::get_is_detectable(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get()->detectable()));
	return 1;
}

/* RST
   .. attribute:: max_amount

      (RO) The maximum amount of this resource that a terrain can have.
*/

int LuaResourceDescription::get_max_amount(lua_State* L) {
	lua_pushinteger(L, get()->max_amount());
	return 1;
}

/* RST
   .. attribute:: representative_image

      (RO) The path to the image representing this resource in the GUI as :class:`string`.
*/
int LuaResourceDescription::get_representative_image(lua_State* L) {
	lua_pushstring(L, get()->representative_image());
	return 1;
}

/*
 ==========================================================
 METHODS
 ==========================================================
 */

/* RST
   .. method:: editor_image(amount)

      (RO) The path to the image representing the specified amount of this
      resource as :class:`string`.

      :arg amount: The amount of the resource what we want an overlay image for.
      :type amount: :class:`integer`

*/
int LuaResourceDescription::editor_image(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const uint32_t amount = luaL_checkuint32(L, 2);
	lua_pushstring(L, get()->editor_image(amount));
	return 1;
}

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

/* RST
Economy
-------
.. class:: Economy

   Provides access to an economy. An economy will be created each time a player places a
   flag on the map. As soon this flag is connected to another flag, their two economies will
   be merged into a single economy. A player can have multiple economies, each of which has
   its own set of economy target settings.

   You can get an economy from a :class:`Flag`.
*/
const char LuaEconomy::className[] = "Economy";
const MethodType<LuaEconomy> LuaEconomy::Methods[] = {
   METHOD(LuaEconomy, target_quantity),
   METHOD(LuaEconomy, set_target_quantity),
   METHOD(LuaEconomy, needs),
   {nullptr, nullptr},
};
const PropertyType<LuaEconomy> LuaEconomy::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaEconomy::__persist(lua_State* L) {
	const Widelands::Economy* economy = get();
	const Widelands::Player& player = economy->owner();
	PERS_UINT32("player", player.player_number());
	PERS_UINT32("economy", economy->serial());
}

void LuaEconomy::__unpersist(lua_State* L) {
	Widelands::PlayerNumber player_number;
	Widelands::Serial economy_serial;
	UNPERS_UINT32("player", player_number)
	UNPERS_UINT32("economy", economy_serial)
	const Widelands::Player& player = get_egbase(L).player(player_number);
	set_economy_pointer(player.get_economy(economy_serial));
}

/* RST
   .. method:: target_quantity(name)

      Returns the amount of the given ware or worker that should be kept in stock for this economy.
      Whether this works only for wares or only for workers is determined by the type of this
      economy.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg name: The name of the ware or worker.
      :type name: :class:`string`
      :returns: :class:`integer`
*/
int LuaEconomy::target_quantity(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(wname);
		if (get_egbase(L).descriptions().ware_exists(index)) {
			const Widelands::Economy::TargetQuantity& quantity = get()->target_quantity(index);
			lua_pushinteger(L, quantity.permanent);
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(wname);
		if (get_egbase(L).descriptions().worker_exists(index)) {
			const Widelands::Economy::TargetQuantity& quantity = get()->target_quantity(index);
			lua_pushinteger(L, quantity.permanent);
		} else {
			report_error(L, "There is no worker '%s'.", wname.c_str());
		}
		break;
	}
	}
	return 1;
}

/* RST
   .. method:: set_target_quantity(name, amount)

      Sets the amount of the given ware or worker type that should be kept in stock for this
      economy. Whether this works only for wares or only for workers is determined by the type of
      this economy.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg workername: The name of the worker type.
      :type workername: :class:`string`

      :arg amount: The new target amount for the worker. Needs to be ``>=0``.
      :type amount: :class:`integer`
*/
int LuaEconomy::set_target_quantity(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(wname);
		if (get_egbase(L).descriptions().ware_exists(index)) {
			const int quantity = luaL_checkinteger(L, 3);
			if (quantity < 0) {
				report_error(L, "Target ware quantity needs to be >= 0 but was '%d'.", quantity);
			}
			get()->set_target_quantity(get()->type(), index, quantity, get_egbase(L).get_gametime());
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(wname);
		if (get_egbase(L).descriptions().worker_exists(index)) {
			const int quantity = luaL_checkinteger(L, 3);
			if (quantity < 0) {
				report_error(L, "Target worker quantity needs to be >= 0 but was '%d'.", quantity);
			}
			get()->set_target_quantity(get()->type(), index, quantity, get_egbase(L).get_gametime());
		} else {
			report_error(L, "There is no worker '%s'.", wname.c_str());
		}
		break;
	}
	}
	return 0;
}

/* RST
   .. method:: needs(name)

      Check whether the economy's stock of the given
      ware or worker is lower than the target setting.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg name: The name of the ware or worker.
      :type name: :class:`string`
      :returns: :class:`boolean`
*/
int LuaEconomy::needs(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().ware_index(wname);
		if (get_egbase(L).descriptions().ware_exists(index)) {
			lua_pushboolean(L, static_cast<int>(get()->needs_ware_or_worker(index)));
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).descriptions().worker_index(wname);
		if (get_egbase(L).descriptions().worker_exists(index)) {
			lua_pushboolean(L, static_cast<int>(get()->needs_ware_or_worker(index)));
		} else {
			report_error(L, "There is no worker '%s'.", wname.c_str());
		}
		break;
	}
	}
	return 1;
}

/* RST
MapObject
---------

.. class:: MapObject

   This is the base class for all objects in Widelands, including
   :class:`immovables <BaseImmovable>` and :class:`bobs <Bob>`. This
   class can't be instantiated directly, but provides the base for all
   others.
*/
const char LuaMapObject::className[] = "MapObject";
const MethodType<LuaMapObject> LuaMapObject::Methods[] = {
   METHOD(LuaMapObject, remove), METHOD(LuaMapObject, destroy),
   METHOD(LuaMapObject, __eq),   METHOD(LuaMapObject, has_attribute),
   {nullptr, nullptr},
};
const PropertyType<LuaMapObject> LuaMapObject::Properties[] = {
   PROP_RO(LuaMapObject, __hash), PROP_RO(LuaMapObject, descr), PROP_RO(LuaMapObject, serial),
   PROP_RO(LuaMapObject, exists), {nullptr, nullptr, nullptr},
};

void LuaMapObject::__persist(lua_State* L) {
	Widelands::MapObjectSaver& mos = *get_mos(L);
	Widelands::Game& game = get_game(L);

	uint32_t idx = 0;
	if (Widelands::MapObject* obj = ptr_.get(game)) {
		idx = mos.get_object_file_index(*obj);
	}

	PERS_UINT32("file_index", idx);
}
void LuaMapObject::__unpersist(lua_State* L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx)

	if (idx == 0u) {
		ptr_ = nullptr;
	} else {
		Widelands::MapObjectLoader& mol = *get_mol(L);
		ptr_ = &mol.get<Widelands::MapObject>(idx);
	}
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: __hash

      (RO) The map object's serial. Used to identify a class in a Set.
*/
int LuaMapObject::get___hash(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->serial());
	return 1;
}

/* RST
   .. attribute:: serial

      (RO) The serial number of this object. Note that this value does not stay
      constant after saving/loading.
*/
int LuaMapObject::get_serial(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->serial());
	return 1;
}

/* RST
   .. attribute:: descr

      (RO) The :class:`MapObjectDescription` for this immovable.

      .. code-block:: lua

         local immovable = wl.Game().map:get_field(20,31).immovable

         -- always check if the immovable was found on the field
         if immovable then
            if immovable.descr.type_name == "warehouse"  -- access MapObjectDescription
               immovable:set_wares("log", 5)
            end
         end

*/
int LuaMapObject::get_descr(lua_State* L) {
	const Widelands::MapObjectDescr* desc = &get(L, get_egbase(L))->descr();
	assert(desc != nullptr);

	return upcasted_map_object_descr_to_lua(L, desc);
}

/* RST
   .. attribute:: exists

      .. versionadded:: 1.2

      (RO) Whether the map object represented by this Lua object still exists.

      If it does not exist, no other attributes or functions of this object may be accessed.
*/
int LuaMapObject::get_exists(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get_or_zero(get_egbase(L)) != nullptr));
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaMapObject::__eq(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	LuaMapObject* other = *get_base_user_class<LuaMapObject>(L, -1);

	Widelands::MapObject* me = get_or_zero(egbase);
	Widelands::MapObject* you = other->get_or_zero(egbase);

	// Both objects are destroyed (nullptr) or equal: they are equal
	if (me == you) {
		lua_pushboolean(L, 1);
	} else if (me == nullptr ||
	           you == nullptr) {  // One of the objects is destroyed: they are distinct
		lua_pushboolean(L, 0);
	} else {  // Compare their serial number.
		lua_pushboolean(
		   L, static_cast<int>(other->get(L, egbase)->serial() == get(L, egbase)->serial()));
	}

	return 1;
}

/* RST
   .. method:: remove()

      Removes this object immediately. If you want to destroy an
      object as if the player had see :func:`destroy`.
*/
int LuaMapObject::remove(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::MapObject* o = get(L, egbase);
	if (o == nullptr) {
		return 0;
	}

	o->remove(egbase);
	return 0;
}

/* RST
   .. method:: destroy()

      Removes this object immediately. Might do special actions (like leaving a
      burning fire). If you want to remove an object without side effects, see
      :func:`remove`.
*/
int LuaMapObject::destroy(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::MapObject* o = get(L, egbase);
	if (o == nullptr) {
		return 0;
	}

	o->destroy(egbase);
	return 0;
}

/* RST
   .. method:: has_attribute(attribute)

      Returns :const:`true` if the map object has this attribute, :const:`false` otherwise.

      :arg attribute: The attribute to check for.
      :type attribute: :class:`string`
*/
int LuaMapObject::has_attribute(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::MapObject* obj = get_or_zero(egbase);
	if (obj == nullptr) {
		lua_pushboolean(L, 0);
		return 1;
	}

	// Check if object has the attribute
	std::string attrib = luaL_checkstring(L, 2);
	if (obj->has_attribute(Widelands::MapObjectDescr::get_attribute_id(attrib))) {
		lua_pushboolean(L, 1);
	} else {
		lua_pushboolean(L, 0);
	}
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Widelands::MapObject*
LuaMapObject::get(lua_State* L, Widelands::EditorGameBase& egbase, const std::string& name) {
	Widelands::MapObject* o = get_or_zero(egbase);
	if (o == nullptr) {
		report_error(L, "%s no longer exists!", name.c_str());
	}
	return o;
}
Widelands::MapObject* LuaMapObject::get_or_zero(const Widelands::EditorGameBase& egbase) {
	return ptr_.get(egbase);
}

/* RST
BaseImmovable
-------------

.. class:: BaseImmovable

   This is the base class for all immovables in Widelands.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :attr:`MapObject.descr`.
*/
const char LuaBaseImmovable::className[] = "BaseImmovable";
const MethodType<LuaBaseImmovable> LuaBaseImmovable::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaBaseImmovable> LuaBaseImmovable::Properties[] = {
   PROP_RO(LuaBaseImmovable, fields),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: fields

      (RO) An :class:`array` of :class:`wl.map.Field` that is occupied by this
      Immovable. If the immovable occupies more than one field (roads or big
      buildings for example) the first entry in this list will be the main field.
*/
int LuaBaseImmovable::get_fields(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);

	Widelands::BaseImmovable::PositionList pl = get(L, egbase)->get_positions(egbase);

	lua_createtable(L, pl.size(), 0);
	uint32_t idx = 1;
	for (const Widelands::Coords& coords : pl) {
		lua_pushuint32(L, idx++);
		to_lua<LuaField>(L, new LuaField(coords.x, coords.y));
		lua_rawset(L, -3);
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
PlayerImmovable
---------------

.. class:: PlayerImmovable

   All Immovables that belong to a Player (Buildings, Flags, ...) are based on
   this Class.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaPlayerImmovable::className[] = "PlayerImmovable";
const MethodType<LuaPlayerImmovable> LuaPlayerImmovable::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPlayerImmovable> LuaPlayerImmovable::Properties[] = {
   PROP_RO(LuaPlayerImmovable, owner),
   PROP_RO(LuaPlayerImmovable, debug_ware_economy),
   PROP_RO(LuaPlayerImmovable, debug_worker_economy),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: owner

      (RO) The :class:`wl.game.Player` who owns this object.
*/
int LuaPlayerImmovable::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

// UNTESTED, for debug only
int LuaPlayerImmovable::get_debug_ware_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(Widelands::wwWARE));
	return 1;
}
int LuaPlayerImmovable::get_debug_worker_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(Widelands::wwWORKER));
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Flag
--------

.. class:: Flag

   One flag in the economy of this Player.

   See also: :ref:`has_wares`.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaFlag::className[] = "Flag";
const MethodType<LuaFlag> LuaFlag::Methods[] = {
   METHOD(LuaFlag, set_wares),      METHOD(LuaFlag, get_wares), METHOD(LuaFlag, get_distance),
   METHOD(LuaFlag, send_geologist), {nullptr, nullptr},
};
const PropertyType<LuaFlag> LuaFlag::Properties[] = {
   PROP_RO(LuaFlag, ware_economy), PROP_RO(LuaFlag, worker_economy), PROP_RO(LuaFlag, roads),
   PROP_RO(LuaFlag, building),     {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: ware_economy

      (RO) Returns the ware economy that this flag belongs to.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you call another function on the resulting economy object.

      :returns: The :class:`Economy` associated with the flag to handle wares.
*/
int LuaFlag::get_ware_economy(lua_State* L) {
	const Widelands::Flag* f = get(L, get_egbase(L));
	return to_lua<LuaEconomy>(L, new LuaEconomy(f->get_economy(Widelands::wwWARE)));
}

/* RST
   .. attribute:: worker_economy

      (RO) Returns the worker economy that this flag belongs to.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you call another function on the resulting economy object.

      :returns: The :class:`Economy` associated with the flag to handle workers.
*/
int LuaFlag::get_worker_economy(lua_State* L) {
	const Widelands::Flag* f = get(L, get_egbase(L));
	return to_lua<LuaEconomy>(L, new LuaEconomy(f->get_economy(Widelands::wwWORKER)));
}

/* RST
   .. attribute:: roads

      (RO) The roads which are connected to this flag, if any.

      Note that waterways are currently treated like roads.

      :returns:  A :class:`table` with directions as keys. Directions can be
         ``"tr"``, ``"r"``, ``"br"``, ``"bl"``, ``"l"`` and ``"tl"``. If this flag has no roads,
         the :class:`table` will be empty.
*/
int LuaFlag::get_roads(lua_State* L) {

	const std::vector<std::string> directions = {"tr", "r", "br", "bl", "l", "tl"};

	lua_newtable(L);

	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Flag* f = get(L, egbase);

	for (uint32_t i = 1; i <= 6; i++) {
		if (f->get_roadbase(i) != nullptr) {
			lua_pushstring(L, directions.at(i - 1));
			upcasted_map_object_to_lua(L, f->get_roadbase(i));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
   .. attribute:: building

      (RO) The building belonging to the flag, if any.
*/
int LuaFlag::get_building(lua_State* L) {

	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Flag* f = get(L, egbase);

	Widelands::PlayerImmovable* building = f->get_building();
	if (building == nullptr) {
		return 0;
	}
	upcasted_map_object_to_lua(L, building);

	return 1;
}
/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
// Documented in ParentClass
int LuaFlag::set_wares(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Flag* f = get(L, egbase);
	const Widelands::Descriptions& descriptions = egbase.descriptions();

	InputMap setpoints;
	parse_wares_workers_counted(L, f->owner().tribe(), &setpoints, true);
	WaresWorkersMap c_wares = count_wares_on_flag_(*f, descriptions);

	for (const auto& ware : c_wares) {
		// all wares currently on the flag without a setpoint should be removed
		if (setpoints.count(std::make_pair(ware.first, Widelands::WareWorker::wwWARE)) == 0u) {
			setpoints.insert(
			   std::make_pair(std::make_pair(ware.first, Widelands::WareWorker::wwWARE), 0));
		}
	}

	// When the wares on the flag increase, we do it at the end to avoid exceeding the capacity
	std::map<Widelands::DescriptionIndex, int> wares_to_add;

	// The idea is to change as little as possible on this flag
	for (const auto& sp : setpoints) {
		int diff = sp.second;

		const Widelands::DescriptionIndex& index = sp.first.first;
		WaresWorkersMap::iterator i = c_wares.find(index);
		if (i != c_wares.end()) {
			diff -= i->second;
		}

		if (diff > 0) {
			// add wares later
			wares_to_add.insert(std::make_pair(index, diff));
		} else {
			while (diff < 0) {
				for (const Widelands::WareInstance* ware : f->get_wares()) {
					if (descriptions.ware_index(ware->descr().name()) == index) {
						const_cast<Widelands::WareInstance*>(ware)->remove(egbase);
						++diff;
						break;
					}
				}
			}
		}
	}

	// Now that wares to be removed have gone, we add the remaining wares
	for (const auto& ware_to_add : wares_to_add) {
		if (f->total_capacity() < f->current_wares() + ware_to_add.second) {
			report_error(L, "Flag has no capacity left!");
		}

		const Widelands::WareDescr& wd = *descriptions.get_ware_descr(ware_to_add.first);
		for (int i = 0; i < ware_to_add.second; i++) {
			Widelands::WareInstance& ware = *new Widelands::WareInstance(ware_to_add.first, &wd);
			ware.init(egbase);
			f->add_ware(egbase, ware);
		}
	}

#ifndef NDEBUG
	WaresWorkersMap wares_on_flag = count_wares_on_flag_(*f, descriptions);
	for (const auto& sp : setpoints) {
		if (sp.second > 0) {
			assert(wares_on_flag.count(sp.first.first) == 1);
			assert(wares_on_flag.at(sp.first.first) == sp.second);
		}
	}
#endif

	return 0;
}

// Documented in parent Class
int LuaFlag::get_wares(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::Descriptions& descriptions = egbase.descriptions();
	Widelands::Flag* flag = get(L, egbase);
	const Widelands::TribeDescr& tribe = flag->owner().tribe();

	Widelands::DescriptionIndex ware_index = Widelands::INVALID_INDEX;
	std::vector<Widelands::DescriptionIndex> ware_list;
	RequestedWareWorker parse_output =
	   parse_wares_workers_list(L, tribe, &ware_index, &ware_list, true);

	WaresWorkersMap wares = count_wares_on_flag_(*flag, descriptions);

	// Here we create the output - either a single integer of table of pairs
	if (ware_index != Widelands::INVALID_INDEX) {
		uint32_t wares_here = 0;
		if (wares.count(ware_index) != 0u) {
			wares_here = wares[ware_index];
		}
		lua_pushuint32(L, wares_here);
	} else {
		// or it is a list of wares
		assert(!ware_list.empty());
		lua_newtable(L);
		// iterating over list of queryied wares
		for (auto idx : ware_list) {

			uint32_t cnt = 0;
			if (wares.count(idx) != 0u) {
				cnt = wares[idx];
			}
			// the information is pushed if count > 0, or the ware was explicitely asked for
			if (parse_output != RequestedWareWorker::kAll || cnt > 0) {
				lua_pushstring(L, tribe.get_ware_descr(idx)->name());
				lua_pushuint32(L, cnt);
				lua_settable(L, -3);
			}
		}
	}

	return 1;
}

/* RST
   .. method:: get_distance(flag)

      Returns the distance of the specified flag from this flag by roads and/or ships. More
      precisely, this is the time that a worker will need to get from this flag to the other
      flag using roads.

      Note that the distance from A to B is not necessarily equal to the distance from B to A.

      :arg flag: The flag to find.
      :type flag: :class:`Flag`

      :returns: The distance of the flags in walking time or ``nil`` if no path exists.
*/
int LuaFlag::get_distance(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Flag* f1 = get(L, egbase);
	Widelands::Flag* f2 = (*get_user_class<LuaMaps::LuaFlag>(L, 2))->get(L, egbase);
	if (f1->get_economy(Widelands::wwWORKER) == f2->get_economy(Widelands::wwWORKER)) {
		Widelands::Route r;
		if (f1->get_economy(Widelands::wwWORKER)->find_route(*f1, *f2, &r)) {
			lua_pushint32(L, r.get_totalcost());
		} else {
			report_error(
			   L, "Unable to discover the walking-time between two flags within one economy!");
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. method:: send_geologist()

      Send a geologist to explore the surroundings of this flag.
*/
int LuaFlag::send_geologist(lua_State* L) {
	Widelands::Flag& f = *get(L, get_egbase(L));
	f.get_owner()->flagaction(f, Widelands::FlagJob::Type::kGeologist);
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

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
	return do_set_workers<LuaRoad>(L, road, get_valid_workers_for(*road));
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

/* RST
PortDock
--------

.. class:: PortDock

   Each :class:`Warehouse` that is a port has a dock attached to
   it. The PortDock is an immovable that also occupies a field on
   the water near the port.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaPortDock::className[] = "PortDock";
const MethodType<LuaPortDock> LuaPortDock::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPortDock> LuaPortDock::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Building
--------

.. class:: Building

   This represents a building owned by a player.

   More properties are available through this object's
   :class:`BuildingDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaBuilding::className[] = "Building";
const MethodType<LuaBuilding> LuaBuilding::Methods[] = {
   METHOD(LuaBuilding, dismantle),
   METHOD(LuaBuilding, enhance),
   {nullptr, nullptr},
};
const PropertyType<LuaBuilding> LuaBuilding::Properties[] = {
   PROP_RO(LuaBuilding, flag),
   PROP_RW(LuaBuilding, destruction_blocked),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: flag

      (RO) The flag that belongs to this building (that is to the bottom right
      of it's main location).
*/
// UNTESTED
int LuaBuilding::get_flag(lua_State* L) {
	return upcasted_map_object_to_lua(L, &get(L, get_egbase(L))->base_flag());
}

/* RST
   .. attribute:: destruction_blocked

      (RW) Whether the player is forbidden to dismantle or destroy this building.
*/
int LuaBuilding::get_destruction_blocked(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L))->is_destruction_blocked()));
	return 1;
}
int LuaBuilding::set_destruction_blocked(lua_State* L) {
	get(L, get_egbase(L))->set_destruction_blocked(luaL_checkboolean(L, -1));
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: dismantle([keep_wares = false])

      Instantly turn this building into a dismantlesite.

      :arg keep_wares: Optional: If :const:`false` (default) the wares in this buildings stock
         get destroyed. If :const:`true` the wares in this buildings stock will be preserved.
      :type keep_wares: :const:`bool`
*/
int LuaBuilding::dismantle(lua_State* L) {
	Widelands::Building* bld = get(L, get_egbase(L));
	bld->get_owner()->dismantle_building(bld, lua_gettop(L) > 1 && luaL_checkboolean(L, 2));
	return 0;
}

/* RST
   .. method:: enhance([keep_wares = false])

      .. versionadded:: 1.1

      Instantly enhance this building if there is an enhancement.

      :arg keep_wares: Optional: If :const:`false` (default) the wares in this buildings stock
         get destroyed. If :const:`true` the wares in this buildings stock will be preserved.
      :type keep_wares: :const:`bool`
*/
int LuaBuilding::enhance(lua_State* L) {
	Widelands::Building* bld = get(L, get_egbase(L));
	const Widelands::DescriptionIndex enhancement = bld->descr().enhancement();
	if (enhancement == Widelands::INVALID_INDEX) {
		return 0;
	}
	bld->get_owner()->enhance_building(
	   bld, enhancement, lua_gettop(L) > 1 && luaL_checkboolean(L, 2));
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
ConstructionSite
-----------------

.. class:: ConstructionSite

   A construction site as it appears in the game.

   More properties are available through this object's
   :class:`ConstructionSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaConstructionSite::className[] = "ConstructionSite";
const MethodType<LuaConstructionSite> LuaConstructionSite::Methods[] = {
   METHOD(LuaConstructionSite, get_priority),
   METHOD(LuaConstructionSite, set_priority),
   METHOD(LuaConstructionSite, get_desired_fill),
   METHOD(LuaConstructionSite, set_desired_fill),
   METHOD(LuaConstructionSite, get_setting_warehouse_policy),
   METHOD(LuaConstructionSite, set_setting_warehouse_policy),
   {nullptr, nullptr},
};
const PropertyType<LuaConstructionSite> LuaConstructionSite::Properties[] = {
   PROP_RO(LuaConstructionSite, building),
   PROP_RW(LuaConstructionSite, has_builder),
   PROP_RW(LuaConstructionSite, setting_soldier_capacity),
   PROP_RW(LuaConstructionSite, setting_soldier_preference),
   PROP_RW(LuaConstructionSite, setting_launch_expedition),
   PROP_RW(LuaConstructionSite, setting_stopped),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: building

      (RO) The internal name of the building that is constructed here.
*/
int LuaConstructionSite::get_building(lua_State* L) {
	lua_pushstring(L, get(L, get_egbase(L))->building().name());
	return 1;
}

/* RST
   .. attribute:: has_builder

      (RW) :const:`true` if this constructionsite has a builder. Changing this setting causes the
      worker to be instantly deleted or to be created from thin air.
*/
int LuaConstructionSite::get_has_builder(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L))->builder_.is_set()));
	return 1;
}
int LuaConstructionSite::set_has_builder(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::PartiallyFinishedBuilding& cs = *get(L, egbase);
	const bool arg = luaL_checkboolean(L, -1);
	if (cs.builder_.is_set()) {
		if (!arg) {
			cs.builder_.get(egbase)->remove(egbase);
			cs.builder_ = nullptr;
			cs.set_seeing(false);
			if (upcast(Widelands::Game, g, &egbase)) {
				cs.request_builder(*g);
			}
		}
	} else if (arg) {
		assert(cs.builder_request_);
		delete cs.builder_request_;
		cs.builder_request_ = nullptr;
		Widelands::Worker& w = egbase.descriptions()
		                          .get_worker_descr(cs.owner().tribe().builder())
		                          ->create(egbase, cs.get_owner(), &cs, cs.get_position());
		cs.builder_ = &w;
		if (upcast(Widelands::Game, g, &egbase)) {
			w.start_task_buildingwork(*g);
		}
		cs.set_seeing(true);
	}
	return 0;
}

/* RST
   .. attribute:: setting_launch_expedition

      (RW) Only valid for ports under construction. :const:`true` if an expedition will be launched
      immediately upon completion.
*/
int LuaConstructionSite::get_setting_launch_expedition(lua_State* L) {
	if (upcast(Widelands::WarehouseSettings, ws, get(L, get_egbase(L))->get_settings())) {
		lua_pushboolean(L, static_cast<int>(ws->launch_expedition));
	} else {
		lua_pushnil(L);
	}
	return 1;
}
int LuaConstructionSite::set_setting_launch_expedition(lua_State* L) {
	upcast(Widelands::WarehouseSettings, ws, get(L, get_egbase(L))->get_settings());
	if (ws == nullptr) {
		report_error(L, "This constructionsite will not become a warehouse");
	}
	ws->launch_expedition = luaL_checkboolean(L, -1);
	return 0;
}

/* RST
   .. attribute:: setting_stopped

      (RW) Only valid for productionsites and trainingsites under construction. :const:`true` if
      this building will be initially stopped after completion.
*/
int LuaConstructionSite::get_setting_stopped(lua_State* L) {
	if (upcast(Widelands::ProductionsiteSettings, ps, get(L, get_egbase(L))->get_settings())) {
		lua_pushboolean(L, static_cast<int>(ps->stopped));
	} else {
		lua_pushnil(L);
	}
	return 1;
}
int LuaConstructionSite::set_setting_stopped(lua_State* L) {
	upcast(Widelands::ProductionsiteSettings, ps, get(L, get_egbase(L))->get_settings());
	if (ps == nullptr) {
		report_error(L, "This constructionsite will not become a productionsite");
	}
	ps->stopped = luaL_checkboolean(L, -1);
	return 0;
}

/* RST
   .. attribute:: setting_soldier_preference

      (RW) Only valid for militarysites under construction. ``"heroes"`` if this site will prefer
      heroes after completion; ``"rookies"`` for rookies; ``"any"`` for no predilection.
*/
int LuaConstructionSite::get_setting_soldier_preference(lua_State* L) {
	if (upcast(Widelands::MilitarysiteSettings, ms, get(L, get_egbase(L))->get_settings())) {
		lua_pushstring(L, soldier_preference_to_string(ms->soldier_preference).c_str());
	} else {
		lua_pushnil(L);
	}
	return 1;
}
int LuaConstructionSite::set_setting_soldier_preference(lua_State* L) {
	upcast(Widelands::MilitarysiteSettings, ms, get(L, get_egbase(L))->get_settings());
	if (ms == nullptr) {
		report_error(L, "This constructionsite will not become a militarysite");
	}
	try {
		ms->soldier_preference = string_to_soldier_preference(luaL_checkstring(L, -1));
	} catch (const WException& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/* RST
   .. attribute:: setting_soldier_capacity

      (RW) Only valid for militarysites and trainingsites under construction. The desired number of
      soldiers stationed here after completion as :const:`integer`.
*/
int LuaConstructionSite::get_setting_soldier_capacity(lua_State* L) {
	upcast(Widelands::MilitarysiteSettings, ms, get(L, get_egbase(L))->get_settings());
	upcast(Widelands::TrainingsiteSettings, ts, get(L, get_egbase(L))->get_settings());
	assert(!ms || !ts);
	if ((ms == nullptr) && (ts == nullptr)) {
		lua_pushnil(L);
	} else {
		lua_pushuint32(L, ms ? ms->desired_capacity : ts->desired_capacity);
	}
	return 1;
}
int LuaConstructionSite::set_setting_soldier_capacity(lua_State* L) {
	upcast(Widelands::MilitarysiteSettings, ms, get(L, get_egbase(L))->get_settings());
	upcast(Widelands::TrainingsiteSettings, ts, get(L, get_egbase(L))->get_settings());
	assert(!ms || !ts);
	if ((ms == nullptr) && (ts == nullptr)) {
		report_error(
		   L, "This constructionsite will become neither a militarysite nor a trainingsite");
	}
	(ms != nullptr ? ms->desired_capacity : ts->desired_capacity) = luaL_checkuint32(L, -1);
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaConstructionSite::get_priority(lua_State* L) {
	const Widelands::DescriptionIndex item =
	   get_egbase(L).descriptions().safe_ware_index(luaL_checkstring(L, 2));
	if (lua_gettop(L) > 2 && luaL_checkboolean(L, 3)) {
		upcast(const Widelands::ProductionsiteSettings, ps, get(L, get_egbase(L))->get_settings());
		if (ps == nullptr) {
			report_error(L, "This constructionsite will not become a productionsite");
		}
		for (const auto& pair : ps->ware_queues) {
			if (pair.first == item) {
				lua_pushstring(L, priority_to_string(pair.second.priority));
				return 1;
			}
		}
		NEVER_HERE();
	}
	lua_pushstring(
	   L, priority_to_string(get(L, get_egbase(L))->get_priority(Widelands::wwWARE, item)));
	return 1;
}
int LuaConstructionSite::set_priority(lua_State* L) {
	const Widelands::DescriptionIndex item =
	   get_egbase(L).descriptions().safe_ware_index(luaL_checkstring(L, 2));
	if (lua_gettop(L) > 3 && luaL_checkboolean(L, 4)) {
		upcast(Widelands::ProductionsiteSettings, ps, get(L, get_egbase(L))->get_settings());
		if (ps == nullptr) {
			report_error(L, "This constructionsite will not become a productionsite");
		}
		for (auto& pair : ps->ware_queues) {
			if (pair.first == item) {
				try {
					pair.second.priority = string_to_priority(luaL_checkstring(L, 3));
				} catch (const WException& e) {
					report_error(L, "%s", e.what());
				}
				return 0;
			}
		}
		NEVER_HERE();
	}
	try {
		get(L, get_egbase(L))
		   ->set_priority(Widelands::wwWARE, item, string_to_priority(luaL_checkstring(L, 3)));
	} catch (const WException& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}
int LuaConstructionSite::get_desired_fill(lua_State* L) {
	const std::string itemname = luaL_checkstring(L, 2);
	const bool is_ware = get_egbase(L).descriptions().ware_exists(itemname);
	const Widelands::DescriptionIndex item =
	   is_ware ? get_egbase(L).descriptions().safe_ware_index(itemname) :
                get_egbase(L).descriptions().safe_worker_index(itemname);
	if (lua_gettop(L) > 2 && luaL_checkboolean(L, 3)) {
		upcast(const Widelands::ProductionsiteSettings, ps, get(L, get_egbase(L))->get_settings());
		if (ps == nullptr) {
			report_error(L, "This constructionsite will not become a productionsite");
		}
		for (const auto& pair : is_ware ? ps->ware_queues : ps->worker_queues) {
			if (pair.first == item) {
				lua_pushuint32(L, pair.second.desired_fill);
				return 1;
			}
		}
		NEVER_HERE();
	}
	lua_pushuint32(
	   L, get(L, get_egbase(L))
	         ->inputqueue(item, is_ware ? Widelands::wwWARE : Widelands::wwWORKER, nullptr)
	         .get_max_fill());
	return 1;
}
int LuaConstructionSite::set_desired_fill(lua_State* L) {
	const std::string itemname = luaL_checkstring(L, 2);
	const bool is_ware = get_egbase(L).descriptions().ware_exists(itemname);
	const Widelands::DescriptionIndex item =
	   is_ware ? get_egbase(L).descriptions().safe_ware_index(itemname) :
                get_egbase(L).descriptions().safe_worker_index(itemname);
	if (lua_gettop(L) > 3 && luaL_checkboolean(L, 4)) {
		upcast(Widelands::ProductionsiteSettings, ps, get(L, get_egbase(L))->get_settings());
		if (ps == nullptr) {
			report_error(L, "This constructionsite will not become a productionsite");
		}
		for (auto& pair : is_ware ? ps->ware_queues : ps->worker_queues) {
			if (pair.first == item) {
				pair.second.desired_fill = luaL_checkuint32(L, 3);
				return 0;
			}
		}
		NEVER_HERE();
	}
	get(L, get_egbase(L))
	   ->inputqueue(item, is_ware ? Widelands::wwWARE : Widelands::wwWORKER, nullptr)
	   .set_max_fill(luaL_checkuint32(L, 3));
	return 0;
}

/* RST
   .. method:: get_setting_warehouse_policy(wareworker)

      Only valid for warehouses under construction. Returns the stock policy to apply to the given
      ware or worker after completion.

      :arg wareworker: The set ware or worker stock policy in this warehouse.
      :type wareworker: :class:`string`
*/
int LuaConstructionSite::get_setting_warehouse_policy(lua_State* L) {
	upcast(Widelands::WarehouseSettings, ws, get(L, get_egbase(L))->get_settings());
	if (ws == nullptr) {
		lua_pushnil(L);
		return 1;
	}
	const std::string itemname = luaL_checkstring(L, 2);
	const bool is_ware = get_egbase(L).descriptions().ware_exists(itemname);
	const Widelands::DescriptionIndex item =
	   is_ware ? get_egbase(L).descriptions().safe_ware_index(itemname) :
                get_egbase(L).descriptions().safe_worker_index(itemname);
	wh_policy_to_string(
	   L, is_ware ? ws->ware_preferences.at(item) : ws->worker_preferences.at(item));
	return 1;
}
/* RST
   .. method:: set_setting_warehouse_policy(wareworker, policystring)

      Only valid for warehouses under construction. Sets the stock policy to apply to the given
      ware or worker after completion. Valid values for **policystring** are documented in
      :meth:`Warehouse.set_warehouse_policies`.

*/
int LuaConstructionSite::set_setting_warehouse_policy(lua_State* L) {
	upcast(Widelands::WarehouseSettings, ws, get(L, get_egbase(L))->get_settings());
	if (ws == nullptr) {
		report_error(L, "This constructionsite will not become a warehouse");
	}
	const std::string itemname = luaL_checkstring(L, 2);
	const bool is_ware = get_egbase(L).descriptions().ware_exists(itemname);
	const Widelands::DescriptionIndex item =
	   is_ware ? get_egbase(L).descriptions().safe_ware_index(itemname) :
                get_egbase(L).descriptions().safe_worker_index(itemname);
	(is_ware ? ws->ware_preferences.at(item) : ws->worker_preferences.at(item)) =
	   string_to_wh_policy(L, 3);
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
DismantleSite
-----------------

.. class:: DismantleSite

   A dismantle site as it appears in the game.

   More properties are available through this object's
   :class:`DismantleSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaDismantleSite::className[] = "DismantleSite";
const MethodType<LuaDismantleSite> LuaDismantleSite::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaDismantleSite> LuaDismantleSite::Properties[] = {
   PROP_RW(LuaDismantleSite, has_builder),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: has_builder

      (RW) :const:`true` if this dismantlesite has a builder. Changing this setting causes the
      worker to be instantly deleted, or to be created from thin air.
*/
int LuaDismantleSite::get_has_builder(lua_State* L) {
	lua_pushboolean(L, static_cast<int>(get(L, get_egbase(L))->builder_.is_set()));
	return 1;
}
int LuaDismantleSite::set_has_builder(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::PartiallyFinishedBuilding& cs = *get(L, egbase);
	const bool arg = luaL_checkboolean(L, -1);
	if (cs.builder_.is_set()) {
		if (!arg) {
			cs.builder_.get(egbase)->remove(egbase);
			cs.builder_ = nullptr;
			cs.set_seeing(false);
			if (upcast(Widelands::Game, g, &egbase)) {
				cs.request_builder(*g);
			}
		}
	} else if (arg) {
		assert(cs.builder_request_);
		delete cs.builder_request_;
		cs.builder_request_ = nullptr;
		Widelands::Worker& w = egbase.descriptions()
		                          .get_worker_descr(cs.owner().tribe().builder())
		                          ->create(egbase, cs.get_owner(), &cs, cs.get_position());
		cs.builder_ = &w;
		if (upcast(Widelands::Game, g, &egbase)) {
			w.start_task_buildingwork(*g);
		}
		cs.set_seeing(true);
	}
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Warehouse
---------

.. class:: Warehouse

   Every Headquarter, Port or Warehouse on the Map is of this type.

   See also:
      * :ref:`has_wares`
      * :ref:`has_workers`
      * :ref:`has_soldiers`

   More properties are available through this object's
   :class:`WarehouseDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaWarehouse::className[] = "Warehouse";
const MethodType<LuaWarehouse> LuaWarehouse::Methods[] = {
   METHOD(LuaWarehouse, set_wares),
   METHOD(LuaWarehouse, get_wares),
   METHOD(LuaWarehouse, set_workers),
   METHOD(LuaWarehouse, get_workers),
   METHOD(LuaWarehouse, set_soldiers),
   METHOD(LuaWarehouse, get_soldiers),
   METHOD(LuaWarehouse, set_warehouse_policies),
   METHOD(LuaWarehouse, get_warehouse_policies),
   METHOD(LuaWarehouse, start_expedition),
   METHOD(LuaWarehouse, cancel_expedition),
   {nullptr, nullptr},
};
const PropertyType<LuaWarehouse> LuaWarehouse::Properties[] = {
   PROP_RO(LuaWarehouse, portdock),
   PROP_RO(LuaWarehouse, expedition_in_progress),
   PROP_RW(LuaWarehouse, warehousename),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// UNTESTED
/* RST
   .. attribute:: portdock

      (RO) If this Warehouse is a port, returns the
      :class:`PortDock` attached to it, otherwise :const:`nil`.
*/
int LuaWarehouse::get_portdock(lua_State* L) {
	return upcasted_map_object_to_lua(L, get(L, get_egbase(L))->get_portdock());
}

/* RST
   .. attribute:: expedition_in_progress

      (RO) If this Warehouse is a port, and an expedition is in
      progress, this is :const:`true`, otherwise :const:`nil`.
*/
int LuaWarehouse::get_expedition_in_progress(lua_State* L) {

	Widelands::EditorGameBase& egbase = get_egbase(L);

	if (egbase.is_game()) {
		const Widelands::PortDock* pd = get(L, egbase)->get_portdock();
		if (pd != nullptr) {
			if (pd->expedition_started()) {
				return 1;
			}
		}
	}
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

int LuaWarehouse::get_workers(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = wh->owner().tribe();

	// Parsing the argument(s), result will be either single index
	// or list of indexes
	Widelands::DescriptionIndex worker_index = Widelands::INVALID_INDEX;
	std::vector<Widelands::DescriptionIndex> workers_list;
	parse_wares_workers_list(L, tribe, &worker_index, &workers_list, false);

	// Here we create the output - either a single integer of table of pairs
	if (worker_index != Widelands::INVALID_INDEX) {
		lua_pushuint32(L, wh->get_workers().stock(worker_index));
	} else if (!workers_list.empty()) {
		lua_newtable(L);
		for (auto idx : workers_list) {
			lua_pushstring(L, tribe.get_worker_descr(idx)->name());
			lua_pushuint32(L, wh->get_workers().stock(idx));
			lua_settable(L, -3);
		}
	} else {
		NEVER_HERE();
	}
	return 1;
}

int LuaWarehouse::get_wares(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = wh->owner().tribe();

	// Parsing the argument(s), result will be either single index
	// or list of indexes
	Widelands::DescriptionIndex ware_index = Widelands::INVALID_INDEX;
	std::vector<Widelands::DescriptionIndex> ware_list;
	parse_wares_workers_list(L, tribe, &ware_index, &ware_list, true);

	// Here we create the output - either a single integer of table of pairs
	if (ware_index != Widelands::INVALID_INDEX) {
		lua_pushuint32(L, wh->get_wares().stock(ware_index));
	} else if (!ware_list.empty()) {
		lua_newtable(L);
		for (auto idx : ware_list) {
			lua_pushstring(L, tribe.get_ware_descr(idx)->name());
			lua_pushuint32(L, wh->get_wares().stock(idx));
			lua_settable(L, -3);
		}
	} else {
		NEVER_HERE();
	}
	return 1;
}

int LuaWarehouse::set_wares(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = wh->owner().tribe();
	InputMap setpoints;
	parse_wares_workers_counted(L, tribe, &setpoints, true);

	for (const auto& ware : setpoints) {
		const Widelands::DescriptionIndex& index = ware.first.first;
		int32_t d = ware.second - wh->get_wares().stock(index);
		if (d < 0) {
			wh->remove_wares(index, -d);
		} else if (d > 0) {
			wh->insert_wares(index, d);
		}
	}
	return 0;
}

int LuaWarehouse::set_workers(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = wh->owner().tribe();
	InputMap setpoints;
	parse_wares_workers_counted(L, tribe, &setpoints, false);

	for (const auto& worker : setpoints) {
		const Widelands::DescriptionIndex& index = worker.first.first;
		int32_t d = worker.second - wh->get_workers().stock(index);
		if (d < 0) {
			wh->remove_workers(index, -d);
		} else if (d > 0) {
			wh->insert_workers(index, d);
		}
	}
	return 0;
}

inline bool do_set_ware_policy(Widelands::Warehouse* wh,
                               const Widelands::DescriptionIndex idx,
                               const Widelands::StockPolicy p) {
	wh->set_ware_policy(idx, p);
	return true;
}

/**
 * Sets the given policy for the given ware in the given warehouse and return true.
 * If the no ware with the given name exists for the tribe of the warehouse, return false.
 */
inline bool do_set_ware_policy(Widelands::Warehouse* wh,
                               const std::string& name,
                               const Widelands::StockPolicy p) {
	const Widelands::TribeDescr& tribe = wh->owner().tribe();
	Widelands::DescriptionIndex idx = tribe.ware_index(name);
	if (!tribe.has_ware(idx)) {
		return false;
	}
	return do_set_ware_policy(wh, idx, p);
}

inline bool do_set_worker_policy(Widelands::Warehouse* wh,
                                 const Widelands::DescriptionIndex idx,
                                 const Widelands::StockPolicy p) {
	const Widelands::TribeDescr& tribe = wh->owner().tribe();
	// If the worker does not cost anything, ignore it
	// Otherwise, an unlimited stream of carriers might leave the warehouse
	if (tribe.get_worker_descr(idx)->is_buildable() &&
	    tribe.get_worker_descr(idx)->buildcost().empty()) {
		return true;
	}
	wh->set_worker_policy(idx, p);
	return true;
}

/**
 * Sets the given policy for the given worker in the given warehouse and returns true.
 * Also returns true if the given worker does not cost anything but in this case does not set its
 * policy.
 * If no worker with the given name exists for the tribe of the warehouse, return false.
 */
inline bool do_set_worker_policy(Widelands::Warehouse* wh,
                                 const std::string& name,
                                 const Widelands::StockPolicy p) {
	const Widelands::TribeDescr& tribe = wh->owner().tribe();
	Widelands::DescriptionIndex idx = tribe.worker_index(name);
	if (!tribe.has_worker(idx)) {
		return false;
	}
	return do_set_worker_policy(wh, idx, p);
}
/* RST
   .. method:: set_warehouse_policies(which, policy)

      Sets the policies how the warehouse should handle the given wares and workers.

      :arg which: Behaves like :meth:`wl.map.MapObject.get_wares`.
      :type which: :class:`string` or :class:`table`
      :arg policy: The policy to apply for all the wares and workers given in **which**.
      :type policy: A string out of ``"normal"``, ``"prefer"``, ``"dontstock"`` or ``"remove"``

      Usage examples:

      .. code-block:: lua

         wh:set_warehouse_policies("all", "remove")               -- remove all wares
         wh:set_warehouse_policies("coal", "prefer")              -- prefer coal
         wh:set_warehouse_policies({"coal", "log"}, "dontstock")  -- don't store coal and logs


*/
int LuaWarehouse::set_warehouse_policies(lua_State* L) {
	int32_t nargs = lua_gettop(L);
	if (nargs != 3) {
		report_error(L, "Wrong number of arguments to set_warehouse_policies!");
	}

	Widelands::Warehouse* wh = get(L, get_egbase(L));
	Widelands::StockPolicy p = string_to_wh_policy(L, -1);
	lua_pop(L, 1);
	const Widelands::TribeDescr& tribe = wh->owner().tribe();

	// takes either "all", a name or an array of names
	if (lua_isstring(L, 2) != 0) {
		const std::string& what = luaL_checkstring(L, -1);
		if (what == "all") {
			for (const Widelands::DescriptionIndex& i : tribe.wares()) {
				do_set_ware_policy(wh, i, p);
			}
			for (const Widelands::DescriptionIndex& i : tribe.workers()) {
				do_set_worker_policy(wh, i, p);
			}
		} else {
			// Only one item requested
			if (!do_set_ware_policy(wh, what, p) && !do_set_worker_policy(wh, what, p)) {
				// Unknown whatever, abort
				report_error(L, "Unknown name: <%s>", what.c_str());
			}
		}
	} else {
		// array of names
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			const std::string& what = luaL_checkstring(L, -1);
			if (!do_set_ware_policy(wh, what, p) && !do_set_worker_policy(wh, what, p)) {
				// Note that this will change the policy for entries earlier in the list
				// but when the user provides broken data its his own fault
				report_error(L, "Unknown name: <%s>", what.c_str());
			}
			lua_pop(L, 1);
		}
	}

	return 0;
}

// Gets the warehouse policy by ware/worker-name or id
#define WH_GET_POLICY(type)                                                                        \
	inline void do_get_##type##_policy(                                                             \
	   lua_State* L, Widelands::Warehouse* wh, const Widelands::DescriptionIndex idx) {             \
		wh_policy_to_string(L, wh->get_##type##_policy(idx));                                        \
	}                                                                                               \
                                                                                                   \
	inline bool do_get_##type##_policy(                                                             \
	   lua_State* L, Widelands::Warehouse* wh, const std::string& name) {                           \
		const Widelands::TribeDescr& tribe = wh->owner().tribe();                                    \
		Widelands::DescriptionIndex idx = tribe.type##_index(name);                                  \
		if (!tribe.has_##type(idx)) {                                                                \
			return false;                                                                             \
		}                                                                                            \
		do_get_##type##_policy(L, wh, idx);                                                          \
		return true;                                                                                 \
	}

WH_GET_POLICY(ware)
WH_GET_POLICY(worker)
#undef WH_GET_POLICY

/* RST
   .. method:: get_warehouse_policies(which)

      Returns the policies how the warehouse should handle the given wares
      and workers. See :meth:`Warehouse.set_warehouse_policies` for policy strings.

      :arg which: Behaves like :meth:`wl.map.MapObject.get_wares`.
      :type which: :class:`string` or :class:`array`

      :returns: :class:`string` or :class:`table`

      Usage example:

      .. code-block:: lua

         wh:get_warehouse_policies("log")
            -- Returns e.g. "normal"

         wh:get_warehouse_policies({"ax", "coal"})
            -- Returns a :class:`table` like {"ax"="normal", "coal"="prefer"}


*/
int LuaWarehouse::get_warehouse_policies(lua_State* L) {
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "Wrong number of arguments to get_warehouse_policies!");
	}
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	const Widelands::TribeDescr& tribe = wh->owner().tribe();
	// takes either "all", a single name or an array of names
	if (lua_isstring(L, 2) != 0) {
		std::string what = luaL_checkstring(L, -1);
		if (what == "all") {
			lua_newtable(L);
			for (const Widelands::DescriptionIndex& i : tribe.wares()) {
				std::string name = tribe.get_ware_descr(i)->name();
				lua_pushstring(L, name.c_str());
				do_get_ware_policy(L, wh, i);
				lua_rawset(L, -3);
			}
			for (const Widelands::DescriptionIndex& i : tribe.workers()) {
				std::string name = tribe.get_worker_descr(i)->name();
				lua_pushstring(L, name.c_str());
				do_get_worker_policy(L, wh, i);
				lua_rawset(L, -3);
			}
		} else {
			// Only one item requested
			if (!do_get_ware_policy(L, wh, what) && !do_get_worker_policy(L, wh, what)) {
				// Unknown whatever, abort
				report_error(L, "Unknown name: <%s>", what.c_str());
			}
		}
	} else {
		// array of names
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_newtable(L);
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			// Stack is: ... input_table new_table nil input_key input_value
			// input_value is the name of the ware or worker and will be added into the new table
			// input_key is an index and is dropped by the next call of lua_next()
			const std::string& what = luaL_checkstring(L, -1);
			if (!do_get_ware_policy(L, wh, what) && !do_get_worker_policy(L, wh, what)) {
				// Note that this will change the policy for entries earlier in the list
				// but when the user provides broken data its his own fault
				report_error(L, "Unknown name: <%s>", what.c_str());
			}
			lua_rawset(L, -4);
		}
	}
	return 1;
}

// documented in parent class
int LuaWarehouse::get_soldiers(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	return do_get_soldiers(L, *wh->soldier_control(), wh->owner().tribe());
}

// documented in parent class
int LuaWarehouse::set_soldiers(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	return do_set_soldiers(L, wh->get_position(), wh->mutable_soldier_control(), wh->get_owner());
}

/* RST
   .. attribute:: warehousename

   .. versionadded:: 1.2

   (RW) The name of the warehouse as :class:`string`.


*/
int LuaWarehouse::get_warehousename(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	lua_pushstring(L, wh->get_warehouse_name().c_str());
	return 1;
}
int LuaWarehouse::set_warehousename(lua_State* L) {
	Widelands::Warehouse* wh = get(L, get_egbase(L));
	wh->set_warehouse_name(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. method:: start_expedition()

      Starts preparation for an expedition.

*/
int LuaWarehouse::start_expedition(lua_State* L) {

	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Warehouse* wh = get(L, egbase);

	if (wh == nullptr) {
		return 0;
	}

	if (upcast(Widelands::Game, game, &egbase)) {
		const Widelands::PortDock* pd = wh->get_portdock();
		if (pd == nullptr) {
			return 0;
		}
		if (!pd->expedition_started()) {
			game->send_player_start_or_cancel_expedition(*wh);
			return 1;
		}
	}

	return 0;
}

/* RST
   .. method:: cancel_expedition()

      Cancels an expedition if in progress.

*/
int LuaWarehouse::cancel_expedition(lua_State* L) {

	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Warehouse* wh = get(L, egbase);

	if (wh == nullptr) {
		return 0;
	}

	if (upcast(Widelands::Game, game, &egbase)) {
		const Widelands::PortDock* pd = wh->get_portdock();
		if (pd == nullptr) {
			return 0;
		}
		if (pd->expedition_started()) {
			game->send_player_start_or_cancel_expedition(*wh);
			return 1;
		}
	}

	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

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
	return do_set_workers<LuaProductionSite>(L, ps, get_valid_workers_for(*ps));
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

// TODO(kaputtnik): Readd RST once this get implemented
/*
Market
---------

.. class:: Market

   A Market used for trading with other players.

   For functions see:
      * :ref:`has_wares`
      * :ref:`has_workers`

   More properties are available through this object's
   :class:`MarketDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaMarket::className[] = "Market";
const MethodType<LuaMarket> LuaMarket::Methods[] = {
   METHOD(LuaMarket, propose_trade),
   // TODO(sirver,trading): Implement and fix documentation.
   // METHOD(LuaMarket, set_wares),
   // METHOD(LuaMarket, get_wares),
   // METHOD(LuaMarket, set_workers),
   // METHOD(LuaMarket, get_workers),
   {nullptr, nullptr},
};
const PropertyType<LuaMarket> LuaMarket::Properties[] = {
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// TODO(kaputtnik): Readd RST once this get implemented
/*
   .. method:: propose_trade(other_market, num_batches, items_to_send, items_to_receive)

      TODO(sirver,trading): document

      :returns: :const:`nil`
*/
int LuaMarket::propose_trade(lua_State* L) {
	if (lua_gettop(L) != 5) {
		report_error(L, "Takes 4 arguments.");
	}
	Widelands::Game& game = get_game(L);
	Widelands::Market* self = get(L, game);
	Widelands::Market* other_market = (*get_user_class<LuaMarket>(L, 2))->get(L, game);
	const int num_batches = luaL_checkinteger(L, 3);

	const Widelands::BillOfMaterials items_to_send =
	   parse_wares_as_bill_of_material(L, 4, self->owner().tribe());
	// TODO(sirver,trading): unsure if correct. Test inter-tribe trading, i.e.
	// Barbarians trading with Empire, but shipping Atlantean only wares.
	const Widelands::BillOfMaterials items_to_receive =
	   parse_wares_as_bill_of_material(L, 5, self->owner().tribe());
	const int trade_id = game.propose_trade(Widelands::Trade{
	   items_to_send, items_to_receive, num_batches, self->serial(), other_market->serial()});

	// TODO(sirver,trading): Wrap 'Trade' into its own Lua class?
	lua_pushint32(L, trade_id);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
MilitarySite
--------------

.. class:: MilitarySite

   Military buildings with stationed soldiers.

   See also: :ref:`has_soldiers`

   More properties are available through this object's
   :class:`MilitarySiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaMilitarySite::className[] = "MilitarySite";
const MethodType<LuaMilitarySite> LuaMilitarySite::Methods[] = {
   METHOD(LuaMilitarySite, get_soldiers),
   METHOD(LuaMilitarySite, set_soldiers),
   {nullptr, nullptr},
};
const PropertyType<LuaMilitarySite> LuaMilitarySite::Properties[] = {
   PROP_RO(LuaMilitarySite, max_soldiers),
   PROP_RW(LuaMilitarySite, soldier_preference),
   PROP_RW(LuaMilitarySite, capacity),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

// documented in parent class
int LuaMilitarySite::get_max_soldiers(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaMilitarySite::get_soldiers(lua_State* L) {
	Widelands::MilitarySite* ms = get(L, get_egbase(L));
	return do_get_soldiers(L, *ms->soldier_control(), ms->owner().tribe());
}

// documented in parent class
int LuaMilitarySite::set_soldiers(lua_State* L) {
	Widelands::MilitarySite* ms = get(L, get_egbase(L));
	return do_set_soldiers(L, ms->get_position(), ms->mutable_soldier_control(), ms->get_owner());
}

/* RST
   .. attribute:: capacity

      (RW) The number of soldiers meant to be stationed here.
*/
int LuaMilitarySite::set_capacity(lua_State* L) {
	get(L, get_egbase(L))->mutable_soldier_control()->set_soldier_capacity(luaL_checkuint32(L, -1));
	return 0;
}
int LuaMilitarySite::get_capacity(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/* RST
   .. attribute:: soldier_preference

      (RW) ``"heroes"`` if this site prefers heroes; ``"rookies"`` for rookies;
         or ``"any"`` for no predilection.
*/
int LuaMilitarySite::get_soldier_preference(lua_State* L) {
	lua_pushstring(
	   L, soldier_preference_to_string(get(L, get_egbase(L))->get_soldier_preference()).c_str());
	return 1;
}
int LuaMilitarySite::set_soldier_preference(lua_State* L) {
	try {
		get(L, get_egbase(L))
		   ->set_soldier_preference(string_to_soldier_preference(luaL_checkstring(L, -1)));
	} catch (const WException& e) {
		report_error(L, "%s", e.what());
	}
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
TrainingSite
--------------

.. class:: TrainingSite

   A specialized production site for training soldiers.

   See also: :ref:`has_soldiers`

   More properties are available through this object's
   :class:`TrainingSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaTrainingSite::className[] = "TrainingSite";
const MethodType<LuaTrainingSite> LuaTrainingSite::Methods[] = {
   METHOD(LuaTrainingSite, get_soldiers),
   METHOD(LuaTrainingSite, set_soldiers),
   {nullptr, nullptr},
};
const PropertyType<LuaTrainingSite> LuaTrainingSite::Properties[] = {
   PROP_RO(LuaTrainingSite, max_soldiers),
   PROP_RW(LuaTrainingSite, capacity),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

// documented in parent class
int LuaTrainingSite::get_max_soldiers(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/* RST
   .. attribute:: capacity

      (RW) The number of soldiers meant to be stationed here.
*/
int LuaTrainingSite::set_capacity(lua_State* L) {
	get(L, get_egbase(L))->mutable_soldier_control()->set_soldier_capacity(luaL_checkuint32(L, -1));
	return 0;
}
int LuaTrainingSite::get_capacity(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldier_control()->soldier_capacity());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaTrainingSite::get_soldiers(lua_State* L) {
	Widelands::TrainingSite* ts = get(L, get_egbase(L));
	return do_get_soldiers(L, *ts->soldier_control(), ts->owner().tribe());
}

// documented in parent class
int LuaTrainingSite::set_soldiers(lua_State* L) {
	Widelands::TrainingSite* ts = get(L, get_egbase(L));
	return do_set_soldiers(L, ts->get_position(), ts->mutable_soldier_control(), ts->get_owner());
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Bob
---

.. class:: Bob

   This is the base class for all Bobs in widelands.

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaBob::className[] = "Bob";
const MethodType<LuaBob> LuaBob::Methods[] = {
   METHOD(LuaBob, has_caps),
   {nullptr, nullptr},
};
const PropertyType<LuaBob> LuaBob::Properties[] = {
   PROP_RO(LuaBob, field),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: field

      (RO) The field the bob is located on.
*/
// UNTESTED
int LuaBob::get_field(lua_State* L) {

	Widelands::EditorGameBase& egbase = get_egbase(L);

	Widelands::Coords coords = get(L, egbase)->get_position();

	return to_lua<LuaMaps::LuaField>(L, new LuaMaps::LuaField(coords.x, coords.y));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
   .. method:: has_caps(swim_or_walk)

      (RO) Whether this bob can swim or walk.

      :arg swim_or_walk: Can be either of :const:`"swims"` or :const:`"walks"`.
      :type swim_or_walk: :class:`string`

      :returns: :const:`true` if this bob is able to **swim_or_walk**, otherwise :const:`false`.
*/
// UNTESTED
int LuaBob::has_caps(lua_State* L) {
	std::string query = luaL_checkstring(L, 2);

	uint32_t movecaps = get(L, get_egbase(L))->descr().movecaps();

	if (query == "swims") {
		lua_pushboolean(L, movecaps & Widelands::MOVECAPS_SWIM);
	} else if (query == "walks") {
		lua_pushboolean(L, movecaps & Widelands::MOVECAPS_WALK);
	} else {
		report_error(L, "Unknown caps queried: %s!", query.c_str());
	}

	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Ship
----

.. class:: Ship

   This represents a ship in game.

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaShip::className[] = "Ship";
const MethodType<LuaShip> LuaShip::Methods[] = {
   METHOD(LuaShip, get_wares),
   METHOD(LuaShip, get_workers),
   METHOD(LuaShip, build_colonization_port),
   METHOD(LuaShip, make_expedition),
   METHOD(LuaShip, refit),
   {nullptr, nullptr},
};
const PropertyType<LuaShip> LuaShip::Properties[] = {
   PROP_RO(LuaShip, debug_ware_economy),
   PROP_RO(LuaShip, debug_worker_economy),
   PROP_RO(LuaShip, last_portdock),
   PROP_RO(LuaShip, destination),
   PROP_RO(LuaShip, state),
   PROP_RO(LuaShip, type),
   PROP_RW(LuaShip, scouting_direction),
   PROP_RW(LuaShip, island_explore_direction),
   PROP_RW(LuaShip, shipname),
   PROP_RW(LuaShip, capacity),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// UNTESTED, for debug only
int LuaShip::get_debug_ware_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(Widelands::wwWARE));
	return 1;
}
int LuaShip::get_debug_worker_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(Widelands::wwWORKER));
	return 1;
}

/* RST
   .. attribute:: destination

      (RO) Either :const:`nil` if there is no current destination, otherwise
      the :class:`PortDock` or :class:`Ship`.
*/
// UNTESTED
int LuaShip::get_destination(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Ship* ship = get(L, egbase);

	if (Widelands::PortDock* pd = ship->get_destination_port(egbase); pd != nullptr) {
		return upcasted_map_object_to_lua(L, pd);
	}
	if (Widelands::Ship* s = ship->get_destination_ship(egbase); s != nullptr) {
		return upcasted_map_object_to_lua(L, s);
	}
#if 0  // TODO(Nordfriese): Pinned notes don't have a Lua class yet
	if (Widelands::PinnedNote* note = ship->get_destination_note(egbase); note != nullptr) {
		return upcasted_map_object_to_lua(L, note);
	}
#endif

	lua_pushnil(L);
	return 1;
}

/* RST
   .. attribute:: last_portdock

      (RO) Either :const:`nil` if no port was ever visited or the last portdock
      was destroyed, otherwise the :class:`PortDock` of the last visited port.
*/
// UNTESTED
int LuaShip::get_last_portdock(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	return upcasted_map_object_to_lua(L, get(L, egbase)->get_lastdock(egbase));
}

/* RST
   .. attribute:: state

      (RO) Query which state the ship is in. Can be either of:

      * :const:`"transport"`,
      * :const:`"exp_waiting"`, :const:`"exp_scouting"`, :const:`"exp_found_port_space"`,
        :const:`"exp_colonizing"`,
      * :const:`"sink_request"`, :const:`"sink_animation"`

      :returns: The ship's state as :const:`string`, or :const:`nil` if there is no valid state.


*/
// UNTESTED sink states
int LuaShip::get_state(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	if (egbase.is_game()) {
		switch (get(L, egbase)->get_ship_state()) {
		case Widelands::ShipStates::kTransport:
			lua_pushstring(L, "transport");
			break;
		case Widelands::ShipStates::kExpeditionWaiting:
			lua_pushstring(L, "exp_waiting");
			break;
		case Widelands::ShipStates::kExpeditionScouting:
			lua_pushstring(L, "exp_scouting");
			break;
		case Widelands::ShipStates::kExpeditionPortspaceFound:
			lua_pushstring(L, "exp_found_port_space");
			break;
		case Widelands::ShipStates::kExpeditionColonizing:
			lua_pushstring(L, "exp_colonizing");
			break;
		case Widelands::ShipStates::kSinkRequest:
			lua_pushstring(L, "sink_request");
			break;
		case Widelands::ShipStates::kSinkAnimation:
			lua_pushstring(L, "sink_animation");
			break;
		}
		return 1;
	}
	return 0;
}

/* RST
   .. attribute:: type

      .. versionadded:: 1.2

      (RO) The state the ship is in as :const:`string`:
      :const:`"transport"` or :const:`"warship"`.
*/
int LuaShip::get_type(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	switch (get(L, egbase)->get_ship_type()) {
	case Widelands::ShipType::kTransport:
		lua_pushstring(L, "transport");
		break;
	case Widelands::ShipType::kWarship:
		lua_pushstring(L, "warship");
		break;
	}
	return 1;
}

int LuaShip::get_scouting_direction(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	if (egbase.is_game()) {
		switch (get(L, egbase)->get_scouting_direction()) {
		case Widelands::WalkingDir::WALK_NE:
			lua_pushstring(L, "ne");
			break;
		case Widelands::WalkingDir::WALK_E:
			lua_pushstring(L, "e");
			break;
		case Widelands::WalkingDir::WALK_SE:
			lua_pushstring(L, "se");
			break;
		case Widelands::WalkingDir::WALK_SW:
			lua_pushstring(L, "sw");
			break;
		case Widelands::WalkingDir::WALK_W:
			lua_pushstring(L, "w");
			break;
		case Widelands::WalkingDir::WALK_NW:
			lua_pushstring(L, "nw");
			break;
		case Widelands::WalkingDir::IDLE:
			return 0;
		}
		return 1;
	}
	return 0;
}

int LuaShip::set_scouting_direction(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	if (upcast(Widelands::Game, game, &egbase)) {
		std::string dirname = luaL_checkstring(L, 3);
		Widelands::WalkingDir dir = Widelands::WalkingDir::IDLE;

		if (dirname == "ne") {
			dir = Widelands::WalkingDir::WALK_NE;
		} else if (dirname == "e") {
			dir = Widelands::WalkingDir::WALK_E;
		} else if (dirname == "se") {
			dir = Widelands::WalkingDir::WALK_SE;
		} else if (dirname == "sw") {
			dir = Widelands::WalkingDir::WALK_SW;
		} else if (dirname == "w") {
			dir = Widelands::WalkingDir::WALK_W;
		} else if (dirname == "nw") {
			dir = Widelands::WalkingDir::WALK_NW;
		} else {
			return 0;
		}
		game->send_player_ship_scouting_direction(*get(L, egbase), dir);
		return 1;
	}
	return 0;
}

/* RST
   .. attribute:: island_explore_direction

      (RW) Actual direction if the ship sails around an island.
      Sets/returns :const:`"cw"` (clockwise), :const:`"ccw"` (counter clock wise) or :const:`nil`.

*/
int LuaShip::get_island_explore_direction(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	if (egbase.is_game()) {
		switch (get(L, egbase)->get_island_explore_direction()) {
		case Widelands::IslandExploreDirection::kCounterClockwise:
			lua_pushstring(L, "ccw");
			break;
		case Widelands::IslandExploreDirection::kClockwise:
			lua_pushstring(L, "cw");
			break;
		case Widelands::IslandExploreDirection::kNotSet:
			return 0;
		}
		return 1;
	}
	return 0;
}

int LuaShip::set_island_explore_direction(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	if (upcast(Widelands::Game, game, &egbase)) {
		Widelands::Ship* ship = get(L, egbase);
		std::string dir = luaL_checkstring(L, 3);
		if (dir == "ccw") {
			game->send_player_ship_explore_island(
			   *ship, Widelands::IslandExploreDirection::kCounterClockwise);
		} else if (dir == "cw") {
			game->send_player_ship_explore_island(
			   *ship, Widelands::IslandExploreDirection::kClockwise);
		} else {
			return 0;
		}
		return 1;
	}
	return 0;
}

/* RST
   .. attribute:: shipname

   .. versionchanged:: 1.2
      Read-only in 1.1 and older.

   (RW) The name of the ship as :class:`string`.


*/
int LuaShip::get_shipname(lua_State* L) {
	Widelands::Ship* ship = get(L, get_egbase(L));
	lua_pushstring(L, ship->get_shipname().c_str());
	return 1;
}
int LuaShip::set_shipname(lua_State* L) {
	Widelands::Ship* ship = get(L, get_egbase(L));
	ship->set_shipname(luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: capacity

   (RW) The ship's current capacity. Defaults to the capacity defined in the tribe's singleton ship
   description.

   Do not change this value if the ship is currently shipping more items than the new capacity
   allows.
*/
int LuaShip::get_capacity(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_capacity());
	return 1;
}
int LuaShip::set_capacity(lua_State* L) {
	Widelands::Ship& s = *get(L, get_egbase(L));
	const uint32_t c = luaL_checkuint32(L, -1);
	if (s.get_nritems() > c) {
		report_error(L, "Ship is currently transporting %u items – cannot set capacity to %u",
		             s.get_nritems(), c);
	}
	s.set_capacity(c);
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: get_wares([which = nil])

      When called without arguments, returns the number of wares on this ship.

      When called with a ware name as argument, returns the amount of the
      specified ware on the ship.

      When called with :const:`""` as argument, returns an :class:`array` with
      the names of all loaded wares.

      :returns: The number of wares or an :class:`array` of :class:`string`.
*/
// UNTESTED
int LuaShip::get_wares(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Ship* ship = get(L, egbase);

	Widelands::WareInstance* ware = nullptr;
	std::string filter;

	if (lua_gettop(L) > 1) {
		filter = luaL_checkstring(L, -1);
		if (filter.empty()) {
			// Push array of all ware names

			lua_newtable(L);
			uint32_t index = 1;
			for (uint32_t i = 0; i < ship->get_nritems(); ++i) {
				const Widelands::ShippingItem& item = ship->get_item(i);
				item.get(egbase, &ware, nullptr);
				if (ware != nullptr) {
					lua_pushuint32(L, index++);
					lua_pushstring(L, ware->descr().name().c_str());
					lua_rawset(L, -3);
				}
			}

			return 1;
		}
	}

	// Count wares, optionally filtering by `filter`.
	int nwares = 0;
	for (uint32_t i = 0; i < ship->get_nritems(); ++i) {
		const Widelands::ShippingItem& item = ship->get_item(i);
		item.get(egbase, &ware, nullptr);
		if ((ware != nullptr) && (filter.empty() || ware->descr().name() == filter)) {
			++nwares;
		}
	}
	lua_pushint32(L, nwares);
	return 1;
}

/* RST
   .. method:: get_workers([which = nil])

      When called without arguments, returns the number of workers on this ship.

      When called with a worker name as argument, returns the amount of the
      specified worker on the ship.

      When called with :const:`""` as argument, returns an :class:`array`
      with all loaded workers.

      :returns: The number of workers or an :class:`array` of :class:`Worker`
*/
// UNTESTED
int LuaShip::get_workers(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Ship* ship = get(L, egbase);

	Widelands::Worker* worker = nullptr;
	std::string filter;

	if (lua_gettop(L) > 1) {
		filter = luaL_checkstring(L, -1);
		if (filter.empty()) {
			// Push array of all workers

			lua_newtable(L);
			uint32_t index = 1;
			for (uint32_t i = 0; i < ship->get_nritems(); ++i) {
				const Widelands::ShippingItem& item = ship->get_item(i);
				item.get(egbase, nullptr, &worker);
				if (worker != nullptr) {
					lua_pushuint32(L, index++);
					upcasted_map_object_to_lua(L, worker);
					lua_rawset(L, -3);
				}
			}

			return 1;
		}
	}

	// Count workers, optionally filtering by `filter`.
	int nworkers = 0;
	for (uint32_t i = 0; i < ship->get_nritems(); ++i) {
		const Widelands::ShippingItem& item = ship->get_item(i);
		item.get(egbase, nullptr, &worker);
		if ((worker != nullptr) && (filter.empty() || worker->descr().name() == filter)) {
			++nworkers;
		}
	}
	lua_pushint32(L, nworkers);
	return 1;
}

/* RST
   .. method:: build_colonization_port()

      Returns :const:`true` if port space construction was started (ship was in adequate
      status and a found portspace nearby).

      :returns: :const:`true` or :const:`false`
*/
int LuaShip::build_colonization_port(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	Widelands::Ship* ship = get(L, egbase);
	if (ship->get_ship_state() == Widelands::ShipStates::kExpeditionPortspaceFound) {
		if (upcast(Widelands::Game, game, &egbase)) {
			game->send_player_ship_construct_port(*ship, ship->exp_port_spaces().back());
			return 1;
		}
	}
	return 0;
}

/* RST
   .. method:: make_expedition([items])

      Turns this ship into an expedition ship without a base port. Creates all necessary
      wares and a builder plus, if desired, the specified additional **items**.
      The ship must be empty and not an expedition ship when this method is called.

      Note that the ships :attr:`capacity` is not adjusted if you give additional **items**.
      If the amount of additional items exceeds the capacity, the game doesn't like it.

      See also :any:`launch_expeditions` which adjusts :attr:`capacity`
      depending on the given wares and workers.

      :arg items: Optional: Additional items to the ones that are needed to build a port.
      :type items: :class:`table` of ``{ware_or_worker=amount}`` pairs.

      :returns: :const:`nil`

      Example with check for sufficient capacity:

      .. code-block:: lua

         -- place a ship on the map
         ship = wl.Game().players[1]:place_ship(wl.Game().map:get_field(21,27))

         -- check capacity
         local free_capacity = ship.capacity

         -- subtract buildcost for port
         local buildings = wl.Game().players[1].tribe.buildings
         for i, building in ipairs(buildings) do
            if building.is_port then
               for ware, amount in pairs(building.buildcost) do
                  free_capacity = free_capacity - amount
               end
               -- stop iterating buildings
               break
            end
         end
         -- finally subtract one slot for the builder
         free_capacity = free_capacity - 1

         -- adjust capacity
         if free_capacity < 13 then
            ship.capacity = ship.capacity + 13
         end

         -- create expedition with additional 13 items
         ship:make_expedition({barbarians_soldier = 10, fish = 3})
*/
int LuaShip::make_expedition(lua_State* L) {
	upcast(Widelands::Game, game, &get_egbase(L));
	assert(game);
	Widelands::Ship* ship = get(L, *game);
	assert(ship);
	if (ship->get_ship_state() != Widelands::ShipStates::kTransport || ship->get_nritems() > 0) {
		report_error(L, "Ship.make_expedition can be used only on empty transport ships!");
	}

	const Widelands::TribeDescr& tribe = ship->owner().tribe();
	for (const auto& pair : tribe.get_building_descr(tribe.port())->buildcost()) {
		for (size_t i = pair.second; i > 0; --i) {
			Widelands::WareInstance& w =
			   *new Widelands::WareInstance(pair.first, tribe.get_ware_descr(pair.first));
			w.init(*game);
			ship->add_item(*game, Widelands::ShippingItem(w));
		}
	}
	ship->add_item(*game, Widelands::ShippingItem(
	                         tribe.get_worker_descr(tribe.builder())
	                            ->create(*game, ship->get_owner(), nullptr, ship->get_position())));
	std::map<Widelands::DescriptionIndex, uint32_t>
	   workers_to_create;  // Lua table sorting order is not deterministic and may cause desyncs
	if (lua_gettop(L) > 1) {
		luaL_checktype(L, 2, LUA_TTABLE);
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			uint32_t amount = luaL_checkuint32(L, -1);
			lua_pop(L, 1);
			std::string what = luaL_checkstring(L, -1);
			Widelands::DescriptionIndex index = game->descriptions().ware_index(what);
			if (tribe.has_ware(index)) {
				while (amount > 0) {
					Widelands::WareInstance& w =
					   *new Widelands::WareInstance(index, tribe.get_ware_descr(index));
					w.init(*game);
					ship->add_item(*game, Widelands::ShippingItem(w));
					--amount;
				}
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					workers_to_create[index] = amount;
				} else {
					report_error(L, "Invalid ware or worker: %s", what.c_str());
				}
			}
		}
	}

	for (auto& pair : workers_to_create) {
		for (; pair.second > 0; --pair.second) {
			ship->add_item(*game, Widelands::ShippingItem(tribe.get_worker_descr(pair.first)
			                                                 ->create(*game, ship->get_owner(),
			                                                          nullptr, ship->get_position())));
		}
	}

	ship->set_destination(*game, nullptr);
	ship->start_task_expedition(*game);

	return 0;
}

/* RST
   .. method:: refit(type)

      .. versionadded:: 1.2

      Order the ship to refit to the given type.

      :arg string type: :const:`"transport"` or :const:`"warship"`

      :returns: :const:`nil`
*/
int LuaShip::refit(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments to refit!");
	}
	Widelands::Game& game = get_game(L);
	Widelands::Ship* ship = get(L, game);
	const std::string type = luaL_checkstring(L, 2);
	if (type == "transport") {
		ship->refit(game, Widelands::ShipType::kTransport);
	} else if (type == "warship") {
		ship->refit(game, Widelands::ShipType::kWarship);
	} else {
		report_error(L, "Invalid ship refit type '%s'", type.c_str());
	}
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Worker
------

.. class:: Worker

   All workers that are visible on the map are of this kind.

   More properties are available through this object's
   :class:`WorkerDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaWorker::className[] = "Worker";
const MethodType<LuaWorker> LuaWorker::Methods[] = {
   METHOD(LuaWorker, evict),
   {nullptr, nullptr},
};
const PropertyType<LuaWorker> LuaWorker::Properties[] = {
   PROP_RO(LuaWorker, owner),
   PROP_RO(LuaWorker, location),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: owner

      (RO) The :class:`wl.game.Player` who owns this worker.
*/
// UNTESTED
int LuaWorker::get_owner(lua_State* L) {
	get_factory(L).push_player(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

/* RST
   .. attribute:: location

      (RO) The location where this worker is situated. This will be either a
      :class:`Building`, :class:`Road`, :class:`Flag` or :const:`nil`. Note
      that a worker that is stored in a warehouse has a location :const:`nil`.
      A worker that is out working (e.g. hunter) has as a location his
      building. A stationed soldier has his military building as location.
      Workers on transit usually have the Road they are currently on as
      location.
*/
// UNTESTED
int LuaWorker::get_location(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	return upcasted_map_object_to_lua(
	   L, dynamic_cast<Widelands::BaseImmovable*>(get(L, egbase)->get_location(egbase)));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
   .. method:: evict()

      .. versionadded:: 1.2

      Evict this worker from his current workplace.
*/
int LuaWorker::evict(lua_State* L) {
	Widelands::Game& game = get_game(L);
	get(L, game)->evict(game);
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Soldier
-------

.. class:: Soldier

   All soldiers that are on the map are represented by this class.

   More properties are available through this object's
   :class:`SoldierDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaSoldier::className[] = "Soldier";
const MethodType<LuaSoldier> LuaSoldier::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaSoldier> LuaSoldier::Properties[] = {
   PROP_RO(LuaSoldier, attack_level),   PROP_RO(LuaSoldier, defense_level),
   PROP_RO(LuaSoldier, health_level),   PROP_RO(LuaSoldier, evade_level),
   PROP_RW(LuaSoldier, current_health), {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: attack_level

      (RO) The current attack level of this soldier
*/
// UNTESTED
int LuaSoldier::get_attack_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_attack_level());
	return 1;
}

/* RST
   .. attribute:: defense_level

      (RO) The current defense level of this soldier
*/
// UNTESTED
int LuaSoldier::get_defense_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_defense_level());
	return 1;
}

/* RST
   .. attribute:: health_level

      (RO) The current health level of this soldier
*/
// UNTESTED
int LuaSoldier::get_health_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_health_level());
	return 1;
}

/* RST
   .. attribute:: evade_level

      (RO) The current evade level of this soldier
*/
// UNTESTED
int LuaSoldier::get_evade_level(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_evade_level());
	return 1;
}

/* RST
   .. attribute:: current_health

      (RW) This soldier's current number of hitpoints left.
*/
int LuaSoldier::get_current_health(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_current_health());
	return 1;
}
int LuaSoldier::set_current_health(lua_State* L) {
	Widelands::Soldier& s = *get(L, get_egbase(L));

	const uint32_t ch = luaL_checkuint32(L, -1);
	if (ch == 0) {
		report_error(L, "Soldier.current_health must be greater than 0");
	}

	const uint32_t maxhealth =
	   s.descr().get_base_health() + s.get_health_level() * s.descr().get_health_incr_per_level();
	if (ch > maxhealth) {
		report_error(
		   L, "Soldier.current_health %u must not be greater than %u for %s with health level %u", ch,
		   maxhealth, s.descr().name().c_str(), s.get_health_level());
	}

	s.set_current_health(ch);
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Field
-----

.. class:: Field

   This class represents one Field in Widelands. The field may contain
   immovables like Flags or Buildings and can be connected via Roads. Every
   Field has two Triangles associated with itself: The right and the down one.

   You cannot instantiate this directly, access it via
   ``wl.Game().map.get_field()`` instead.
*/

const char LuaField::className[] = "Field";
const MethodType<LuaField> LuaField::Methods[] = {
   METHOD(LuaField, __eq),     METHOD(LuaField, __tostring),   METHOD(LuaField, region),
   METHOD(LuaField, has_caps), METHOD(LuaField, has_max_caps),
#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
   METHOD(LuaField, indicate),
#endif
   {nullptr, nullptr},
};
const PropertyType<LuaField> LuaField::Properties[] = {
   PROP_RO(LuaField, __hash),
   PROP_RO(LuaField, x),
   PROP_RO(LuaField, y),
   PROP_RO(LuaField, rn),
   PROP_RO(LuaField, ln),
   PROP_RO(LuaField, trn),
   PROP_RO(LuaField, tln),
   PROP_RO(LuaField, bln),
   PROP_RO(LuaField, brn),
   PROP_RO(LuaField, immovable),
   PROP_RO(LuaField, bobs),
   PROP_RW(LuaField, terr),
   PROP_RW(LuaField, terd),
   PROP_RW(LuaField, height),
   PROP_RW(LuaField, raw_height),
   PROP_RO(LuaField, viewpoint_x),
   PROP_RO(LuaField, viewpoint_y),
   PROP_RW(LuaField, resource),
   PROP_RW(LuaField, resource_amount),
   PROP_RW(LuaField, initial_resource_amount),
   PROP_RO(LuaField, claimers),
   PROP_RO(LuaField, owner),
   PROP_RO(LuaField, buildable),
   PROP_RO(LuaField, has_roads),
   {nullptr, nullptr, nullptr},
};

void LuaField::__persist(lua_State* L) {
	PERS_INT32("x", coords_.x);
	PERS_INT32("y", coords_.y);
}

void LuaField::__unpersist(lua_State* L) {
	UNPERS_INT32("x", coords_.x)
	UNPERS_INT32("y", coords_.y)
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: __hash

      (RO) The hashed coordinates of the field's position. Used to identify a class in a Set.
*/
int LuaField::get___hash(lua_State* L) {
	lua_pushuint32(L, coords_.hash());
	return 1;
}

/* RST
   .. attribute:: x, y

      (RO) The x/y coordinate of this field
*/
int LuaField::get_x(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, coords_.x);
	return 1;
}
int LuaField::get_y(lua_State* L) {  // NOLINT - can not be made const
	lua_pushuint32(L, coords_.y);
	return 1;
}

/* RST
   .. attribute:: height

      (RW) The height of this field. The default height is 10, you can increase
      or decrease this value to build mountains. Note though that if you change
      this value too much, all surrounding fields will also change their
      heights because the slope is constrained. If you are changing the height
      of many terrains at once, use :attr:`raw_height` instead and then call
      :any:`recalculate` afterwards.
*/
int LuaField::get_height(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_height());
	return 1;
}
int LuaField::set_height(lua_State* L) {
	uint32_t height = luaL_checkuint32(L, -1);
	Widelands::FCoords f = fcoords(L);

	if (f.field->get_height() == height) {
		return 0;
	}

	if (height > MAX_FIELD_HEIGHT) {
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);
	}

	Widelands::EditorGameBase& egbase = get_egbase(L);
	egbase.mutable_map()->set_height(egbase, f, height);

	return 0;
}

/* RST
   .. attribute:: raw_height

      (RW) The same as :attr:`height`, but setting this will not trigger a
      recalculation of the surrounding fields. You can use this field to
      change the height of many fields on a map quickly, then use
      :any:`recalculate` to make sure that everything is in order.
*/
// UNTESTED
int LuaField::get_raw_height(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_height());
	return 1;
}
int LuaField::set_raw_height(lua_State* L) {
	uint32_t height = luaL_checkuint32(L, -1);
	Widelands::FCoords f = fcoords(L);

	if (f.field->get_height() == height) {
		return 0;
	}

	if (height > MAX_FIELD_HEIGHT) {
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);
	}

	f.field->set_height(height);

	return 0;
}

/* RST
   .. attribute:: viewpoint_x, viewpoint_y

      (RO) Returns the position in pixels to move the view to to center
      this field for the current interactive player.
*/
int LuaField::get_viewpoint_x(lua_State* L) {
	Vector2f point =
	   MapviewPixelFunctions::to_map_pixel_with_normalization(get_egbase(L).map(), coords_);
	lua_pushdouble(L, point.x);
	return 1;
}
int LuaField::get_viewpoint_y(lua_State* L) {
	Vector2f point =
	   MapviewPixelFunctions::to_map_pixel_with_normalization(get_egbase(L).map(), coords_);
	lua_pushdouble(L, point.y);
	return 1;
}

/* RST
   .. attribute:: resource

      (RW) The name of the resource that is available in this field or
      "none".

      :see also: :attr:`resource_amount`
*/
int LuaField::get_resource(lua_State* L) {
	const Widelands::ResourceDescription* res_desc =
	   get_egbase(L).descriptions().get_resource_descr(fcoords(L).field->get_resources());

	lua_pushstring(L, res_desc != nullptr ? res_desc->name().c_str() : "none");

	return 1;
}
int LuaField::set_resource(lua_State* L) {
	auto& egbase = get_egbase(L);
	Widelands::DescriptionIndex res = egbase.descriptions().resource_index(luaL_checkstring(L, -1));

	if (res == Widelands::INVALID_INDEX) {
		report_error(L, "Illegal resource: '%s'", luaL_checkstring(L, -1));
	}

	auto c = fcoords(L);
	const auto current_amount = c.field->get_resources_amount();
	auto* map = egbase.mutable_map();
	map->initialize_resources(c, res, c.field->get_initial_res_amount());
	map->set_resources(c, current_amount);
	return 0;
}

/* RST
   .. attribute:: resource_amount

      (RW) How many items of the resource is available in this field.

      :see also: :attr:`resource`
*/
int LuaField::get_resource_amount(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_resources_amount());
	return 1;
}
int LuaField::set_resource_amount(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	auto c = fcoords(L);
	Widelands::DescriptionIndex res = c.field->get_resources();
	auto amount = luaL_checkint32(L, -1);
	const Widelands::ResourceDescription* res_desc = egbase.descriptions().get_resource_descr(res);
	Widelands::ResourceAmount max_amount = (res_desc != nullptr) ? res_desc->max_amount() : 0;

	if (amount < 0 || amount > max_amount) {
		report_error(L, "Illegal amount: %i, must be >= 0 and <= %i", amount,
		             static_cast<unsigned int>(max_amount));
	}

	auto* map = egbase.mutable_map();
	if (egbase.is_game()) {
		map->set_resources(c, amount);
	} else {
		// in editor, reset also initial amount
		map->initialize_resources(c, res, amount);
	}
	return 0;
}

/* RST
   .. attribute:: initial_resource_amount

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) Starting value of resource.

      :see also: :attr:`resource`
*/
int LuaField::get_initial_resource_amount(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_initial_res_amount());
	return 1;
}
int LuaField::set_initial_resource_amount(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	auto c = fcoords(L);
	Widelands::DescriptionIndex res = c.field->get_resources();
	auto amount = luaL_checkint32(L, -1);
	const Widelands::ResourceDescription* res_desc = egbase.descriptions().get_resource_descr(res);
	Widelands::ResourceAmount max_amount = (res_desc != nullptr) ? res_desc->max_amount() : 0;

	if (amount < 0 || amount > max_amount) {
		report_error(L, "Illegal amount: %i, must be >= 0 and <= %i", amount,
		             static_cast<unsigned int>(max_amount));
	}

	egbase.mutable_map()->initialize_resources(c, res, amount);
	return 0;
}

/* RST
   .. attribute:: immovable

      (RO) The immovable that stands on this field or :const:`nil`. If you want
      to remove an immovable, you can use :func:`wl.map.MapObject.remove`.
*/
int LuaField::get_immovable(lua_State* L) {
	Widelands::BaseImmovable* bi = get_egbase(L).map().get_immovable(coords_);

	if (bi == nullptr) {
		return 0;
	}
	upcasted_map_object_to_lua(L, bi);

	return 1;
}

/* RST
   .. attribute:: bobs

      (RO) An :class:`array` of :class:`~wl.map.Bob` that are associated
      with this field.
*/
// UNTESTED
int LuaField::get_bobs(lua_State* L) {
	Widelands::Bob* b = fcoords(L).field->get_first_bob();

	lua_newtable(L);
	uint32_t cidx = 1;
	while (b != nullptr) {
		lua_pushuint32(L, cidx++);
		upcasted_map_object_to_lua(L, b);
		lua_rawset(L, -3);
		b = b->get_next_bob();
	}
	return 1;
}

/* RST
   .. attribute:: terr, terd

      (RW) The terrain of the right/down triangle. This is a string value
      containing the name of the terrain as it is defined in the world
      configuration. You can change the terrain by simply assigning another
      valid name to these variables. If you are changing the terrain from or to
      water, the map will not recalculate whether it allows seafaring, because
      this recalculation can take up a lot of performance. If you need this
      recalculated, you can do so by calling :any:`recalculate_seafaring` after
      you're done changing terrains.
*/
int LuaField::get_terr(lua_State* L) {
	const Widelands::TerrainDescription* td =
	   get_egbase(L).descriptions().get_terrain_descr(fcoords(L).field->terrain_r());
	lua_pushstring(L, td->name().c_str());
	return 1;
}
int LuaField::set_terr(lua_State* L) {
	const char* name = luaL_checkstring(L, -1);
	Widelands::EditorGameBase& egbase = get_egbase(L);
	try {
		const Widelands::DescriptionIndex td = egbase.mutable_descriptions()->load_terrain(name);
		egbase.mutable_map()->change_terrain(
		   egbase, Widelands::TCoords<Widelands::FCoords>(fcoords(L), Widelands::TriangleIndex::R),
		   td);
	} catch (const Widelands::GameDataError& e) {
		report_error(L, "set_terr: %s", e.what());
	}

	lua_pushstring(L, name);
	return 1;
}

int LuaField::get_terd(lua_State* L) {
	const Widelands::TerrainDescription* td =
	   get_egbase(L).descriptions().get_terrain_descr(fcoords(L).field->terrain_d());
	lua_pushstring(L, td->name().c_str());
	return 1;
}
int LuaField::set_terd(lua_State* L) {
	const char* name = luaL_checkstring(L, -1);
	Widelands::EditorGameBase& egbase = get_egbase(L);
	try {
		const Widelands::DescriptionIndex td = egbase.mutable_descriptions()->load_terrain(name);
		egbase.mutable_map()->change_terrain(
		   egbase, Widelands::TCoords<Widelands::FCoords>(fcoords(L), Widelands::TriangleIndex::D),
		   td);
	} catch (const Widelands::GameDataError& e) {
		report_error(L, "set_terd: %s", e.what());
	}

	lua_pushstring(L, name);
	return 1;
}

/* RST
   .. attribute:: rn, ln, brn, bln, trn, tln

      (RO) The neighbour fields of this field. The abbreviations stand for:

      * ``rn`` -- Right neighbour
      * ``ln`` -- Left neighbour
      * ``brn`` -- Bottom right neighbour
      * ``bln`` -- Bottom left neighbour
      * ``trn`` -- Top right neighbour
      * ``tln`` -- Top left neighbour

      Note that the widelands map wraps at its borders, that is the following
      holds:

      .. code-block:: lua

         wl.map.Field(wl.map.get_width()-1, 10).rn == wl.map.Field(0, 10)
*/
#define GET_X_NEIGHBOUR(X)                                                                         \
	int LuaField::get_##X(lua_State* L) {                                                           \
		Widelands::Coords n;                                                                         \
		get_egbase(L).map().get_##X(coords_, &n);                                                    \
		to_lua<LuaField>(L, new LuaField(n.x, n.y));                                                 \
		return 1;                                                                                    \
	}
GET_X_NEIGHBOUR(rn)
GET_X_NEIGHBOUR(ln)
GET_X_NEIGHBOUR(trn)
GET_X_NEIGHBOUR(tln)
GET_X_NEIGHBOUR(bln)
GET_X_NEIGHBOUR(brn)

#undef GET_X_NEIGHBOUR

/* RST
   .. attribute:: owner

      (RO) The current owner of the field or :const:`nil` if noone owns it. See
      also :attr:`claimers`.
*/
int LuaField::get_owner(lua_State* L) {
	Widelands::PlayerNumber current_owner = fcoords(L).field->get_owned_by();
	if (current_owner != 0u) {
		get_factory(L).push_player(L, current_owner);
		return 1;
	}
	return 0;
}

/* RST
   .. attribute:: buildable

      (RO) Returns :const:`true` if a flag or building could be built on this field,
      independently of whether anybody currently owns this field.
*/
int LuaField::get_buildable(lua_State* L) {
	const Widelands::NodeCaps caps = fcoords(L).field->nodecaps();
	const bool is_buildable = ((caps & Widelands::BUILDCAPS_FLAG) != 0) ||
	                          ((caps & Widelands::BUILDCAPS_BUILDINGMASK) != 0);
	lua_pushboolean(L, static_cast<int>(is_buildable));
	return 1;
}

/* RST
   .. attribute:: has_roads

      (RO) Whether any roads lead to the field.
      Note that waterways are currently treated like roads.

      :returns: :const:`true` if any of the 6 directions has a road on it, :const:`false` otherwise.
*/
int LuaField::get_has_roads(lua_State* L) {

	const Widelands::FCoords& fc = fcoords(L);
	Widelands::Field* f = fc.field;
	if (f->get_road(Widelands::WalkingDir::WALK_E) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	if (f->get_road(Widelands::WalkingDir::WALK_SE) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	if (f->get_road(Widelands::WalkingDir::WALK_SW) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}

	Widelands::FCoords neighbor;
	const Widelands::Map& map = get_egbase(L).map();
	map.get_ln(fc, &neighbor);
	if (neighbor.field->get_road(Widelands::WalkingDir::WALK_E) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	map.get_tln(fc, &neighbor);
	if (neighbor.field->get_road(Widelands::WalkingDir::WALK_SE) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	map.get_trn(fc, &neighbor);
	if (neighbor.field->get_road(Widelands::WalkingDir::WALK_SW) != Widelands::RoadSegment::kNone) {
		lua_pushboolean(L, 1);
		return 1;
	}
	lua_pushboolean(L, 0);

	return 1;
}

/* RST
   .. attribute:: claimers

      (RO) An :class:`array` of players that have military influence over this
      field sorted by the amount of influence they have. Note that this does
      not necessarily mean that claimers[1] is also the owner of the field, as
      a field that houses a surrounded military building is owned by the
      surrounded player, but others have more military influence over it.

      Note: The one currently owning the field is in :attr:`owner`.
*/
int LuaField::get_claimers(lua_State* L) {
	Widelands::EditorGameBase& egbase = get_egbase(L);
	const Widelands::Map& map = egbase.map();

	std::vector<PlrInfluence> claimers;

	iterate_players_existing(other_p, map.get_nrplayers(), egbase, plr) claimers.emplace_back(
	   plr->player_number(), plr->military_influence(map.get_index(coords_, map.get_width())));

	std::stable_sort(claimers.begin(), claimers.end(), sort_claimers);

	lua_createtable(L, 1, 0);  // We mostly expect one claimer per field.

	// Push the players with military influence
	uint32_t cidx = 1;
	for (const PlrInfluence& claimer : claimers) {
		if (claimer.second <= 0) {
			continue;
		}
		lua_pushuint32(L, cidx++);
		get_factory(L).push_player(L, claimer.first);
		lua_rawset(L, -3);
	}

	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaField::__eq(lua_State* L) {
	lua_pushboolean(L, static_cast<int>((*get_user_class<LuaField>(L, -1))->coords_ == coords_));
	return 1;
}

int LuaField::__tostring(lua_State* L) {  // NOLINT - can not be made const
	const std::string pushme = format("Field(%i,%i)", coords_.x, coords_.y);
	lua_pushstring(L, pushme);
	return 1;
}

/* RST
   .. function:: region(r1[, r2])

      Returns an :class:`array` of all Fields inside the given region. If one argument
      is given it defines the radius of the region. If both arguments are
      specified, the first one defines the outer radius and the second one the
      inner radius and a hollow region is returned, that is all fields in the
      outer radius region minus all fields in the inner radius region.

      A small example:

      .. code-block:: lua

         f:region(1)

      will return an :class:`array` with the following entries (Note: Ordering of the
      fields inside the :class:`array` is not guaranteed):

      .. code-block:: lua

         {f, f.rn, f.ln, f.brn, f.bln, f.tln, f.trn}

      :returns: The :class:`array` of the given fields.
      :rtype: :class:`array`
*/
int LuaField::region(lua_State* L) {
	uint32_t n = lua_gettop(L);

	if (n == 3) {
		uint32_t radius = luaL_checkuint32(L, -2);
		uint32_t inner_radius = luaL_checkuint32(L, -1);
		return hollow_region(L, radius, inner_radius);
	}

	uint32_t radius = luaL_checkuint32(L, -1);
	return region(L, radius);
}

/* RST
   .. method:: has_caps(capname)

      Returns :const:`true` if the field has this **capname** associated
      with it, otherwise returns :const:`false`.

      Note: Immovables will hide the caps. If you want to have the caps
      without immovables use has_max_caps instead

      :arg capname: Can be either of:
      :type capname: :class:`string`

      * :const:`"small"`: Can a small building be built here?
      * :const:`"medium"`: Can a medium building be built here?
      * :const:`"big"`: Can a big building be built here?
      * :const:`"mine"`: Can a mine be built here?
      * :const:`"port"`: Can a port be built here?
      * :const:`"flag"`: Can a flag be built here?
      * :const:`"walkable"`: Is this field passable for walking bobs?
      * :const:`"swimmable"`: Is this field passable for swimming bobs?


*/
int LuaField::has_caps(lua_State* L) {
	const Widelands::FCoords& f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);
	lua_pushboolean(
	   L, static_cast<int>(check_has_caps(L, query, f, f.field->nodecaps(), get_egbase(L).map())));
	return 1;
}

/* RST
   .. method:: has_max_caps(capname)

      Returns :const:`true` if the field has this maximum caps (not taking immovables into account)
      associated with it, otherwise returns :const:`false`.

      :arg capname: For possible values see :meth:`has_caps`
      :type capname: :class:`string`

*/
int LuaField::has_max_caps(lua_State* L) {
	const Widelands::FCoords& f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);
	lua_pushboolean(
	   L, static_cast<int>(check_has_caps(L, query, f, f.field->maxcaps(), get_egbase(L).map())));
	return 1;
}

#if 0  // TODO(Nordfriese): Re-add training wheels code after v1.0
/* R#S#T
   .. method:: indicate(on)

      Show/Hide an arrow that points to this field. You can only point to 1 field at the same time.

      :arg on: Whether to show or hide the arrow
      :type on: :class:`boolean`
*/
// UNTESTED
int LuaField::indicate(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Expected 1 boolean");
	}

	InteractivePlayer* ipl = dynamic_cast<InteractivePlayer*>(get_egbase(L).get_ibase());
	if (ipl == nullptr) {
		report_error(L, "This can only be called when there's an interactive player");
	}

	const bool on = luaL_checkboolean(L, -1);
	if (on) {
		ipl->set_training_wheel_indicator_field(fcoords(L));
	} else {
		ipl->set_training_wheel_indicator_field(
		   Widelands::FCoords(Widelands::FCoords::null(), nullptr));
	}
	return 2;
}
#endif

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
int LuaField::region(lua_State* L, uint32_t radius) {
	const Widelands::Map& map = get_egbase(L).map();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   map, Widelands::Area<Widelands::FCoords>(fcoords(L), radius));

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		const Widelands::FCoords& loc = mr.location();
		to_lua<LuaField>(L, new LuaField(loc.x, loc.y));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

int LuaField::hollow_region(lua_State* L, uint32_t radius, uint32_t inner_radius) {
	const Widelands::Map& map = get_egbase(L).map();
	Widelands::HollowArea<Widelands::Area<>> har(Widelands::Area<>(coords_, radius), inner_radius);

	Widelands::MapHollowRegion<Widelands::Area<>> mr(map, har);

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		to_lua<LuaField>(L, new LuaField(mr.location()));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

const Widelands::FCoords LuaField::fcoords(lua_State* L) {
	return get_egbase(L).map().get_fcoords(coords_);
}

/* RST
PlayerSlot
----------

.. class:: PlayerSlot

   A player description as it is in the map. This contains information
   about the start position, the name of the player if this map is played
   as scenario and it's tribe. Note that these information can be different
   than the players actually valid in the game as in single player games,
   the player can choose most parameters freely.
*/
const char LuaPlayerSlot::className[] = "PlayerSlot";
const MethodType<LuaPlayerSlot> LuaPlayerSlot::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaPlayerSlot> LuaPlayerSlot::Properties[] = {
   PROP_RW(LuaPlayerSlot, tribe_name),
   PROP_RW(LuaPlayerSlot, name),
   PROP_RW(LuaPlayerSlot, starting_field),
   {nullptr, nullptr, nullptr},
};

void LuaPlayerSlot::__persist(lua_State* L) {
	PERS_UINT32("player", player_number_);
}

void LuaPlayerSlot::__unpersist(lua_State* L) {
	UNPERS_UINT32("player", player_number_)
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: tribe_name

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) The name of the tribe suggested for this player in this map.
*/
int LuaPlayerSlot::get_tribe_name(lua_State* L) {  // NOLINT - can not be made const
	lua_pushstring(L, get_egbase(L).map().get_scenario_player_tribe(player_number_));
	return 1;
}
int LuaPlayerSlot::set_tribe_name(lua_State* L) {  // NOLINT - can not be made const
	get_egbase(L).mutable_map()->set_scenario_player_tribe(player_number_, luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: name

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) The name for this player as suggested in this map.
*/
int LuaPlayerSlot::get_name(lua_State* L) {  // NOLINT - can not be made const
	lua_pushstring(L, get_egbase(L).map().get_scenario_player_name(player_number_));
	return 1;
}
int LuaPlayerSlot::set_name(lua_State* L) {  // NOLINT - can not be made const
	get_egbase(L).mutable_map()->set_scenario_player_name(player_number_, luaL_checkstring(L, -1));
	return 0;
}

/* RST
   .. attribute:: starting_field

      .. versionchanged:: 1.2
         Read-only in 1.1 and older.

      (RW) The starting_field for this player as set in the map.
      Note that it is not guaranteed that the HQ of the player is on this
      field as scenarios and starting conditions are free to place the HQ
      wherever it want. This field is only centered when the game starts.
*/
int LuaPlayerSlot::get_starting_field(lua_State* L) {  // NOLINT - can not be made const
	to_lua<LuaField>(L, new LuaField(get_egbase(L).map().get_starting_pos(player_number_)));
	return 1;
}
int LuaPlayerSlot::set_starting_field(lua_State* L) {  // NOLINT - can not be made const
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, -1);
	get_egbase(L).mutable_map()->set_starting_pos(player_number_, c->coords());
	return 0;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/*
 ==========================================================
 C METHODS
 ==========================================================
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
}
}  // namespace LuaMaps
