/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "scripting/lua_map.h"

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/input_queue.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/findimmovable.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/terrain_affinity.h"
#include "logic/map_objects/tribes/carrier.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/world/resource_description.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/widelands_geometry.h"
#include "scripting/factory.h"
#include "scripting/globals.h"
#include "scripting/lua_errors.h"
#include "scripting/lua_game.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

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
                    const FCoords& f,
                    const NodeCaps& caps,
                    const Widelands::Map& map) {
	if (query == "walkable") {
		return caps & MOVECAPS_WALK;
	}
	if (query == "swimmable") {
		return caps & MOVECAPS_SWIM;
	}
	if (query == "small") {
		return caps & BUILDCAPS_SMALL;
	}
	if (query == "medium") {
		return caps & BUILDCAPS_MEDIUM;
	}
	if (query == "big") {
		return (caps & BUILDCAPS_BIG) == BUILDCAPS_BIG;
	}
	if (query == "port") {
		return (caps & BUILDCAPS_PORT) && map.is_port_space(f);
	}
	if (query == "mine") {
		return caps & BUILDCAPS_MINE;
	}
	if (query == "flag") {
		return caps & BUILDCAPS_FLAG;
	}
	report_error(L, "Unknown caps queried: %s!", query.c_str());
}

// Pushes a lua table with (name, count) pairs for the given 'ware_amount_container' on the
// stack. The 'type' needs to be WARE or WORKER. Returns 1.
int wares_or_workers_map_to_lua(lua_State* L,
                                const Buildcost& ware_amount_map,
                                MapObjectType type) {
	lua_newtable(L);
	for (const auto& ware_amount : ware_amount_map) {
		switch (type) {
		case MapObjectType::WORKER:
			lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(ware_amount.first)->name());
			break;
		case MapObjectType::WARE:
			lua_pushstring(L, get_egbase(L).tribes().get_ware_descr(ware_amount.first)->name());
			break;
		default:
			throw wexception("wares_or_workers_map_to_lua needs a ware or worker");
		}
		lua_pushuint32(L, ware_amount.second);
		lua_settable(L, -3);
	}
	return 1;
}

// Pushes a lua table of tables with food ware names on the stack. Returns 1.
// Resulting table will look e.g. like {{"barbarians_bread"}, {"fish", "meat"}}
int food_list_to_lua(lua_State* L, const std::vector<std::vector<std::string>>& table) {
	lua_newtable(L);
	int counter = 0;
	for (const std::vector<std::string>& foodlist : table) {
		lua_pushuint32(L, ++counter);
		lua_newtable(L);
		int counter2 = 0;
		for (const std::string& foodname : foodlist) {
			lua_pushuint32(L, ++counter2);
			lua_pushstring(L, foodname);
			lua_settable(L, -3);
		}
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
	SoldierMapDescr() : health(0), attack(0), defense(0), evade(0) {
	}

	uint8_t health;
	uint8_t attack;
	uint8_t defense;
	uint8_t evade;

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
		if (health == ot.health && attack == ot.attack && defense == ot.defense &&
		    evade == ot.evade) {
			return true;
		}
		return false;
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
InputSet parse_get_input_arguments(lua_State* L, const TribeDescr& tribe, bool* return_number) {
	/* takes either "all", a name or an array of names */
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "Wrong number of arguments to get_inputs!");
	}
	*return_number = false;
	InputSet rv;
	if (lua_isstring(L, 2)) {
		std::string what = luaL_checkstring(L, -1);
		if (what == "all") {
			for (const DescriptionIndex& i : tribe.wares()) {
				rv.insert(std::make_pair(i, wwWARE));
			}
			for (const DescriptionIndex& i : tribe.workers()) {
				rv.insert(std::make_pair(i, wwWORKER));
			}
		} else {
			/* Only one item requested */
			DescriptionIndex index = tribe.ware_index(what);
			if (tribe.has_ware(index)) {
				rv.insert(std::make_pair(index, wwWARE));
				*return_number = true;
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					rv.insert(std::make_pair(index, wwWORKER));
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
			DescriptionIndex index = tribe.ware_index(what);
			if (tribe.has_ware(index)) {
				rv.insert(std::make_pair(index, wwWARE));
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					rv.insert(std::make_pair(index, wwWORKER));
				} else {
					report_error(L, "Invalid input: <%s>", what.c_str());
				}
			}
			lua_pop(L, 1);
		}
	}
	return rv;
}

InputMap parse_set_input_arguments(lua_State* L, const TribeDescr& tribe) {
	int32_t nargs = lua_gettop(L);
	if (nargs != 2 && nargs != 3) {
		report_error(L, "Wrong number of arguments to set_inputs!");
	}
	InputMap rv;
	if (nargs == 3) {
		/* name amount */
		std::string what = luaL_checkstring(L, 2);
		DescriptionIndex index = tribe.ware_index(what);
		if (tribe.has_ware(index)) {
			rv.insert(std::make_pair(std::make_pair(index, wwWARE), luaL_checkuint32(L, 3)));
		} else {
			index = tribe.worker_index(what);
			if (tribe.has_worker(index)) {
				rv.insert(std::make_pair(std::make_pair(index, wwWORKER), luaL_checkuint32(L, 3)));
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
			DescriptionIndex index = tribe.ware_index(what);
			if (tribe.has_ware(index)) {
				rv.insert(std::make_pair(std::make_pair(index, wwWARE), luaL_checkuint32(L, -1)));
			} else {
				index = tribe.worker_index(what);
				if (tribe.has_worker(index)) {
					rv.insert(std::make_pair(std::make_pair(index, wwWORKER), luaL_checkuint32(L, -1)));
				} else {
					report_error(L, "Invalid input: <%s>", what.c_str());
				}
			}
			lua_pop(L, 1);
		}
	}
	return rv;
}

WaresWorkersMap count_wares_on_flag_(Flag& f, const Tribes& tribes) {
	WaresWorkersMap rv;

	for (const WareInstance* ware : f.get_wares()) {
		DescriptionIndex i = tribes.ware_index(ware->descr().name());
		if (!rv.count(i)) {
			rv.insert(Widelands::WareAmount(i, 1));
		} else {
			++rv[i];
		}
	}
	return rv;
}

// Sort functor to sort the owners claiming a field by their influence.
static int sort_claimers(const PlrInfluence& first, const PlrInfluence& second) {
	return first.second > second.second;
}

// Return the valid workers for a Road.
WaresWorkersMap get_valid_workers_for(const RoadBase& r) {
	WaresWorkersMap valid_workers;
	if (r.descr().type() == MapObjectType::WATERWAY) {
		valid_workers.insert(WorkerAmount(r.owner().tribe().ferry(), 1));
	} else {
		valid_workers.insert(WorkerAmount(r.owner().tribe().carrier(), 1));
		upcast(const Road, road, &r);
		assert(road);
		if (road->is_busy()) {
			valid_workers.insert(WorkerAmount(r.owner().tribe().carrier2(), 1));
		}
	}

	return valid_workers;
}

// Returns the valid workers allowed in 'pi'.
WaresWorkersMap get_valid_workers_for(const ProductionSite& ps) {
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
		lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(item.first)->name());
		lua_pushuint32(L, item.second);
		lua_rawset(L, -3);
	}
	return 1;
}

// Does most of the work of get_workers for player immovables (buildings and roads mainly).
int do_get_workers(lua_State* L, const PlayerImmovable& pi, const WaresWorkersMap& valid_workers) {
	const TribeDescr& tribe = pi.owner().tribe();

	DescriptionIndex worker_index = INVALID_INDEX;
	std::vector<DescriptionIndex> workers_list;

	RequestedWareWorker parse_output =
	   parse_wares_workers_list(L, tribe, &worker_index, &workers_list, false);

	// c_workers is map (index:count) of all workers at the immovable
	WaresWorkersMap c_workers;
	for (const Worker* w : pi.get_workers()) {
		DescriptionIndex i = tribe.worker_index(w->descr().name());
		if (!c_workers.count(i)) {
			c_workers.insert(WorkerAmount(i, 1));
		} else {
			++c_workers[i];
		}
	}

	// We return quantity for asked worker
	if (worker_index != INVALID_INDEX) {
		if (c_workers.count(worker_index)) {
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
		for (const DescriptionIndex& i : workers_list) {
			Widelands::Quantity cnt = 0;
			if (c_workers.count(i)) {
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
	EditorGameBase& egbase = get_egbase(L);
	const TribeDescr& tribe = pi->owner().tribe();

	// setpoints is map of index:quantity
	InputMap setpoints;
	parse_wares_workers_counted(L, tribe, &setpoints, false);

	// c_workers is actual statistics, the map index:quantity
	WaresWorkersMap c_workers;
	for (const Worker* w : pi->get_workers()) {
		DescriptionIndex i = tribe.worker_index(w->descr().name());
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
		const WorkerDescr* wdes = tribe.get_worker_descr(index);
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
				for (const Worker* w : pi->get_workers()) {
					if (tribe.worker_index(w->descr().name()) == index) {
						const_cast<Worker*>(w)->remove(egbase);
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
unbox_lua_soldier_description(lua_State* L, int table_index, const SoldierDescr& sd) {
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
// documentation in HasSoldiers to understand the valid arguments.
SoldiersMap parse_set_soldiers_arguments(lua_State* L, const SoldierDescr& soldier_descr) {
	SoldiersMap rv;
	if (lua_gettop(L) > 2) {
		// STACK: cls, descr, count
		const Widelands::Quantity count = luaL_checkuint32(L, 3);
		const SoldierMapDescr d = unbox_lua_soldier_description(L, 2, soldier_descr);
		rv.insert(SoldierAmount(d, count));
	} else {
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			const SoldierMapDescr d = unbox_lua_soldier_description(L, 3, soldier_descr);
			const Widelands::Quantity count = luaL_checkuint32(L, -1);
			rv.insert(SoldierAmount(d, count));
			lua_pop(L, 1);
		}
	}
	return rv;
}

// Does most of the work of get_soldiers for buildings.
int do_get_soldiers(lua_State* L, const Widelands::SoldierControl& sc, const TribeDescr& tribe) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Invalid arguments!");
	}

	const SoldiersList soldiers = sc.stationed_soldiers();
	if (lua_isstring(L, -1)) {
		if (std::string(luaL_checkstring(L, -1)) != "all") {
			report_error(L, "Invalid arguments!");
		}

		// Return All Soldiers
		SoldiersMap hist;
		for (const Soldier* s : soldiers) {
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
		const SoldierDescr& soldier_descr =
		   dynamic_cast<const SoldierDescr&>(*tribe.get_worker_descr(tribe.soldier()));

		// Only return the number of those requested
		const SoldierMapDescr wanted = unbox_lua_soldier_description(L, 2, soldier_descr);
		Widelands::Quantity rv = 0;
		for (const Soldier* s : soldiers) {
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
                    const Coords& building_position,
                    SoldierControl* sc,
                    Player* owner) {
	assert(sc != nullptr);
	assert(owner != nullptr);

	const TribeDescr& tribe = owner->tribe();
	const SoldierDescr& soldier_descr =  //  soldiers
	   dynamic_cast<const SoldierDescr&>(*tribe.get_worker_descr(tribe.soldier()));
	SoldiersMap setpoints = parse_set_soldiers_arguments(L, soldier_descr);

	// Get information about current soldiers
	const std::vector<Soldier*> curs = sc->stationed_soldiers();
	SoldiersMap hist;
	for (const Soldier* s : curs) {
		SoldierMapDescr sd(s->get_health_level(), s->get_attack_level(), s->get_defense_level(),
		                   s->get_evade_level());

		SoldiersMap::iterator i = hist.find(sd);
		if (i == hist.end()) {
			hist[sd] = 1;
		} else {
			++i->second;
		}
		if (!setpoints.count(sd)) {
			setpoints[sd] = 0;
		}
	}

	// Now adjust them
	EditorGameBase& egbase = get_egbase(L);
	for (const SoldiersMap::value_type& sp : setpoints) {
		Widelands::Quantity cur = 0;
		SoldiersMap::iterator i = hist.find(sp.first);
		if (i != hist.end()) {
			cur = i->second;
		}

		int d = sp.second - cur;
		if (d < 0) {
			while (d) {
				for (Soldier* s : sc->stationed_soldiers()) {
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
			for (; d; --d) {
				Soldier& soldier = dynamic_cast<Soldier&>(
				   soldier_descr.create(egbase, owner, nullptr, building_position));
				soldier.set_level(sp.first.health, sp.first.attack, sp.first.defense, sp.first.evade);
				if (sc->incorporate_soldier(egbase, soldier)) {
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
                         const TribeDescr& tribe,
                         InputMap* ware_workers_list,
                         bool is_ware) {
	luaL_checktype(L, table_index, LUA_TTABLE);
	lua_pushnil(L);
	while (lua_next(L, table_index) != 0) {
		if (is_ware) {
			if (tribe.ware_index(luaL_checkstring(L, -2)) == INVALID_INDEX) {
				report_error(L, "Illegal ware %s", luaL_checkstring(L, -2));
			}
			ware_workers_list->insert(
			   std::make_pair(std::make_pair(tribe.ware_index(luaL_checkstring(L, -2)),
			                                 Widelands::WareWorker::wwWARE),
			                  luaL_checkuint32(L, -1)));
		} else {
			if (tribe.worker_index(luaL_checkstring(L, -2)) == INVALID_INDEX) {
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

BillOfMaterials
parse_wares_as_bill_of_material(lua_State* L, int table_index, const TribeDescr& tribe) {
	InputMap input_map;
	parse_wares_workers(L, table_index, tribe, &input_map, true /* is_ware */);
	BillOfMaterials result;
	for (const auto& pair : input_map) {
		result.push_back(std::make_pair(pair.first.first, pair.second));
	}
	return result;
}

const Widelands::TribeDescr& get_tribe_descr(lua_State* L, const std::string& tribename) {
	const Tribes& tribes = get_egbase(L).tribes();
	if (!tribes.tribe_exists(tribename)) {
		report_error(L, "Tribe '%s' does not exist", tribename.c_str());
	}
	return *get_egbase(L).tribes().get_tribe_descr(
	   get_egbase(L).mutable_tribes()->load_tribe(tribename));
}

}  // namespace

/*
 * Upcast the given map object description to a higher type and hand this
 * to Lua. We use this so that scripters always work with the highest class
 * object available.
 */
#define CAST_TO_LUA(klass, lua_klass)                                                              \
	to_lua<lua_klass>(L, new lua_klass(dynamic_cast<const klass*>(descr)))
int upcasted_map_object_descr_to_lua(lua_State* L, const MapObjectDescr* const descr) {
	assert(descr != nullptr);

	if (descr->type() >= MapObjectType::BUILDING) {
		switch (descr->type()) {
		case MapObjectType::CONSTRUCTIONSITE:
			return CAST_TO_LUA(ConstructionSiteDescr, LuaConstructionSiteDescription);
		case MapObjectType::DISMANTLESITE:
			return CAST_TO_LUA(DismantleSiteDescr, LuaDismantleSiteDescription);
		case MapObjectType::PRODUCTIONSITE:
			return CAST_TO_LUA(ProductionSiteDescr, LuaProductionSiteDescription);
		case MapObjectType::MILITARYSITE:
			return CAST_TO_LUA(MilitarySiteDescr, LuaMilitarySiteDescription);
		case MapObjectType::WAREHOUSE:
			return CAST_TO_LUA(WarehouseDescr, LuaWarehouseDescription);
		case MapObjectType::MARKET:
			return CAST_TO_LUA(MarketDescr, LuaMarketDescription);
		case MapObjectType::TRAININGSITE:
			return CAST_TO_LUA(TrainingSiteDescr, LuaTrainingSiteDescription);
		default:
			return CAST_TO_LUA(BuildingDescr, LuaBuildingDescription);
		}
	} else {
		switch (descr->type()) {
		case MapObjectType::WARE:
			return CAST_TO_LUA(WareDescr, LuaWareDescription);
		case MapObjectType::WORKER:
			return CAST_TO_LUA(WorkerDescr, LuaWorkerDescription);
		case MapObjectType::CARRIER:
			return CAST_TO_LUA(WorkerDescr, LuaWorkerDescription);
		case MapObjectType::FERRY:
			return CAST_TO_LUA(WorkerDescr, LuaWorkerDescription);
		case MapObjectType::SOLDIER:
			return CAST_TO_LUA(SoldierDescr, LuaSoldierDescription);
		case MapObjectType::IMMOVABLE:
			return CAST_TO_LUA(ImmovableDescr, LuaImmovableDescription);
		default:
			return CAST_TO_LUA(MapObjectDescr, LuaMapObjectDescription);
		}
	}
}
#undef CAST_TO_LUA

/*
 * Upcast the given map object to a higher type and hand this to
 * Lua. We use this so that scripters always work with the highest class
 * object available.
 */
#define CAST_TO_LUA(k) to_lua<Lua##k>(L, new Lua##k(*dynamic_cast<k*>(mo)))
int upcasted_map_object_to_lua(lua_State* L, MapObject* mo) {
	if (!mo) {
		return 0;
	}

	switch (mo->descr().type()) {
	case MapObjectType::CRITTER:
		return CAST_TO_LUA(Bob);
	case MapObjectType::SHIP:
		return CAST_TO_LUA(Ship);
	case MapObjectType::WORKER:
		return CAST_TO_LUA(Worker);
	case MapObjectType::CARRIER:
		// TODO(sirver): not yet implemented
		return CAST_TO_LUA(Worker);
	case MapObjectType::FERRY:
		// TODO(Nordfriese): not yet implemented
		return CAST_TO_LUA(Worker);
	case MapObjectType::SOLDIER:
		return CAST_TO_LUA(Soldier);

	case MapObjectType::IMMOVABLE:
		return CAST_TO_LUA(BaseImmovable);

	case MapObjectType::FLAG:
		return CAST_TO_LUA(Flag);
	case MapObjectType::ROAD:
		return CAST_TO_LUA(Road);
	case MapObjectType::WATERWAY:
		// TODO(Nordfriese): not yet implemented
		return CAST_TO_LUA(Road);
	case MapObjectType::ROADBASE:
		// TODO(Nordfriese): not yet implemented
		return CAST_TO_LUA(Road);
	case MapObjectType::PORTDOCK:
		return CAST_TO_LUA(PortDock);

	case MapObjectType::BUILDING:
		return CAST_TO_LUA(Building);
	case MapObjectType::CONSTRUCTIONSITE:
		return CAST_TO_LUA(ConstructionSite);
	case MapObjectType::DISMANTLESITE:
		// TODO(sirver): not yet implemented.
		return CAST_TO_LUA(Building);
	case MapObjectType::WAREHOUSE:
		return CAST_TO_LUA(Warehouse);
	case MapObjectType::MARKET:
		return CAST_TO_LUA(Market);
	case MapObjectType::PRODUCTIONSITE:
		return CAST_TO_LUA(ProductionSite);
	case MapObjectType::MILITARYSITE:
		return CAST_TO_LUA(MilitarySite);
	case MapObjectType::TRAININGSITE:
		return CAST_TO_LUA(TrainingSite);
	case MapObjectType::MAPOBJECT:
	case MapObjectType::RESOURCE:
	case MapObjectType::TERRAIN:
	case MapObjectType::BATTLE:
	case MapObjectType::BOB:
	case MapObjectType::SHIP_FLEET:
	case MapObjectType::FERRY_FLEET:
	case MapObjectType::WARE:
		throw LuaError((boost::format("upcasted_map_object_to_lua: Unknown %i") %
		                static_cast<int>(mo->descr().type()))
		                  .str());
	}
	NEVER_HERE();
}

// This is used for get_ware/workers functions, when argument can be
// 'all', single ware/worker, or array of ware/workers
RequestedWareWorker parse_wares_workers_list(lua_State* L,
                                             const TribeDescr& tribe,
                                             Widelands::DescriptionIndex* single_item,
                                             std::vector<Widelands::DescriptionIndex>* item_list,
                                             bool is_ware) {
	RequestedWareWorker result = RequestedWareWorker::kUndefined;
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "One argument is required for produced_wares_count()");
	}

	/* If we have single string as an argument */
	if (lua_isstring(L, 2)) {

		std::string what = luaL_checkstring(L, -1);
		if (what != "all") {
			result = RequestedWareWorker::kSingle;
			// This is name of ware/worker
			if (is_ware) {
				*single_item = tribe.ware_index(what);
			} else {
				*single_item = tribe.worker_index(what);
			}
			if (*single_item == INVALID_INDEX) {
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
			if (item_list->back() == INVALID_INDEX) {
				report_error(L, "Unrecognized ware %s", what.c_str());
			}
		}
	}
	assert((*single_item == INVALID_INDEX) != item_list->empty());
	return result;
}

// Very similar to above function, but expects numbers for every received ware/worker
RequestedWareWorker parse_wares_workers_counted(lua_State* L,
                                                const TribeDescr& tribe,
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
			if (tribe.ware_index(luaL_checkstring(L, 2)) == INVALID_INDEX) {
				report_error(L, "Illegal ware %s", luaL_checkstring(L, 2));
			}
			ware_workers_list->insert(std::make_pair(
			   std::make_pair(tribe.ware_index(luaL_checkstring(L, 2)), Widelands::WareWorker::wwWARE),
			   luaL_checkuint32(L, 3)));
		} else {
			if (tribe.worker_index(luaL_checkstring(L, 2)) == INVALID_INDEX) {
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

#undef CAST_TO_LUA

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Interfaces
^^^^^^^^^^^^^^^^^

*/

/* RST
HasWares
--------

.. class:: HasWares

   HasWares is an interface that most :class:`PlayerImmovable` objects
   that can contain wares implement. This is at the time of this writing
   :class:`~wl.map.Flag` and :class:`~wl.map.Warehouse`.
*/

/* RST
   .. method:: get_wares(which)

      Gets the number of wares that currently reside here.

      :arg which:  can be either of

      * the string :const:`all`.
           In this case the function will return a
           :class:`table` of (ware name,amount) pairs that gives information
           about all ware information available for this object.
      * a ware name.
         In this case a single integer is returned. No check is made
         if this ware makes sense for this location, you can for example ask a
         :const:`lumberjacks_hut` for the number of :const:`granite` he has
         and he will return 0.
      * an :class:`array` of ware names.
         In this case a :class:`table` of
         (ware name,amount) pairs is returned where only the requested wares
         are listed. All other entries are :const:`nil`.

      :returns: :class:`integer` or :class:`table`
*/

/* RST
   .. method:: set_wares(which[, amount])

      Sets the wares available in this location. Either takes two arguments,
      a ware name and an amount to set it to. Or it takes a table of
      (ware name, amount) pairs. Wares are created and added to an economy out
      of thin air.

      :arg which: name of ware or (ware_name, amount) table
      :type which: :class:`string` or :class:`table`
      :arg amount: this many units will be available after the call
      :type amount: :class:`integer`
*/

/* RST
   .. attribute:: valid_wares

      (RO) A :class:`table` of (ware_name, count) if storage is somehow
      constrained in this location. For example for a
      :class:`~wl.map.ProductionSite` this is the information what wares
      and how much can be stored as inputs. For unconstrained storage (like
      :class:`~wl.map.Warehouse`) this is :const:`nil`.

      You can use this to quickly fill a building:

      .. code-block:: lua

         if b.valid_wares then b:set_wares(b.valid_wares) end
*/

/* RST
HasInputs
---------

.. class:: HasInputs

   HasInputs is an interface that some :class:`PlayerImmovable` objects
   implement. At the time of this writing these are
   :class:`~wl.map.ProductionSite` and :class:`~wl.map.TrainingSite`.
   This interface is similar to :class:`HasWares` but additionally allows
   to set workers as inputs. These workers are consumed by the production
   or trainings programm.
*/

/* RST
   .. method:: get_inputs(which)

      Gets the number of wares and workers that currently reside here
      for consumption.

      :arg which:  can be either of

      * the string :const:`all`.
           In this case the function will return a
           :class:`table` of (ware/worker name,amount) pairs that gives
           information about all ware information available for this object.
      * a ware or worker name.
           In this case a single integer is returned. No check is made
           if this ware/worker makes sense for this location, you can for example ask a
           :const:`lumberjacks_hut` for the number of :const:`granite` he has
           and he will return 0.
      * an :class:`array` of ware and worker names.
           In this case a :class:`table` of
           (ware/worker name,amount) pairs is returned where only the requested
           wares/workers are listed. All other entries are :const:`nil`.

      :returns: :class:`integer` or :class:`table`
*/

/* RST
   .. method:: set_inputs(which[, amount])

      Sets the wares/workers available in this location which will
      be consumed by the production/training programm. Either takes two arguments,
      a ware/worker name and an amount to set it to. Or it takes a table of
      (ware/worker name, amount) pairs. Wares are created and added to an
      economy out of thin air.

      :arg which: name of ware/worker or (ware/worker name, amount) table
      :type which: :class:`string` or :class:`table`
      :arg amount: this many units will be available after the call
      :type amount: :class:`integer`
*/

/* RST
   .. attribute:: valid_inputs

      (RO) A :class:`table` of (ware/worker name, count) which describes how
      many wares/workers can be stored here for consumption. For example for a
      :class:`~wl.map.ProductionSite` this is the information what wares/workers
      and can be stored in which amount as inputs.

      You can use this to quickly fill a building:

      .. code-block:: lua

         if b.valid_inputs then b:set_inputs(b.valid_inputs) end
*/

/* RST
HasWorkers
----------

.. class:: HasWorkers

   Analogon to :class:`HasWares`, but for Workers. Supported at the time
   of this writing by :class:`~wl.map.Road`, :class:`~wl.map.Warehouse`
   and :class:`~wl.map.ProductionSite`. In the case of ProductionSites,
   these methods allow access to the workers which do the work instead of
   workers which are consumed as accessed by the methods of :class:`HasInputs`.
*/

/* RST
   .. method:: get_workers(which)

      Similar to :meth:`HasWares.get_wares`.
*/

/* RST
   .. method:: set_workers(which[, amount])

      Similar to :meth:`HasWares.set_wares`.
*/

/* RST
   .. attribute:: valid_workers

      (RO) Similar to :attr:`HasWares.valid_wares` but for workers in this
      location.
*/

/* RST
HasSoldiers
------------

.. class:: HasSoldiers

   Analogon to :class:`HasWorkers`, but for Soldiers. Due to differences in
   Soldiers and implementation details in Lua this class has a slightly
   different interface than :class:`HasWorkers`. Supported at the time of this
   writing by :class:`~wl.map.Warehouse`, :class:`~wl.map.MilitarySite` and
   :class:`~wl.map.TrainingSite`.
*/

/* RST
   .. method:: get_soldiers(descr)

      Gets information about the soldiers in a location.

      :arg descr: can be either of

      * a soldier description.
         Returns an :class:`integer` which is the number of soldiers of this
         kind in this building.

         A soldier description is a :class:`array` that contains the level for
         health, attack, defense and evade (in this order). A usage example:

         .. code-block:: lua

            w:get_soldiers({0,0,0,0})

         would return the number of soldiers of level 0 in this location.

      * the string :const:`all`.
         In this case a :class:`table` of (soldier descriptions, count) is
         returned. Note that the following will not work, because Lua indexes
         tables by identity:

         .. code-block:: lua

            w:set_soldiers({0,0,0,0}, 100)
            w:get_soldiers({0,0,0,0}) -- works, returns 100
            w:get_soldiers("all")[{0,0,0,0}] -- works not, this is nil

            -- Following is a working way to check for a {0,0,0,0} soldier
            for descr,count in pairs(w:get_soldiers("all")) do
               if descr[1] == 0 and descr[2] == 0 and
                  descr[3] == 0 and descr[4] == 0 then
                     print(count)
               end
            end

      :returns: Number of soldiers that match descr or the :class:`table`
         containing all soldiers
      :rtype: :class:`integer` or :class:`table`.
*/

/* RST
   .. method:: set_soldiers(which[, amount])

      Analogous to :meth:`HasWorkers.set_workers`, but for soldiers. Instead of
      a name a :class:`array` is used to define the soldier. See
      :meth:`get_soldiers` for an example.

      Usage example:

      .. code-block:: lua

         l:set_soldiers({0,0,0,0}, 100)

      would add 100 level 0 soldiers. While

      .. code-block:: lua

         l:set_soldiers({
           [{0,0,0,0}] = 10,
           [{1,2,3,4}] = 5,
         })

      would add 10 level 0 soldier and 5 soldiers with hit point level 1,
      attack level 2, defense level 3 and evade level 4 (as long as this is
      legal for the players tribe).

      :arg which: either a table of (description, count) pairs or one
         description. In that case amount has to be specified as well.
      :type which: :class:`table` or :class:`array`.
*/

/* RST
   .. attribute:: max_soldiers

      (RO) The maximum number of soldiers that can be inside this building at
      one time. If it is not constrained, like for :class:`~wl.map.Warehouse`
      this will be :const:`nil`.
*/

/* RST
Module Classes
^^^^^^^^^^^^^^

*/

/* RST
Map
---

.. class:: Map

   Access to the map and its objects. You cannot instantiate this directly,
   instead access it via :attr:`wl.Game.map`.
*/
const char LuaMap::className[] = "Map";
const MethodType<LuaMap> LuaMap::Methods[] = {
   METHOD(LuaMap, count_conquerable_fields),
   METHOD(LuaMap, count_terrestrial_fields),
   METHOD(LuaMap, count_owned_valuable_fields),
   METHOD(LuaMap, place_immovable),
   METHOD(LuaMap, get_field),
   METHOD(LuaMap, recalculate),
   METHOD(LuaMap, recalculate_seafaring),
   METHOD(LuaMap, set_port_space),
   METHOD(LuaMap, sea_route_exists),
   METHOD(LuaMap, find_ocean_fields),
   {nullptr, nullptr},
};
const PropertyType<LuaMap> LuaMap::Properties[] = {
   PROP_RO(LuaMap, allows_seafaring), PROP_RO(LuaMap, number_of_port_spaces),
   PROP_RO(LuaMap, port_spaces),      PROP_RO(LuaMap, width),
   PROP_RO(LuaMap, height),           PROP_RO(LuaMap, player_slots),
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

      :returns: True if there are at least two port spaces that can be reached from each other.
*/
int LuaMap::get_allows_seafaring(lua_State* L) {
	lua_pushboolean(L, get_egbase(L).map().allows_seafaring());
	return 1;
}
/* RST
   .. attribute:: number_of_port_spaces

      (RO) The amount of port spaces on the map.

      :returns: An integer with the number of port spaces.
*/
int LuaMap::get_number_of_port_spaces(lua_State* L) {
	lua_pushuint32(L, get_egbase(L).map().get_port_spaces().size());
	return 1;
}

/* RST
   .. attribute:: port_spaces

      (RO) A list of coordinates for all port spaces on the map.

      :returns: A table of port space coordinates,
        like this: ``{{x = 0, y = 2}, {x = 54, y = 23}}``.
*/
int LuaMap::get_port_spaces(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const Coords& space : get_egbase(L).map().get_port_spaces()) {
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
   .. attribute:: player_slots

      (RO) This is an :class:`array` that contains :class:`~wl.map.PlayerSlots`
      for each player defined in the map.
*/
int LuaMap::get_player_slots(lua_State* L) {
	const Map& map = get_egbase(L).map();

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
	lua_pushinteger(L, get_egbase(L).mutable_map()->count_all_fields_excluding_caps(MOVECAPS_SWIM));
	return 1;
}

/* RST
   .. method:: count_owned_valuable_fields([immovable_attribute])

      (RO) Counts the number of owned valuable fields for all players.

      :arg name: *Optional*. If this is set, only count fields that have an
        immovable with the given atttribute.
      :type name: :class:`string`

     :returns: A table mapping player numbers to their number of owned fields.
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

      Returns an array with the given number of Fields so that every field is swimmable,
      and from each field a sea route to any port space exists.

      :arg number: The number of fields to find.

     :returns: :class:`array` of :class:`wl.map.Field`
*/
int LuaMap::find_ocean_fields(lua_State* L) {
	upcast(Game, game, &get_egbase(L));
	assert(game);
	const Map& map = game->map();

	std::vector<LuaMaps::LuaField*> result;
	for (uint32_t i = luaL_checkuint32(L, 2); i;) {
		const uint32_t x = game->logic_rand() % map.get_width();
		const uint32_t y = game->logic_rand() % map.get_height();
		Widelands::Coords field(x, y);
		bool success = false;
		if (map[field].maxcaps() & Widelands::MOVECAPS_SWIM) {
			for (Widelands::Coords port : map.get_port_spaces()) {
				for (const Widelands::Coords& c : map.find_portdock(port, false)) {
					Widelands::Path p;
					if (map.findpath(field, c, 0, p, CheckStepDefault(MOVECAPS_SWIM)) >= 0) {
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
   .. method:: place_immovable(name, field, from_where)

      Creates an immovable that is defined by the world (e.g. trees, rocks...)
      or a tribe (field) on a given field. If there is already an immovable on
      the field, an error is reported.

      :arg name: The name of the immovable to create
      :type name: :class:`string`
      :arg field: The immovable is created on this field.
      :type field: :class:`wl.map.Field`
      :arg from_where: "world" if the immovable is defined in the world,
         "tribes" if it is defined in the tribes.
      :type from_where: :class:`string`

      :returns: The created immovable.
*/
int LuaMap::place_immovable(lua_State* const L) {
	std::string from_where;

	const std::string objname = luaL_checkstring(L, 2);
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, 3);
	if (lua_gettop(L) > 3 && !lua_isnil(L, 4)) {
		from_where = luaL_checkstring(L, 4);
	}

	// Check if the map is still free here
	if (BaseImmovable const* const imm = c->fcoords(L).field->get_immovable()) {
		if (imm->get_size() >= BaseImmovable::SMALL) {
			report_error(L, "Node is no longer free!");
		}
	}

	// The immovable type might not have been loaded yet
	try {
		Notifications::publish(
		   NoteMapObjectDescription(objname, NoteMapObjectDescription::LoadType::kObject));
	} catch (const Widelands::GameDataError&) {
		report_error(L, "Unknown immovable <%s>", objname.c_str());
	}

	EditorGameBase& egbase = get_egbase(L);
	BaseImmovable* m = nullptr;
	if (from_where == "world") {
		DescriptionIndex const imm_idx = egbase.world().get_immovable_index(objname);
		if (imm_idx == Widelands::INVALID_INDEX) {
			report_error(L, "Unknown world immovable <%s>", objname.c_str());
		}

		m = &egbase.create_immovable(
		   c->coords(), imm_idx, MapObjectDescr::OwnerType::kWorld, nullptr /* owner */);
	} else if (from_where == "tribes") {
		DescriptionIndex const imm_idx = egbase.tribes().immovable_index(objname);
		if (imm_idx == Widelands::INVALID_INDEX) {
			report_error(L, "Unknown tribes immovable <%s>", objname.c_str());
		}

		m = &egbase.create_immovable(
		   c->coords(), imm_idx, MapObjectDescr::OwnerType::kTribe, nullptr /* owner */);
	} else {
		report_error(
		   L, "There are no immovables for <%s>. Use \"world\" or \"tribes\"", from_where.c_str());
	}

	return LuaMaps::upcasted_map_object_to_lua(L, m);
}

/* RST
   .. method:: get_field(x, y)

      Returns a :class:`wl.map.Field` object of the given index.
*/
int LuaMap::get_field(lua_State* L) {
	uint32_t x = luaL_checkuint32(L, 2);
	uint32_t y = luaL_checkuint32(L, 3);

	const Map& map = get_egbase(L).map();

	if (x >= static_cast<uint32_t>(map.get_width())) {
		report_error(L, "x coordinate out of range!");
	}
	if (y >= static_cast<uint32_t>(map.get_height())) {
		report_error(L, "y coordinate out of range!");
	}

	return to_lua<LuaMaps::LuaField>(L, new LuaMaps::LuaField(x, y));
}

/* RST
   .. method:: recalculate()

      This map recalculates the whole map state: height of fields, buildcaps,
      whether the map allows seafaring and so on. You only need to call this
      function if you changed :any:`raw_height` in any way.
*/
// TODO(unknown): do we really want this function?
int LuaMap::recalculate(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
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
      Returns false if the port space couldn't be set.

      :arg x: The x coordinate of the port space to set/unset.
      :type x: :class:`int`
      :arg y: The y coordinate of the port space to set/unset.
      :type y: :class:`int`
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
	lua_pushboolean(L, success);
	return 1;
}

/* RST
   .. method:: sea_route_exists(field, port)

      Returns whether a sea route exists from the given field to the given port space.

      :arg field: The field where to start
      :type field: :class:`wl.map.Field`
      :arg port: The port space to find
      :type port: :class:`wl.map.Field`

      :rtype: :class:`bool`
*/
int LuaMap::sea_route_exists(lua_State* L) {
	const Widelands::Map& map = get_egbase(L).map();
	const Widelands::FCoords f_start = (*get_user_class<LuaMaps::LuaField>(L, 2))->fcoords(L);
	const Widelands::FCoords f_port = (*get_user_class<LuaMaps::LuaField>(L, 3))->fcoords(L);
	for (const Widelands::Coords& c : map.find_portdock(f_port, false)) {
		Widelands::Path p;
		if (map.findpath(f_start, c, 0, p, CheckStepDefault(MOVECAPS_SWIM)) >= 0) {
			lua_pushboolean(L, true);
			return 1;
		}
	}
	lua_pushboolean(L, false);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

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
   {nullptr, nullptr, nullptr},
};

void LuaTribeDescription::__persist(lua_State* L) {
	const TribeDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaTribeDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	if (!Widelands::tribe_exists(name)) {
		report_error(L, "Tribe '%s' does not exist", name.c_str());
	}
	set_description_pointer(
	   get_egbase(L).tribes().get_tribe_descr(get_egbase(L).mutable_tribes()->load_tribe(name)));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: buildings

      (RO) an array of :class:`LuaBuildingDescription` with all the buildings that the tribe can
      use, casted to their appropriate subclasses.
*/
int LuaTribeDescription::get_buildings(lua_State* L) {
	const TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (DescriptionIndex building : tribe.buildings()) {
		lua_pushinteger(L, ++counter);
		upcasted_map_object_descr_to_lua(L, tribe.get_building_descr(building));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: carrier

         (RO) the :class:`string` internal name of the carrier type that this tribe uses
*/

int LuaTribeDescription::get_carrier(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(get()->carrier())->name());
	return 1;
}

/* RST
   .. attribute:: carrier2

         (RO) the :class:`string` internal name of the carrier2 type that this tribe uses.
              e.g. 'atlanteans_horse'
*/

int LuaTribeDescription::get_carrier2(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(get()->carrier2())->name());
	return 1;
}

/* RST
   .. attribute:: ferry

         (RO) the :class:`string` internal name of the ferry type that this tribe uses.
              e.g. 'atlanteans_ferry'
*/

int LuaTribeDescription::get_ferry(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(get()->ferry())->name());
	return 1;
}

/* RST
   .. attribute:: descname

         (RO) a :class:`string` with the tribe's localized name
*/

int LuaTribeDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: geologist

         (RO) the :class:`string` internal name of the geologist type that this tribe uses
*/

int LuaTribeDescription::get_geologist(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(get()->geologist())->name());
	return 1;
}

/* RST
   .. attribute:: immovables

      (RO) an array of :class:`LuaImmovableDescription` with all the immovables that the tribe can
      use.
*/
int LuaTribeDescription::get_immovables(lua_State* L) {
	const TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (DescriptionIndex immovable : tribe.immovables()) {
		lua_pushinteger(L, ++counter);
		to_lua<LuaImmovableDescription>(
		   L, new LuaImmovableDescription(tribe.get_immovable_descr(immovable)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: resource_indicators

      (RO) the table `resource_indicators` as defined in the tribe's `tribename.lua`.
      See `data/tribes/atlanteans.lua` for more information on the table structure.
*/
int LuaTribeDescription::get_resource_indicators(lua_State* L) {
	const TribeDescr& tribe = *get();
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
   .. attribute:: name

         (RO) a :class:`string` with the tribe's internal name
*/

int LuaTribeDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: port

         (RO) the :class:`string` internal name of the port type that this tribe uses
*/

int LuaTribeDescription::get_port(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_building_descr(get()->port())->name());
	return 1;
}

/* RST
   .. attribute:: ship

         (RO) the :class:`string` internal name of the ship type that this tribe uses
*/

int LuaTribeDescription::get_ship(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_ship_descr(get()->ship())->name());
	return 1;
}

/* RST
   .. attribute:: soldier

         (RO) the :class:`string` internal name of the soldier type that this tribe uses
*/

int LuaTribeDescription::get_soldier(lua_State* L) {
	lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(get()->soldier())->name());
	return 1;
}

/* RST
   .. attribute:: wares

         (RO) an array of :class:`LuaWareDescription` with all the wares that the tribe can use.
*/
int LuaTribeDescription::get_wares(lua_State* L) {
	const TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (DescriptionIndex ware : tribe.wares()) {
		lua_pushinteger(L, ++counter);
		to_lua<LuaWareDescription>(L, new LuaWareDescription(tribe.get_ware_descr(ware)));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: workers

         (RO) an array of :class:`LuaWorkerDescription` with all the workers that the tribe can use,
              casted to their appropriate subclasses.
*/
int LuaTribeDescription::get_workers(lua_State* L) {
	const TribeDescr& tribe = *get();
	lua_newtable(L);
	int counter = 0;
	for (DescriptionIndex worker : tribe.workers()) {
		lua_pushinteger(L, ++counter);
		upcasted_map_object_descr_to_lua(L, tribe.get_worker_descr(worker));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. method:: has_building(buildingname)

      Returns true if buildingname is a building and the tribe can use it.

      :returns: :const:`true` or :const:`false`
      :rtype: :class:`bool`
*/
int LuaTribeDescription::has_building(lua_State* L) {
	const std::string buildingname = luaL_checkstring(L, 2);
	const DescriptionIndex index = get_egbase(L).tribes().building_index(buildingname);
	lua_pushboolean(L, get()->has_building(index));
	return 1;
}

/* RST
   .. method:: has_ware(warename)

      Returns true if warename is a ware and the tribe uses it.

      :returns: :const:`true` or :const:`false`
      :rtype: :class:`bool`
*/
int LuaTribeDescription::has_ware(lua_State* L) {
	const std::string warename = luaL_checkstring(L, 2);
	const DescriptionIndex index = get_egbase(L).tribes().ware_index(warename);
	lua_pushboolean(L, get()->has_ware(index));
	return 1;
}

/* RST
   .. method:: has_worker(workername)

      Returns true if workername is a worker and the tribe can use it.

      :returns: :const:`true` or :const:`false`
      :rtype: :class:`bool`
*/
int LuaTribeDescription::has_worker(lua_State* L) {
	const std::string workername = luaL_checkstring(L, 2);
	const DescriptionIndex index = get_egbase(L).tribes().worker_index(workername);
	lua_pushboolean(L, get()->has_worker(index));
	return 1;
}

/* RST
MapObjectDescription
--------------------

.. class:: MapObjectDescription

   A static description of a tribe's map object, so it can be used in help files
   without having to access an actual object on the map.
   This class contains the properties that are common to all map objects such as buildings or wares.

   The dynamic MapObject class corresponding to this class is the base class for all Objects in
   widelands, including immovables and Bobs. This class can't be instantiated directly, but provides
   the base for all others.
*/
const char LuaMapObjectDescription::className[] = "MapObjectDescription";
const MethodType<LuaMapObjectDescription> LuaMapObjectDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaMapObjectDescription> LuaMapObjectDescription::Properties[] = {
   PROP_RO(LuaMapObjectDescription, descname),  PROP_RO(LuaMapObjectDescription, helptext_script),
   PROP_RO(LuaMapObjectDescription, icon_name), PROP_RO(LuaMapObjectDescription, name),
   PROP_RO(LuaMapObjectDescription, type_name), {nullptr, nullptr, nullptr},
};

// Only base classes can be persisted.
void LuaMapObjectDescription::__persist(lua_State*) {
	NEVER_HERE();
}

void LuaMapObjectDescription::__unpersist(lua_State*) {
	NEVER_HERE();
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: descname

         (RO) a :class:`string` with the map object's localized name
*/

int LuaMapObjectDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: helptext_script

         (RO) The path and filename to the helptext script. Can be empty.
*/
int LuaMapObjectDescription::get_helptext_script(lua_State* L) {
	lua_pushstring(L, get()->helptext_script());
	return 1;
}

/* RST
   .. attribute:: icon_name

         (RO) the filename for the menu icon.
*/
int LuaMapObjectDescription::get_icon_name(lua_State* L) {
	lua_pushstring(L, get()->icon_filename());
	return 1;
}

/* RST
   .. attribute:: name

         (RO) a :class:`string` with the map object's internal name
*/

int LuaMapObjectDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: type_name

         (RO) the map object's type as a string. Map object types are
         organized in a hierarchy, where an element that's lower in the
         hierarchy has all the properties of the higher-placed types,
         as well as its own additional properties. Any map object's
         description that isn't linked below can be accessed via its
         higher types, e.g. a ``bob`` is a
         :class:`general map object <MapObjectDescription>`, and a
         ``carrier`` is a :class:`worker <WorkerDescription>` as well as a
         general map object. Possible values are:

         * **Bobs:** Bobs are map objects that can move around the map.
           Bob types are:

           * :class:`bob <BobDescription>`, the abstract base type for
             all bobs,
           * :class:`critter <CritterDescription>`, animals that aren't
             controlled by any tribe,
           * :class:`ship <ShipDescription>`, a sea-going vessel
             belonging to a tribe that can ferry wares or an expedition,
           * :class:`worker <WorkerDescription>`, a worker belonging to
             a tribe,
           * :class:`carrier <CarrierDescription>`, a specialized
             worker for carrying items along a road,
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
               special tapes of warehouses, but they belong to the same
               class,
             * :class:`militarysite <MilitarySiteDescription>`, a
               building manned by soldiers to expand a tribe's territory,
             * :class:`productionsite <ProductionSiteDescription>`, the
               most common type of building, which can produce wares,
             * :class:`trainingsite <TrainingSiteDescription>`, a
               specialized productionsite for improving soldiers.

           * **Other Immovables:** Specialized immovables that aren't buildings:

             * :class:`flag <FlagDescription>`, a flag that can hold
               wares for transport,
             * :class:`road <RoadDescription>`, a road or waterway
               connecting two flags,
             * :class:`portdock <PortdockDescription>`, a 'parking space'
               on water terrain where ships can load/unload wares and
               workers. A portdock is invisible to the player and one is
               automatically placed next to each port building.

         * **Abstract:** These types are abstract map objects that are used by the engine and are
           not visible on the map.

           * :class:`battle <BattleDescription>`, holds information
             about two soldiers in a fight,
           * :class:`fleet <FleetDescription>`, holds information for
             managing ships.
*/
int LuaMapObjectDescription::get_type_name(lua_State* L) {
	lua_pushstring(L, to_string(get()->type()));
	return 1;
}

/* RST
ImmovableDescription
--------------------

.. class:: ImmovableDescription

   Child of: :class:`MapObjectDescription`

   A static description of a :class:`base immovable <BaseImmovable>`, so it can be used in help
   files without having to access an actual immovable on the map. See also
   :class:`MapObjectDescription` for more properties.
*/
const char LuaImmovableDescription::className[] = "ImmovableDescription";
const MethodType<LuaImmovableDescription> LuaImmovableDescription::Methods[] = {
   METHOD(LuaImmovableDescription, has_attribute),
   METHOD(LuaImmovableDescription, probability_to_grow),
   {nullptr, nullptr},
};
const PropertyType<LuaImmovableDescription> LuaImmovableDescription::Properties[] = {
   PROP_RO(LuaImmovableDescription, species),
   PROP_RO(LuaImmovableDescription, buildcost),
   PROP_RO(LuaImmovableDescription, becomes),
   PROP_RO(LuaImmovableDescription, terrain_affinity),
   PROP_RO(LuaImmovableDescription, owner_type),
   PROP_RO(LuaImmovableDescription, size),
   {nullptr, nullptr, nullptr},
};

void LuaImmovableDescription::__persist(lua_State* L) {
	const ImmovableDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaImmovableDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const World& world = get_egbase(L).world();
	DescriptionIndex idx = world.get_immovable_index(name);
	if (idx != INVALID_INDEX) {
		set_description_pointer(world.get_immovable_descr(idx));
	} else {
		const Tribes& tribes = get_egbase(L).tribes();
		idx = tribes.safe_immovable_index(name);
		set_description_pointer(tribes.get_immovable_descr(idx));
	}
}

/* RST
   .. attribute:: species

         the species name of a tree for editor lists

         (RO) the localized species name of the immovable, or an empty string if it has none.
*/
int LuaImmovableDescription::get_species(lua_State* L) {
	lua_pushstring(L, get()->species());
	return 1;
}

/* RST
   .. attribute:: buildcost

         (RO) a table of ware-to-count pairs, describing the build cost for the
         immovable.
*/
int LuaImmovableDescription::get_buildcost(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->buildcost(), MapObjectType::WARE);
}

/* RST
   .. attribute:: becomes

         (RO) a table of map object names that this immovable can turn into, e.g.
         ``{"atlanteans_ship"}`` or ``{"deadtree2", "fallentree"}``.
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

         returns the terrain affinity values for this immovable

         (RO) a table containing numbers labeled as pickiness (uint), preferred_fertility (uint),
         preferred_humidity (uint), and preferred_temperature (uint),
         or nil if the immovable has no terrain affinity.
*/
int LuaImmovableDescription::get_terrain_affinity(lua_State* L) {
	if (get()->has_terrain_affinity()) {
		const TerrainAffinity& affinity = get()->terrain_affinity();
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
   .. attribute:: owner_type

         the owner type of this immovable

         (RO) "world" for world immovables and "tribe" for tribe immovables.
*/
int LuaImmovableDescription::get_owner_type(lua_State* L) {
	switch (get()->owner_type()) {
	case MapObjectDescr::OwnerType::kWorld:
		lua_pushstring(L, "world");
		break;
	case MapObjectDescr::OwnerType::kTribe:
		lua_pushstring(L, "tribe");
		break;
	}
	return 1;
}

/* RST
   .. attribute:: size

      (RO) The size of this immovable. Can be either of

      * :const:`none` -- Example: mushrooms. Immovables will be destroyed when
         something else is built on this field.
      * :const:`small` -- Example: trees or flags
      * :const:`medium` -- Example: Medium sized buildings
      * :const:`big` -- Example: Big sized buildings or rocks
*/
int LuaImmovableDescription::get_size(lua_State* L) {
	try {
		lua_pushstring(L, BaseImmovable::size_to_string(get()->get_size()));
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
   .. method:: has_attribute

      whether the immovable has the given attribute

      :arg attribute_name: The attribute that we are checking for.
      :type attribute_name: :class:`string`

         (RO) true if the immovable has the attribute, false otherwise.
*/
int LuaImmovableDescription::has_attribute(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const uint32_t attribute_id = get()->get_attribute_id(luaL_checkstring(L, 2));
	lua_pushboolean(L, get()->has_attribute(attribute_id));
	return 1;
}

/* RST
   .. method:: probability_to_grow

      :arg terrain: The terrain that we are checking the probability for.
      :type terrain: :class:`wl.map.TerrainDescription`

      (RO) A double describing the probability that this tree will grow on the given terrain.
           Returns nil if this immovable tree has no terrain affinity (all trees should have one).
*/
int LuaImmovableDescription::probability_to_grow(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	if (get()->has_terrain_affinity()) {
		const TerrainDescription* terrain =
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

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`

   A static description of a tribe's building, so it can be used in help files
   without having to access an actual building on the map.
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
   PROP_RO(LuaBuildingDescription, returned_wares),
   PROP_RO(LuaBuildingDescription, returned_wares_enhanced),
   PROP_RO(LuaBuildingDescription, vision_range),
   PROP_RO(LuaBuildingDescription, workarea_radius),
   {nullptr, nullptr, nullptr},
};

void LuaBuildingDescription::__persist(lua_State* L) {
	const BuildingDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaBuildingDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Tribes& tribes = get_egbase(L).tribes();
	DescriptionIndex idx = tribes.safe_building_index(name.c_str());
	set_description_pointer(tribes.get_building_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: buildcost

         (RO) a list of ware build cost for the building.
*/
int LuaBuildingDescription::get_buildcost(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->buildcost(), MapObjectType::WARE);
}

/* RST
   .. attribute:: buildable

         (RO) true if the building can be built.
*/
int LuaBuildingDescription::get_buildable(lua_State* L) {
	lua_pushboolean(L, get()->is_buildable());
	return 1;
}

/* RST
   .. attribute:: conquers

         (RO) the conquer range of the building as an int.
*/
int LuaBuildingDescription::get_conquers(lua_State* L) {
	lua_pushinteger(L, get()->get_conquers());
	return 1;
}

/* RST
   .. attribute:: destructible

         (RO) true if the building is destructible.
*/
int LuaBuildingDescription::get_destructible(lua_State* L) {
	lua_pushboolean(L, get()->is_destructible());
	return 1;
}

/* RST
   .. attribute:: enhanced

         (RO) true if the building is enhanced from another building.
*/
int LuaBuildingDescription::get_enhanced(lua_State* L) {
	lua_pushboolean(L, get()->is_enhanced());
	return 1;
}

/* RST
   .. attribute:: enhanced_from

         (RO) returns the building that this was enhanced from, or nil if this isn't an enhanced
         building.
*/
int LuaBuildingDescription::get_enhanced_from(lua_State* L) {
	if (get()->is_enhanced()) {
		const DescriptionIndex& enhanced_from = get()->enhanced_from();
		EditorGameBase& egbase = get_egbase(L);
		assert(egbase.tribes().building_exists(enhanced_from));
		return upcasted_map_object_descr_to_lua(L, egbase.tribes().get_building_descr(enhanced_from));
	}
	lua_pushnil(L);
	return 0;
}

/* RST
   .. attribute:: enhancement_cost

         (RO) a list of ware cost for enhancing to this building type.
*/
int LuaBuildingDescription::get_enhancement_cost(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->enhancement_cost(), MapObjectType::WARE);
}

/* RST
   .. attribute:: enhancement

      (RO) a building description that this building can enhance to.
*/
int LuaBuildingDescription::get_enhancement(lua_State* L) {
	const DescriptionIndex enhancement = get()->enhancement();
	if (enhancement == INVALID_INDEX) {
		return 0;
	}
	return upcasted_map_object_descr_to_lua(
	   L, get_egbase(L).tribes().get_building_descr(enhancement));
}

/* RST
   .. attribute:: is_mine

         (RO) true if the building is a mine.
*/
int LuaBuildingDescription::get_is_mine(lua_State* L) {
	lua_pushboolean(L, get()->get_ismine());
	return 1;
}

/* RST
   .. attribute:: is_port

         (RO) true if the building is a port.
*/
int LuaBuildingDescription::get_is_port(lua_State* L) {
	lua_pushboolean(L, get()->get_isport());
	return 1;
}

/* RST
   .. attribute:: size

      (RO) The size of this building. Can be either of

      * :const:`small` -- Small sized buildings
      * :const:`medium` -- Medium sized buildings
      * :const:`big` -- Big sized buildings
*/
int LuaBuildingDescription::get_size(lua_State* L) {
	try {
		lua_pushstring(L, BaseImmovable::size_to_string(get()->get_size()));
	} catch (std::exception&) {
		report_error(L, "Unknown size %i in LuaBuildingDescription::get_size: %s", get()->get_size(),
		             get()->name().c_str());
	}
	return 1;
}

/* RST
   .. attribute:: returned_wares

         (RO) a list of wares returned upon dismantling.
*/
int LuaBuildingDescription::get_returned_wares(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->returned_wares(), MapObjectType::WARE);
}

/* RST
   .. attribute:: returned_wares_enhanced

         (RO) a list of wares returned upon dismantling an enhanced building.
*/
int LuaBuildingDescription::get_returned_wares_enhanced(lua_State* L) {
	return wares_or_workers_map_to_lua(L, get()->returned_wares_enhanced(), MapObjectType::WARE);
}

/* RST
   .. attribute:: vision range

         (RO) the vision_range of the building as an int.
*/
int LuaBuildingDescription::get_vision_range(lua_State* L) {
	lua_pushinteger(L, get()->vision_range());
	return 1;
}

/* RST
   .. attribute:: workarea_radius

         (RO) the first workarea_radius of the building as an int,
              nil in case bulding has no workareas
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

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`

   A static description of a tribe's constructionsite, so it can be used in help files
   without having to access an actual building on the map.
   See the parent classes for more properties.
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

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`

   A static description of a tribe's dismantlesite, so it can be used in help files
   without having to access an actual building on the map.
   See the parent classes for more properties.
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

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`

   A static description of a tribe's productionsite, so it can be used in help files
   without having to access an actual building on the map.
   This class contains the properties for productionsites that have workers.
   For militarysites and trainingsites, please use the subclasses.
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
   PROP_RO(LuaProductionSiteDescription, output_ware_types),
   PROP_RO(LuaProductionSiteDescription, output_worker_types),
   PROP_RO(LuaProductionSiteDescription, production_programs),
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

      (RO) An array with :class:`LuaWareDescription` containing the wares that
      the productionsite needs for its production.
*/
int LuaProductionSiteDescription::get_inputs(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& input_ware : get()->input_wares()) {
		lua_pushint32(L, index++);
		const WareDescr* descr = get_egbase(L).tribes().get_ware_descr(input_ware.first);
		to_lua<LuaWareDescription>(L, new LuaWareDescription(descr));
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: output_ware_types

      (RO) An array with :class:`LuaWareDescription` containing the wares that
      the productionsite can produce.
*/
int LuaProductionSiteDescription::get_output_ware_types(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& ware_index : get()->output_ware_types()) {
		lua_pushint32(L, index++);
		const WareDescr* descr = get_egbase(L).tribes().get_ware_descr(ware_index);
		to_lua<LuaWareDescription>(L, new LuaWareDescription(descr));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: output_worker_types

      (RO) An array with :class:`LuaWorkerDescription` containing the workers that
      the productionsite can produce.
*/
int LuaProductionSiteDescription::get_output_worker_types(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& worker_index : get()->output_worker_types()) {
		lua_pushint32(L, index++);
		const WorkerDescr* descr = get_egbase(L).tribes().get_worker_descr(worker_index);
		to_lua<LuaWorkerDescription>(L, new LuaWorkerDescription(descr));
		lua_rawset(L, -3);
	}

	return 1;
}

/* RST
   .. attribute:: production_programs

      (RO) An array with the production program names as string.
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
   .. attribute:: working_positions

      (RO) An array with :class:`WorkerDescription` containing the workers that
      can work here with their multiplicity, i.e. for a atlantean mine this
      would be { miner, miner, miner }.
*/
int LuaProductionSiteDescription::get_working_positions(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const auto& positions_pair : get()->working_positions()) {
		int amount = positions_pair.second;
		while (amount > 0) {
			lua_pushint32(L, index++);
			const WorkerDescr* descr = get_egbase(L).tribes().get_worker_descr(positions_pair.first);
			to_lua<LuaWorkerDescription>(L, new LuaWorkerDescription(descr));
			lua_settable(L, -3);
			--amount;
		}
	}
	return 1;
}

/* RST
   .. attribute:: consumed_wares_workers

      :arg program_name: the name of the production program that we want to get the consumed wares
         for :type tribename: :class:`string`

      (RO) Returns a table of {{ware name}, ware amount} for the wares consumed by this production
      program. Multiple entries in {ware name} are alternatives (OR logic)).
*/
int LuaProductionSiteDescription::consumed_wares_workers(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const ProductionProgram& program = *programs.at(program_name);
		lua_newtable(L);
		int counter = 0;
		for (const auto& group : program.consumed_wares_workers()) {
			lua_pushuint32(L, ++counter);
			lua_newtable(L);
			for (const auto& entry : group.first) {
				const DescriptionIndex& index = entry.first;
				if (entry.second == wwWARE) {
					lua_pushstring(L, get_egbase(L).tribes().get_ware_descr(index)->name());
				} else {
					lua_pushstring(L, get_egbase(L).tribes().get_worker_descr(index)->name());
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
   .. attribute:: produced_wares

      :arg program_name: the name of the production program that we want to get the produced wares
         for :type tribename: :class:`string`

         (RO) Returns a table of {ware name, ware amount} for the wares produced by this production
         program
*/
int LuaProductionSiteDescription::produced_wares(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const ProductionProgram& program = *programs.at(program_name);
		return wares_or_workers_map_to_lua(L, program.produced_wares(), MapObjectType::WARE);
	}
	return 1;
}

/* RST
   .. attribute:: recruited_workers

      :arg program_name: the name of the production program that we want to get the recruited
         workers for :type tribename: :class:`string`

         (RO) Returns a table of {worker name, worker amount} for the workers recruited
         by this production program
*/
int LuaProductionSiteDescription::recruited_workers(lua_State* L) {
	std::string program_name = luaL_checkstring(L, -1);
	const Widelands::ProductionSiteDescr::Programs& programs = get()->programs();
	if (programs.count(program_name) == 1) {
		const ProductionProgram& program = *programs.at(program_name);
		return wares_or_workers_map_to_lua(L, program.recruited_workers(), MapObjectType::WORKER);
	}
	return 1;
}

/* RST
MilitarySiteDescription
-----------------------

.. class:: MilitarySiteDescription

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`

   A static description of a tribe's militarysite, so it can be used in help files
   without having to access an actual building on the map.
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

      (RO) The number of health healed per second by the militarysite
*/
int LuaMilitarySiteDescription::get_heal_per_second(lua_State* L) {
	const MilitarySiteDescr* descr = get();
	lua_pushinteger(L, descr->get_heal_per_second());
	return 1;
}

/* RST
   .. attribute:: max_number_of_soldiers

      (RO) The number of soldiers that can be garrisoned at the militarysite
*/
int LuaMilitarySiteDescription::get_max_number_of_soldiers(lua_State* L) {
	const MilitarySiteDescr* descr = get();
	lua_pushinteger(L, descr->get_max_number_of_soldiers());
	return 1;
}

/* RST
TrainingSiteDescription
-----------------------

.. class:: TrainingSiteDescription

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`, :class:`ProductionSiteDescription`

   A static description of a tribe's trainingsite, so it can be used in help files
   without having to access an actual building on the map.
   A training site can train some or all of a soldier's properties (Attack, Defense, Evade and
   Health). See the parent classes for more properties.
*/
const char LuaTrainingSiteDescription::className[] = "TrainingSiteDescription";
const MethodType<LuaTrainingSiteDescription> LuaTrainingSiteDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaTrainingSiteDescription> LuaTrainingSiteDescription::Properties[] = {
   PROP_RO(LuaTrainingSiteDescription, food_attack),
   PROP_RO(LuaTrainingSiteDescription, food_defense),
   PROP_RO(LuaTrainingSiteDescription, food_evade),
   PROP_RO(LuaTrainingSiteDescription, food_health),
   PROP_RO(LuaTrainingSiteDescription, max_attack),
   PROP_RO(LuaTrainingSiteDescription, max_defense),
   PROP_RO(LuaTrainingSiteDescription, max_evade),
   PROP_RO(LuaTrainingSiteDescription, max_health),
   PROP_RO(LuaTrainingSiteDescription, max_number_of_soldiers),
   PROP_RO(LuaTrainingSiteDescription, min_attack),
   PROP_RO(LuaTrainingSiteDescription, min_defense),
   PROP_RO(LuaTrainingSiteDescription, min_evade),
   PROP_RO(LuaTrainingSiteDescription, min_health),
   PROP_RO(LuaTrainingSiteDescription, weapons_attack),
   PROP_RO(LuaTrainingSiteDescription, weapons_defense),
   PROP_RO(LuaTrainingSiteDescription, weapons_evade),
   PROP_RO(LuaTrainingSiteDescription, weapons_health),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: food_attack

      (RO) A table of tables with food ware names used for Attack training,
           e.g. {{"barbarians_bread"}, {"fish", "meat"}}
*/
int LuaTrainingSiteDescription::get_food_attack(lua_State* L) {
	return food_list_to_lua(L, get()->get_food_attack());
}

/* RST
   .. attribute:: food_defense

      (RO) A table of tables with food ware names used for Defense training,
           e.g. {{"barbarians_bread"}, {"fish", "meat"}}
*/
int LuaTrainingSiteDescription::get_food_defense(lua_State* L) {
	return food_list_to_lua(L, get()->get_food_defense());
}

/* RST
   .. attribute:: food_evade

      (RO) A table of tables with food ware names used for Evade training,
           e.g. {{"barbarians_bread"}, {"fish", "meat"}}
*/
int LuaTrainingSiteDescription::get_food_evade(lua_State* L) {
	return food_list_to_lua(L, get()->get_food_evade());
}

/* RST
   .. attribute:: food_health

      (RO) A table of tables with food ware names used for Health training,
           e.g. {{"barbarians_bread"}, {"fish", "meat"}}
*/
int LuaTrainingSiteDescription::get_food_health(lua_State* L) {
	return food_list_to_lua(L, get()->get_food_health());
}

/* RST
   .. attribute:: max_attack

      (RO) The number of attack points that a soldier can train
*/
int LuaTrainingSiteDescription::get_max_attack(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_attack()) {
		lua_pushinteger(L, descr->get_max_level(TrainingAttribute::kAttack));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_defense

      (RO) The number of defense points that a soldier can train
*/
int LuaTrainingSiteDescription::get_max_defense(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_defense()) {
		lua_pushinteger(L, descr->get_max_level(TrainingAttribute::kDefense));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_evade

      (RO) The number of evade points that a soldier can train
*/
int LuaTrainingSiteDescription::get_max_evade(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_evade()) {
		lua_pushinteger(L, descr->get_max_level(TrainingAttribute::kEvade));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_health

      (RO) The number of health points that a soldier can train
*/
int LuaTrainingSiteDescription::get_max_health(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_health()) {
		lua_pushinteger(L, descr->get_max_level(TrainingAttribute::kHealth));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: max_number_of_soldiers

      (RO) The number of soldiers that can be garrisoned at the trainingsite
*/
int LuaTrainingSiteDescription::get_max_number_of_soldiers(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	lua_pushinteger(L, descr->get_max_number_of_soldiers());
	return 1;
}

/* RST
   .. attribute:: min_attack

      (RO) The number of attack points that a soldier starts training with
*/
int LuaTrainingSiteDescription::get_min_attack(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_attack()) {
		lua_pushinteger(L, descr->get_min_level(TrainingAttribute::kAttack));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_defense

      (RO) The number of defense points that a soldier starts training with
*/
int LuaTrainingSiteDescription::get_min_defense(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_defense()) {
		lua_pushinteger(L, descr->get_min_level(TrainingAttribute::kDefense));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_evade

      (RO) The number of evade points that a soldier starts training with
*/
int LuaTrainingSiteDescription::get_min_evade(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_evade()) {
		lua_pushinteger(L, descr->get_min_level(TrainingAttribute::kEvade));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: min_health

      (RO) The number of health points that a soldier starts training with
*/
int LuaTrainingSiteDescription::get_min_health(lua_State* L) {
	const TrainingSiteDescr* descr = get();
	if (descr->get_train_health()) {
		lua_pushinteger(L, descr->get_min_level(TrainingAttribute::kHealth));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: weapons_attack

      (RO) A table with weapon ware names used for Attack training
*/
int LuaTrainingSiteDescription::get_weapons_attack(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const std::string& weaponname : get()->get_weapons_attack()) {
		lua_pushuint32(L, ++counter);
		lua_pushstring(L, weaponname);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: weapons_defense

      (RO) A table with weapon ware names used for Defense training
*/
int LuaTrainingSiteDescription::get_weapons_defense(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const std::string& weaponname : get()->get_weapons_defense()) {
		lua_pushuint32(L, ++counter);
		lua_pushstring(L, weaponname);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: weapons_evade

      (RO) A table with weapon ware names used for Evade training
*/
int LuaTrainingSiteDescription::get_weapons_evade(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const std::string& weaponname : get()->get_weapons_evade()) {
		lua_pushuint32(L, ++counter);
		lua_pushstring(L, weaponname);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: weapons_health

      (RO) A table with weapon ware names used for Health training
*/
int LuaTrainingSiteDescription::get_weapons_health(lua_State* L) {
	lua_newtable(L);
	int counter = 0;
	for (const std::string& weaponname : get()->get_weapons_health()) {
		lua_pushuint32(L, ++counter);
		lua_pushstring(L, weaponname);
		lua_settable(L, -3);
	}
	return 1;
}

/* RST
WarehouseDescription
--------------------

.. class:: WarehouseDescription

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`

   A static description of a tribe's warehouse, so it can be used in help files
   without having to access an actual building on the map.
   Note that headquarters are also warehouses.
   A warehouse keeps people, animals and wares.
   See the parent classes for more properties.
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

      (RO) The number of health healed per second by the warehouse
*/
int LuaWarehouseDescription::get_heal_per_second(lua_State* L) {
	const WarehouseDescr* descr = get();
	lua_pushinteger(L, descr->get_heal_per_second());
	return 1;
}

/* RST
MarketDescription
-----------------

.. class:: MarketDescription

   Child of: :class:`MapObjectDescription`, :class:`ImmovableDescription`,
   :class:`BuildingDescription`

   A static description of a tribe's market, so it can be used in help files
   without having to access an actual building on the map. A Market is used for
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

   Child of: :class:`MapObjectDescription`

   A static description of a tribe's ship, so it can be used in help files
   without having to access an actual instance of the ship on the map.
   See also :class:`MapObjectDescription` for more properties.
*/
const char LuaShipDescription::className[] = "ShipDescription";
const MethodType<LuaShipDescription> LuaShipDescription::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaShipDescription> LuaShipDescription::Properties[] = {
   {nullptr, nullptr, nullptr},
};

void LuaShipDescription::__persist(lua_State* L) {
	const ShipDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaShipDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Tribes& tribes = get_egbase(L).tribes();
	DescriptionIndex idx = tribes.safe_ship_index(name.c_str());
	set_description_pointer(tribes.get_ship_descr(idx));
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

   Child of: :class:`MapObjectDescription`

   A static description of a tribe's ware, so it can be used in help files
   without having to access an actual instance of the ware on the map.
   See also :class:`MapObjectDescription` for more properties.
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
	const WareDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaWareDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Tribes& tribes = get_egbase(L).tribes();
	DescriptionIndex idx = tribes.safe_ware_index(name.c_str());
	set_description_pointer(tribes.get_ware_descr(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. method:: consumers(tribename)

      Returns a list of buildings for the 'tribe' that consume this ware.
      Loads the tribe if it hasn't been loaded yet.

      :arg tribename: the name of the tribe that this ware gets checked for
      :type tribename: :class:`string`

      (RO) An array with :class:`LuaBuildingDescription` with buildings that
      need this ware for their production.
*/
int LuaWareDescription::consumers(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::TribeDescr& tribe = get_tribe_descr(L, luaL_checkstring(L, 2));

	lua_newtable(L);
	int index = 1;
	for (const DescriptionIndex& building_index : get()->consumers()) {
		if (tribe.has_building(building_index)) {
			lua_pushint32(L, index++);
			upcasted_map_object_descr_to_lua(
			   L, get_egbase(L).tribes().get_building_descr(building_index));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
   .. method:: is_construction_material(tribename)

      :arg tribename: the name of the tribe that this ware gets checked for
      :type tribename: :class:`string`

      (RO) A bool that is true if this ware is used by the tribe's construction sites.
*/
int LuaWareDescription::is_construction_material(lua_State* L) {
	std::string tribename = luaL_checkstring(L, -1);
	const Tribes& tribes = get_egbase(L).tribes();
	if (tribes.tribe_exists(tribename)) {
		const DescriptionIndex& ware_index = tribes.safe_ware_index(get()->name());
		int tribeindex = tribes.tribe_index(tribename);
		lua_pushboolean(L, tribes.get_tribe_descr(tribeindex)->is_construction_material(ware_index));
	} else {
		lua_pushboolean(L, false);
	}
	return 1;
}

/* RST
   .. method:: producers(tribename)

      Returns a list of buildings for the 'tribe' that produce this ware.
      Loads the tribe if it hasn't been loaded yet.

      :arg tribename: the name of the tribe that this ware gets checked for
      :type tribename: :class:`string`

      (RO) An array with :class:`LuaBuildingDescription` with buildings that
      can procude this ware.
*/
int LuaWareDescription::producers(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Takes only one argument.");
	}
	const Widelands::TribeDescr& tribe = get_tribe_descr(L, luaL_checkstring(L, 2));

	lua_newtable(L);
	int index = 1;
	for (const DescriptionIndex& building_index : get()->producers()) {
		if (tribe.has_building(building_index)) {
			lua_pushint32(L, index++);
			upcasted_map_object_descr_to_lua(
			   L, get_egbase(L).tribes().get_building_descr(building_index));
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
WorkerDescription
-----------------

.. class:: WorkerDescription

   Child of: :class:`MapObjectDescription`

   A static description of a tribe's worker, so it can be used in help files
   without having to access an actual instance of the worker on the map.
   See also :class:`MapObjectDescription` for more properties.
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
	const WorkerDescr* descr = get();
	PERS_STRING("name", descr->name());
}

void LuaWorkerDescription::__unpersist(lua_State* L) {
	std::string name;
	UNPERS_STRING("name", name)
	const Tribes& tribes = get_egbase(L).tribes();
	DescriptionIndex idx = tribes.safe_worker_index(name.c_str());
	set_description_pointer(tribes.get_worker_descr(idx));
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
	const DescriptionIndex becomes_index = get()->becomes();
	if (becomes_index == INVALID_INDEX) {
		lua_pushnil(L);
		return 1;
	}
	return to_lua<LuaWorkerDescription>(
	   L, new LuaWorkerDescription(get_egbase(L).tribes().get_worker_descr(becomes_index)));
}

/* RST
   .. attribute:: buildcost

      (RO) a list of building requirements, e.g. {"atlanteans_carrier", "ax"}
*/
int LuaWorkerDescription::get_buildcost(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	if (get()->is_buildable()) {
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

      (RO) An array with :class:`LuaBuildingDescription` with buildings where
      this worker can be employed.
*/
int LuaWorkerDescription::get_employers(lua_State* L) {
	lua_newtable(L);
	int index = 1;
	for (const DescriptionIndex& building_index : get()->employers()) {
		lua_pushint32(L, index++);
		upcasted_map_object_descr_to_lua(
		   L, get_egbase(L).tribes().get_building_descr(building_index));
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
   .. attribute:: buildable

      (RO) returns true if this worker is buildable
*/
int LuaWorkerDescription::get_buildable(lua_State* L) {
	lua_pushboolean(L, get()->is_buildable());
	return 1;
}

/* RST
   .. attribute:: needed_experience

         (RO) the experience the worker needs to reach this level.
*/
int LuaWorkerDescription::get_needed_experience(lua_State* L) {
	lua_pushinteger(L, get()->get_needed_experience());
	return 1;
}

/* RST
SoldierDescription
--------------------

.. class:: SoldierDescription

   Child of: :class:`MapObjectDescription`, :class:`WorkerDescription`

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
   .. attribute:: get_max_health_level

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
   .. attribute:: get_base_health

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
   .. attribute:: get_health_incr_per_level

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
	const World& world = get_egbase(L).world();
	const ResourceDescription* descr = world.get_resource(world.safe_resource_index(name));
	set_description_pointer(descr);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: name

         (RO) the :class:`string` internal name of this resource
*/

int LuaResourceDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: descname

         (RO) the :class:`string` display name of this resource
*/

int LuaResourceDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: is_detectable

         (RO) true if geologists can find this resource
*/

int LuaResourceDescription::get_is_detectable(lua_State* L) {
	lua_pushboolean(L, get()->detectable());
	return 1;
}

/* RST
   .. attribute:: max_amount

         (RO) the maximum amount of this resource that a terrain can have
*/

int LuaResourceDescription::get_max_amount(lua_State* L) {
	lua_pushinteger(L, get()->max_amount());
	return 1;
}

/* RST
   .. attribute:: representative_image

         (RO) the :class:`string` path to the image representing this resource in the GUI
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

      :arg amount: The amount of the resource what we want an overlay image for

         (RO) the :class:`string` path to the image representing the specified amount of this
         resource
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
	set_description_pointer(get_egbase(L).world().terrain_descr(name));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

/* RST
   .. attribute:: name

         (RO) the :class:`string` internal name of this terrain
*/

int LuaTerrainDescription::get_name(lua_State* L) {
	lua_pushstring(L, get()->name());
	return 1;
}

/* RST
   .. attribute:: descname

         (RO) the :class:`string` display name of this terrain
*/

int LuaTerrainDescription::get_descname(lua_State* L) {
	lua_pushstring(L, get()->descname());
	return 1;
}

/* RST
   .. attribute:: get_default_resource

         (RO) the :class:`wl.map.ResourceDescription` for the default resource provided by this
         terrain, or nil if the terrain has no default resource.
*/

int LuaTerrainDescription::get_default_resource(lua_State* L) {
	DescriptionIndex res_index = get()->get_default_resource();
	const World& world = get_egbase(L).world();
	if (res_index != Widelands::kNoResource && res_index < world.get_nr_resources()) {
		to_lua<LuaMaps::LuaResourceDescription>(
		   L, new LuaMaps::LuaResourceDescription(world.get_resource(res_index)));
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/* RST
   .. attribute:: default_resource_amount

      (RO) the int amount of the default resource provided by this terrain.
*/

int LuaTerrainDescription::get_default_resource_amount(lua_State* L) {
	lua_pushinteger(L, get()->get_default_resource_amount());
	return 1;
}

/* RST
   .. attribute:: fertility

         (RO) the :class:`uint` fertility value for this terrain
*/

int LuaTerrainDescription::get_fertility(lua_State* L) {
	lua_pushuint32(L, get()->fertility());
	return 1;
}

/* RST
   .. attribute:: humidity

         (RO) the :class:`uint` humidity value for this terrain
*/

int LuaTerrainDescription::get_humidity(lua_State* L) {
	lua_pushuint32(L, get()->humidity());
	return 1;
}

/* RST
   .. attribute:: representative_image

         (RO) the :class:`string` file path to a representative image
*/
int LuaTerrainDescription::get_representative_image(lua_State* L) {
	lua_pushstring(L, get()->texture_paths().front());
	return 1;
}

/* RST
   .. attribute:: temperature

         (RO) the :class:`uint` temperature value for this terrain
*/

int LuaTerrainDescription::get_temperature(lua_State* L) {
	lua_pushuint32(L, get()->temperature());
	return 1;
}

/* RST
   .. attribute:: valid_resources

         (RO) a list of :class:`wl.map.ResourceDescription` with all valid resources for this
         terrain.
*/

int LuaTerrainDescription::get_valid_resources(lua_State* L) {
	const World& world = get_egbase(L).world();
	lua_newtable(L);
	int index = 1;
	for (DescriptionIndex res_index : get()->valid_resources()) {
		if (res_index != Widelands::kNoResource && res_index < world.get_nr_resources()) {
			lua_pushint32(L, index++);
			to_lua<LuaMaps::LuaResourceDescription>(
			   L, new LuaMaps::LuaResourceDescription(world.get_resource(res_index)));
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
.. class:: LuaEconomy

   Provides access to an economy. A player can have multiple economies;
   you can get an economy from a :class:`Flag`.
*/
const char LuaEconomy::className[] = "Economy";
const MethodType<LuaEconomy> LuaEconomy::Methods[] = {
   METHOD(LuaEconomy, target_quantity),
   METHOD(LuaEconomy, set_target_quantity),
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

      :arg name: the name of the ware or worker.
      :type name: :class:`string`
*/
int LuaEconomy::target_quantity(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).tribes().ware_index(wname);
		if (get_egbase(L).tribes().ware_exists(index)) {
			const Widelands::Economy::TargetQuantity& quantity = get()->target_quantity(index);
			lua_pushinteger(L, quantity.permanent);
		} else {
			report_error(L, "There is no ware '%s'.", wname.c_str());
		}
		break;
	}
	case Widelands::wwWORKER: {
		const Widelands::DescriptionIndex index = get_egbase(L).tribes().worker_index(wname);
		if (get_egbase(L).tribes().worker_exists(index)) {
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
   .. method:: set_target_quantity(name)

      Sets the amount of the given ware or worker type that should be kept in stock for this
      economy. Whether this works only for wares or only for workers is determined by the type of
      this economy.

      **Warning**: Since economies can disappear when a player merges them
      through placing/deleting roads and flags, you must get a fresh economy
      object every time you use this function.

      :arg workername: the name of the worker type.
      :type workername: :class:`string`

      :arg amount: the new target amount for the worker. Needs to be >= 0.
      :type amount: :class:`integer`
*/
int LuaEconomy::set_target_quantity(lua_State* L) {
	const std::string wname = luaL_checkstring(L, 2);
	switch (get()->type()) {
	case Widelands::wwWARE: {
		const Widelands::DescriptionIndex index = get_egbase(L).tribes().ware_index(wname);
		if (get_egbase(L).tribes().ware_exists(index)) {
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
		const Widelands::DescriptionIndex index = get_egbase(L).tribes().worker_index(wname);
		if (get_egbase(L).tribes().worker_exists(index)) {
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

   More properties are available through this object's
   :class:`MapObjectDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaMapObject::className[] = "MapObject";
const MethodType<LuaMapObject> LuaMapObject::Methods[] = {
   METHOD(LuaMapObject, remove), METHOD(LuaMapObject, destroy),
   METHOD(LuaMapObject, __eq),   METHOD(LuaMapObject, has_attribute),
   {nullptr, nullptr},
};
const PropertyType<LuaMapObject> LuaMapObject::Properties[] = {
   PROP_RO(LuaMapObject, __hash),
   PROP_RO(LuaMapObject, descr),
   PROP_RO(LuaMapObject, serial),
   {nullptr, nullptr, nullptr},
};

void LuaMapObject::__persist(lua_State* L) {
	MapObjectSaver& mos = *get_mos(L);
	Game& game = get_game(L);

	uint32_t idx = 0;
	if (MapObject* obj = ptr_.get(game)) {
		idx = mos.get_object_file_index(*obj);
	}

	PERS_UINT32("file_index", idx);
}
void LuaMapObject::__unpersist(lua_State* L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx)

	if (!idx) {
		ptr_ = nullptr;
	} else {
		MapObjectLoader& mol = *get_mol(L);
		ptr_ = &mol.get<MapObject>(idx);
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

// use the dynamic type of BuildingDescription
#define CAST_TO_LUA(klass, lua_klass)                                                              \
	to_lua<lua_klass>(L, new lua_klass(dynamic_cast<const klass*>(desc)))

/* RST
   .. attribute:: descr

      (RO) The description object for this immovable, e.g. BuildingDescription.
*/
int LuaMapObject::get_descr(lua_State* L) {
	const MapObjectDescr* desc = &get(L, get_egbase(L))->descr();
	assert(desc != nullptr);

	switch (desc->type()) {
	case MapObjectType::BUILDING:
		return CAST_TO_LUA(BuildingDescr, LuaBuildingDescription);
	case MapObjectType::CONSTRUCTIONSITE:
		return CAST_TO_LUA(ConstructionSiteDescr, LuaConstructionSiteDescription);
	case MapObjectType::DISMANTLESITE:
		return CAST_TO_LUA(DismantleSiteDescr, LuaDismantleSiteDescription);
	case MapObjectType::PRODUCTIONSITE:
		return CAST_TO_LUA(ProductionSiteDescr, LuaProductionSiteDescription);
	case MapObjectType::MILITARYSITE:
		return CAST_TO_LUA(MilitarySiteDescr, LuaMilitarySiteDescription);
	case MapObjectType::TRAININGSITE:
		return CAST_TO_LUA(TrainingSiteDescr, LuaTrainingSiteDescription);
	case MapObjectType::WAREHOUSE:
		return CAST_TO_LUA(WarehouseDescr, LuaWarehouseDescription);
	case MapObjectType::MARKET:
		return CAST_TO_LUA(MarketDescr, LuaMarketDescription);
	case MapObjectType::IMMOVABLE:
		return CAST_TO_LUA(ImmovableDescr, LuaImmovableDescription);
	case MapObjectType::WORKER:
	case MapObjectType::CARRIER:
	case MapObjectType::FERRY:
	case MapObjectType::SOLDIER:
		return CAST_TO_LUA(WorkerDescr, LuaWorkerDescription);
	case MapObjectType::SHIP:
		return CAST_TO_LUA(ShipDescr, LuaShipDescription);
	case MapObjectType::RESOURCE:
		return CAST_TO_LUA(ResourceDescription, LuaResourceDescription);
	case MapObjectType::TERRAIN:
		return CAST_TO_LUA(TerrainDescription, LuaTerrainDescription);
	case MapObjectType::MAPOBJECT:
	case MapObjectType::BATTLE:
	case MapObjectType::BOB:
	case MapObjectType::CRITTER:
	case MapObjectType::FERRY_FLEET:
	case MapObjectType::SHIP_FLEET:
	case MapObjectType::FLAG:
	case MapObjectType::ROAD:
	case MapObjectType::WATERWAY:
	case MapObjectType::ROADBASE:
	case MapObjectType::PORTDOCK:
	case MapObjectType::WARE:
		return CAST_TO_LUA(MapObjectDescr, LuaMapObjectDescription);
	}
	NEVER_HERE();
}

#undef CAST_TO_LUA

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaMapObject::__eq(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	LuaMapObject* other = *get_base_user_class<LuaMapObject>(L, -1);

	MapObject* me = get_or_zero(egbase);
	MapObject* you = other->get_or_zero(egbase);

	// Both objects are destroyed (nullptr) or equal: they are equal
	if (me == you) {
		lua_pushboolean(L, true);
	} else if (me == nullptr ||
	           you == nullptr) {  // One of the objects is destroyed: they are distinct
		lua_pushboolean(L, false);
	} else {  // Compare their serial number.
		lua_pushboolean(L, other->get(L, egbase)->serial() == get(L, egbase)->serial());
	}

	return 1;
}

/* RST
   .. method:: remove()

      Removes this object immediately. If you want to destroy an
      object as if the player had see :func:`destroy`.
*/
int LuaMapObject::remove(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	MapObject* o = get(L, egbase);
	if (!o) {
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
	EditorGameBase& egbase = get_egbase(L);
	MapObject* o = get(L, egbase);
	if (!o) {
		return 0;
	}

	o->destroy(egbase);
	return 0;
}

/* RST
   .. method:: has_attribute(string)

      returns true, if the map object has this attribute, else false
*/
int LuaMapObject::has_attribute(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	MapObject* obj = get_or_zero(egbase);
	if (!obj) {
		lua_pushboolean(L, false);
		return 1;
	}

	// Check if object has the attribute
	std::string attrib = luaL_checkstring(L, 2);
	if (obj->has_attribute(MapObjectDescr::get_attribute_id(attrib))) {
		lua_pushboolean(L, true);
	} else {
		lua_pushboolean(L, false);
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
	if (!o) {
		report_error(L, "%s no longer exists!", name.c_str());
	}
	return o;
}
MapObject* LuaMapObject::get_or_zero(EditorGameBase& egbase) {
	return ptr_.get(egbase);
}

/* RST
BaseImmovable
-------------

.. class:: BaseImmovable

   Child of: :class:`MapObject`

   This is the base class for all immovables in Widelands.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
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

      (RO) An :class:`array` of :class:`~wl.map.Field` that is occupied by this
      Immovable. If the immovable occupies more than one field (roads or big
      buildings for example) the first entry in this list will be the main field
*/
int LuaBaseImmovable::get_fields(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);

	BaseImmovable::PositionList pl = get(L, egbase)->get_positions(egbase);

	lua_createtable(L, pl.size(), 0);
	uint32_t idx = 1;
	for (const Coords& coords : pl) {
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

   Child of: :class:`BaseImmovable`

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
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(wwWARE));
	return 1;
}
int LuaPlayerImmovable::get_debug_worker_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(wwWORKER));
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

   Child of: :class:`PlayerImmovable`, :class:`HasWares`

   One flag in the economy of this Player.

   More properties are available through this object's
   :class:`ImmovableDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaFlag::className[] = "Flag";
const MethodType<LuaFlag> LuaFlag::Methods[] = {
   METHOD(LuaFlag, set_wares),
   METHOD(LuaFlag, get_wares),
   {nullptr, nullptr},
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
	const Flag* f = get(L, get_egbase(L));
	return to_lua<LuaEconomy>(L, new LuaEconomy(f->get_economy(wwWARE)));
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
	const Flag* f = get(L, get_egbase(L));
	return to_lua<LuaEconomy>(L, new LuaEconomy(f->get_economy(wwWORKER)));
}

/* RST
   .. attribute:: roads

      (RO) Array of roads leading to the flag. Directions
      can be tr,r,br,bl,l and tl
      Note that waterways are currently treated like roads.

      :returns: The array of 'direction:road', if any
*/
int LuaFlag::get_roads(lua_State* L) {

	const std::vector<std::string> directions = {"tr", "r", "br", "bl", "l", "tl"};

	lua_newtable(L);

	EditorGameBase& egbase = get_egbase(L);
	Flag* f = get(L, egbase);

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

      (RO) building belonging to the flag
*/
int LuaFlag::get_building(lua_State* L) {

	EditorGameBase& egbase = get_egbase(L);
	Flag* f = get(L, egbase);

	PlayerImmovable* building = f->get_building();
	if (!building) {
		return 0;
	} else {
		upcasted_map_object_to_lua(L, building);
	}
	return 1;
}
/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
// Documented in ParentClass
int LuaFlag::set_wares(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	Flag* f = get(L, egbase);
	const Tribes& tribes = egbase.tribes();

	InputMap setpoints;
	parse_wares_workers_counted(L, f->owner().tribe(), &setpoints, true);
	WaresWorkersMap c_wares = count_wares_on_flag_(*f, tribes);

	for (const auto& ware : c_wares) {
		// all wares currently on the flag without a setpoint should be removed
		if (!setpoints.count(std::make_pair(ware.first, Widelands::WareWorker::wwWARE))) {
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
				for (const WareInstance* ware : f->get_wares()) {
					if (tribes.ware_index(ware->descr().name()) == index) {
						const_cast<WareInstance*>(ware)->remove(egbase);
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

		const WareDescr& wd = *tribes.get_ware_descr(ware_to_add.first);
		for (int i = 0; i < ware_to_add.second; i++) {
			WareInstance& ware = *new WareInstance(ware_to_add.first, &wd);
			ware.init(egbase);
			f->add_ware(egbase, ware);
		}
	}

#ifndef NDEBUG
	WaresWorkersMap wares_on_flag = count_wares_on_flag_(*f, tribes);
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
	EditorGameBase& egbase = get_egbase(L);
	const Tribes& tribes = egbase.tribes();
	Flag* flag = get(L, egbase);
	const TribeDescr& tribe = flag->owner().tribe();

	DescriptionIndex ware_index = INVALID_INDEX;
	std::vector<DescriptionIndex> ware_list;
	RequestedWareWorker parse_output =
	   parse_wares_workers_list(L, tribe, &ware_index, &ware_list, true);

	WaresWorkersMap wares = count_wares_on_flag_(*flag, tribes);

	// Here we create the output - either a single integer of table of pairs
	if (ware_index != INVALID_INDEX) {
		uint32_t wares_here = 0;
		if (wares.count(ware_index)) {
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
			if (wares.count(idx)) {
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

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

/* RST
Road
----

.. class:: Road

   Child of: :class:`PlayerImmovable`, :class:`HasWorkers`

   A road connecting two flags in the economy of this Player.
   Waterways are currently treated like roads in scripts; however,
   there are significant differences. You can check whether an
   instance of Road is a road or waterway using `get_road_type()`.

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

      (RO) The flag were this road starts
*/
int LuaRoad::get_start_flag(lua_State* L) {
	return to_lua<LuaFlag>(L, new LuaFlag(get(L, get_egbase(L))->get_flag(RoadBase::FlagStart)));
}

/* RST
   .. attribute:: end_flag

      (RO) The flag were this road ends
*/
int LuaRoad::get_end_flag(lua_State* L) {
	return to_lua<LuaFlag>(L, new LuaFlag(get(L, get_egbase(L))->get_flag(RoadBase::FlagEnd)));
}

/* RST
   .. attribute:: road_type

      (RO) Type of road. Can be any either of:

      * normal
      * busy
      * waterway
*/
int LuaRoad::get_road_type(lua_State* L) {
	RoadBase* r = get(L, get_egbase(L));
	if (r->descr().type() == MapObjectType::WATERWAY) {
		lua_pushstring(L, "waterway");
	} else if (upcast(Road, road, r)) {
		lua_pushstring(L, road->is_busy() ? "busy" : "normal");
	} else {
		report_error(L, "Unknown road type! Please report as a bug!");
	}
	return 1;
}

// documented in parent class
int LuaRoad::get_valid_workers(lua_State* L) {
	RoadBase* road = get(L, get_egbase(L));
	return workers_map_to_lua(L, get_valid_workers_for(*road));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaRoad::get_workers(lua_State* L) {
	RoadBase* road = get(L, get_egbase(L));
	return do_get_workers(L, *road, get_valid_workers_for(*road));
}

int LuaRoad::set_workers(lua_State* L) {
	RoadBase* road = get(L, get_egbase(L));
	return do_set_workers<LuaRoad>(L, road, get_valid_workers_for(*road));
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

bool LuaRoad::create_new_worker(lua_State* L,
                                RoadBase& rb,
                                EditorGameBase& egbase,
                                const WorkerDescr* wdes) {
	Road* r = dynamic_cast<Road*>(&rb);
	const bool is_busy = r && r->is_busy();
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
	Flag& start = rb.get_flag(RoadBase::FlagStart);
	Coords idle_position = start.get_position();
	const Path& path = rb.get_path();
	Path::StepVector::size_type idle_index = rb.get_idle_index();
	for (Path::StepVector::size_type i = 0; i < idle_index; ++i) {
		egbase.map().get_neighbour(idle_position, path[i], &idle_position);
	}

	// Ensure the position is free - e.g. we want carrier + carrier2 for busy road, not 2x carrier!
	for (Worker* existing : rb.get_workers()) {
		if (existing->descr().name() == wdes->name()) {
			report_error(L, "Road already has worker <%s> assigned at (%d, %d)", wdes->name().c_str(),
			             idle_position.x, idle_position.y);
		}
	}

	Carrier& carrier =
	   dynamic_cast<Carrier&>(wdes->create(egbase, rb.get_owner(), &rb, idle_position));

	if (upcast(Game, game, &egbase)) {
		carrier.start_task_road(*game);
	}

	rb.assign_carrier(carrier, 0);
	return true;
}

/* RST
PortDock
--------

.. class:: PortDock

   Child of: :class:`PlayerImmovable`

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

   Child of: :class:`PlayerImmovable`

   This represents a building owned by a player.

   More properties are available through this object's
   :class:`BuildingDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaBuilding::className[] = "Building";
const MethodType<LuaBuilding> LuaBuilding::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaBuilding> LuaBuilding::Properties[] = {
   PROP_RO(LuaBuilding, flag),
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
ConstructionSite
-----------------

.. class:: ConstructionSite

   Child of: :class:`Building`

   A ConstructionSite as it appears in Game. This is only a minimal wrapping at
   the moment.

   More properties are available through this object's
   :class:`ConstructionSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaConstructionSite::className[] = "ConstructionSite";
const MethodType<LuaConstructionSite> LuaConstructionSite::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaConstructionSite> LuaConstructionSite::Properties[] = {
   PROP_RO(LuaConstructionSite, building),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
   .. attribute:: building

      (RO) The name of the building that is constructed here
*/
int LuaConstructionSite::get_building(lua_State* L) {
	lua_pushstring(L, get(L, get_egbase(L))->building().name());
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
Warehouse
---------

.. class:: Warehouse

   Child of: :class:`Building`, :class:`HasWares`, :class:`HasWorkers`,
   :class:`HasSoldiers`

   Every Headquarter, Port or Warehouse on the Map is of this type.

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
      :class:`PortDock` attached to it, otherwise nil.
*/
int LuaWarehouse::get_portdock(lua_State* L) {
	return upcasted_map_object_to_lua(L, get(L, get_egbase(L))->get_portdock());
}

/* RST
   .. attribute:: expedition_in_progress

      (RO) If this Warehouse is a port, and an expedition is in
      progress, returns true, otherwise nil
*/
int LuaWarehouse::get_expedition_in_progress(lua_State* L) {

	EditorGameBase& egbase = get_egbase(L);

	if (is_a(Game, &egbase)) {
		const PortDock* pd = get(L, egbase)->get_portdock();
		if (pd) {
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
	Warehouse* wh = get(L, get_egbase(L));
	const TribeDescr& tribe = wh->owner().tribe();

	// Parsing the argument(s), result will be either single index
	// or list of indexes
	DescriptionIndex worker_index = INVALID_INDEX;
	std::vector<DescriptionIndex> workers_list;
	parse_wares_workers_list(L, tribe, &worker_index, &workers_list, false);

	// Here we create the output - either a single integer of table of pairs
	if (worker_index != INVALID_INDEX) {
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
	Warehouse* wh = get(L, get_egbase(L));
	const TribeDescr& tribe = wh->owner().tribe();

	// Parsing the argument(s), result will be either single index
	// or list of indexes
	DescriptionIndex ware_index = INVALID_INDEX;
	std::vector<DescriptionIndex> ware_list;
	parse_wares_workers_list(L, tribe, &ware_index, &ware_list, true);

	// Here we create the output - either a single integer of table of pairs
	if (ware_index != INVALID_INDEX) {
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
	Warehouse* wh = get(L, get_egbase(L));
	const TribeDescr& tribe = wh->owner().tribe();
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
	Warehouse* wh = get(L, get_egbase(L));
	const TribeDescr& tribe = wh->owner().tribe();
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

// Transforms the given warehouse policy to a string which is used by the lua code
inline void wh_policy_to_string(lua_State* L, StockPolicy p) {
	switch (p) {
	case StockPolicy::kNormal:
		lua_pushstring(L, "normal");
		break;
	case StockPolicy::kPrefer:
		lua_pushstring(L, "prefer");
		break;
	case StockPolicy::kDontStock:
		lua_pushstring(L, "dontstock");
		break;
	case StockPolicy::kRemove:
		lua_pushstring(L, "remove");
		break;
	}
}
// Transforms the given string from the lua code to a warehouse policy
inline StockPolicy string_to_wh_policy(lua_State* L, uint32_t index) {
	std::string str = luaL_checkstring(L, index);
	if (str == "normal") {
		return StockPolicy::kNormal;
	} else if (str == "prefer") {
		return StockPolicy::kPrefer;
	} else if (str == "dontstock") {
		return StockPolicy::kDontStock;
	} else if (str == "remove") {
		return StockPolicy::kRemove;
	} else {
		report_error(L, "<%s> is no valid warehouse policy!", str.c_str());
	}
}

inline bool do_set_ware_policy(Warehouse* wh, const DescriptionIndex idx, const StockPolicy p) {
	wh->set_ware_policy(idx, p);
	return true;
}

/**
 * Sets the given policy for the given ware in the given warehouse and return true.
 * If the no ware with the given name exists for the tribe of the warehouse, return false.
 */
inline bool do_set_ware_policy(Warehouse* wh, const std::string& name, const StockPolicy p) {
	const TribeDescr& tribe = wh->owner().tribe();
	DescriptionIndex idx = tribe.ware_index(name);
	if (!tribe.has_ware(idx)) {
		return false;
	}
	return do_set_ware_policy(wh, idx, p);
}

inline bool do_set_worker_policy(Warehouse* wh, const DescriptionIndex idx, const StockPolicy p) {
	const TribeDescr& tribe = wh->owner().tribe();
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
inline bool do_set_worker_policy(Warehouse* wh, const std::string& name, const StockPolicy p) {
	const TribeDescr& tribe = wh->owner().tribe();
	DescriptionIndex idx = tribe.worker_index(name);
	if (!tribe.has_worker(idx)) {
		return false;
	}
	return do_set_worker_policy(wh, idx, p);
}
/* RST
   .. method:: set_warehouse_policies(which, policy)

      Sets the policies how the warehouse should handle the given wares and workers.

      Usage example:

      .. code-block:: lua

         wh:set_warehouse_policies("coal", "prefer")

      :arg which: behaves like for :meth:`HasWares.get_wares`.

      :arg policy: the policy to apply for all the wares and workers given in `which`.
      :type policy: a string out of "normal", "prefer", "dontstock", "remove".
*/
int LuaWarehouse::set_warehouse_policies(lua_State* L) {
	int32_t nargs = lua_gettop(L);
	if (nargs != 3) {
		report_error(L, "Wrong number of arguments to set_warehouse_policies!");
	}

	Warehouse* wh = get(L, get_egbase(L));
	StockPolicy p = string_to_wh_policy(L, -1);
	lua_pop(L, 1);
	const TribeDescr& tribe = wh->owner().tribe();

	// takes either "all", a name or an array of names
	if (lua_isstring(L, 2)) {
		const std::string& what = luaL_checkstring(L, -1);
		if (what == "all") {
			for (const DescriptionIndex& i : tribe.wares()) {
				do_set_ware_policy(wh, i, p);
			}
			for (const DescriptionIndex& i : tribe.workers()) {
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
	inline void do_get_##type##_policy(lua_State* L, Warehouse* wh, const DescriptionIndex idx) {   \
		wh_policy_to_string(L, wh->get_##type##_policy(idx));                                        \
	}                                                                                               \
                                                                                                   \
	inline bool do_get_##type##_policy(lua_State* L, Warehouse* wh, const std::string& name) {      \
		const TribeDescr& tribe = wh->owner().tribe();                                               \
		DescriptionIndex idx = tribe.type##_index(name);                                             \
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

      Gets the policies how the warehouse should handle the given wares and workers.
      The method to handle is one of the strings "normal", "prefer", "dontstock", "remove".

      Usage example:

      .. code-block:: lua

         wh:get_warehouse_policies({"ax", "coal"})
         -- Returns a table like {ax="normal", coal="prefer"}

      :arg which: behaves like for :meth:`HasWares.get_wares`.

      :returns: :class:`string` or :class:`table`
*/
int LuaWarehouse::get_warehouse_policies(lua_State* L) {
	int32_t nargs = lua_gettop(L);
	if (nargs != 2) {
		report_error(L, "Wrong number of arguments to get_warehouse_policies!");
	}
	Warehouse* wh = get(L, get_egbase(L));
	const TribeDescr& tribe = wh->owner().tribe();
	// takes either "all", a single name or an array of names
	if (lua_isstring(L, 2)) {
		std::string what = luaL_checkstring(L, -1);
		if (what == "all") {
			lua_newtable(L);
			for (const DescriptionIndex& i : tribe.wares()) {
				std::string name = tribe.get_ware_descr(i)->name();
				lua_pushstring(L, name.c_str());
				do_get_ware_policy(L, wh, i);
				lua_rawset(L, -3);
			}
			for (const DescriptionIndex& i : tribe.workers()) {
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
	Warehouse* wh = get(L, get_egbase(L));
	return do_get_soldiers(L, *wh->soldier_control(), wh->owner().tribe());
}

// documented in parent class
int LuaWarehouse::set_soldiers(lua_State* L) {
	Warehouse* wh = get(L, get_egbase(L));
	return do_set_soldiers(L, wh->get_position(), wh->mutable_soldier_control(), wh->get_owner());
}

/* RST
   .. method:: start_expedition()

      Starts preparation for expedition

*/
int LuaWarehouse::start_expedition(lua_State* L) {

	EditorGameBase& egbase = get_egbase(L);
	Warehouse* wh = get(L, egbase);

	if (!wh) {
		return 0;
	}

	if (upcast(Game, game, &egbase)) {
		const PortDock* pd = wh->get_portdock();
		if (!pd) {
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

      Cancels an expedition if in progress

*/
int LuaWarehouse::cancel_expedition(lua_State* L) {

	EditorGameBase& egbase = get_egbase(L);
	Warehouse* wh = get(L, egbase);

	if (!wh) {
		return 0;
	}

	if (upcast(Game, game, &egbase)) {
		const PortDock* pd = wh->get_portdock();
		if (!pd) {
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

   Child of: :class:`Building`, :class:`HasInputs`, :class:`HasWorkers`

   Every building that produces anything.

   More properties are available through this object's
   :class:`ProductionSiteDescription`, which you can access via :any:`MapObject.descr`.
*/
const char LuaProductionSite::className[] = "ProductionSite";
const MethodType<LuaProductionSite> LuaProductionSite::Methods[] = {
   METHOD(LuaProductionSite, set_inputs),
   METHOD(LuaProductionSite, get_inputs),
   METHOD(LuaProductionSite, get_workers),
   METHOD(LuaProductionSite, set_workers),
   METHOD(LuaProductionSite, toggle_start_stop),

   {nullptr, nullptr},
};
const PropertyType<LuaProductionSite> LuaProductionSite::Properties[] = {
   PROP_RO(LuaProductionSite, valid_workers),
   PROP_RO(LuaProductionSite, valid_inputs),
   PROP_RO(LuaProductionSite, is_stopped),
   {nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// documented in parent class
int LuaProductionSite::get_valid_inputs(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	ProductionSite* ps = get(L, egbase);

	lua_newtable(L);
	for (const auto& input_ware : ps->descr().input_wares()) {
		const WareDescr* descr = egbase.tribes().get_ware_descr(input_ware.first);
		lua_pushstring(L, descr->name());
		lua_pushuint32(L, input_ware.second);
		lua_rawset(L, -3);
	}
	for (const auto& input_worker : ps->descr().input_workers()) {
		const WorkerDescr* descr = egbase.tribes().get_worker_descr(input_worker.first);
		lua_pushstring(L, descr->name());
		lua_pushuint32(L, input_worker.second);
		lua_rawset(L, -3);
	}
	return 1;
}

// documented in parent class
int LuaProductionSite::get_valid_workers(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	return workers_map_to_lua(L, get_valid_workers_for(*ps));
}

/* RST
   .. attribute:: is_stopped

      (RO) Returns whether this productionsite is currently active or stopped

      :returns: true if the productionsite has been started,
         false if it has been stopped.
*/
int LuaProductionSite::get_is_stopped(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	lua_pushboolean(L, ps->is_stopped());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
// documented in parent class
int LuaProductionSite::set_inputs(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	const TribeDescr& tribe = ps->owner().tribe();
	InputMap setpoints = parse_set_input_arguments(L, tribe);

	InputSet valid_inputs;
	for (const auto& input_ware : ps->descr().input_wares()) {
		valid_inputs.insert(std::make_pair(input_ware.first, wwWARE));
	}
	for (const auto& input_worker : ps->descr().input_workers()) {
		valid_inputs.insert(std::make_pair(input_worker.first, wwWORKER));
	}
	for (const auto& sp : setpoints) {
		if (!valid_inputs.count(sp.first)) {
			report_error(L, "<%s> can't be stored in this building: %s!",
			             sp.first.second == wwWARE ?
			                tribe.get_ware_descr(sp.first.first)->name().c_str() :
			                tribe.get_worker_descr(sp.first.first)->name().c_str(),
			             ps->descr().name().c_str());
		}
		InputQueue& iq = ps->inputqueue(sp.first.first, sp.first.second);
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
	ProductionSite* ps = get(L, get_egbase(L));
	const TribeDescr& tribe = ps->owner().tribe();

	bool return_number = false;
	InputSet input_set = parse_get_input_arguments(L, tribe, &return_number);

	InputSet valid_inputs;
	for (const auto& input_ware : ps->descr().input_wares()) {
		valid_inputs.insert(std::make_pair(input_ware.first, wwWARE));
	}
	for (const auto& input_worker : ps->descr().input_workers()) {
		valid_inputs.insert(std::make_pair(input_worker.first, wwWORKER));
	}

	if (input_set.size() == tribe.get_nrwares() + tribe.get_nrworkers()) {  // Want all returned
		input_set = valid_inputs;
	}

	if (!return_number) {
		lua_newtable(L);
	}

	for (const auto& input : input_set) {
		uint32_t cnt = 0;
		if (valid_inputs.count(input)) {
			cnt = ps->inputqueue(input.first, input.second).get_filled();
		}

		if (return_number) {  // this is the only thing the customer wants to know
			lua_pushuint32(L, cnt);
			break;
		} else {
			lua_pushstring(L, input.second == wwWARE ? tribe.get_ware_descr(input.first)->name() :
			                                           tribe.get_worker_descr(input.first)->name());
			lua_pushuint32(L, cnt);
			lua_settable(L, -3);
		}
	}
	return 1;
}

// documented in parent class
int LuaProductionSite::get_workers(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	return do_get_workers(L, *ps, get_valid_workers_for(*ps));
}

// documented in parent class
int LuaProductionSite::set_workers(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	return do_set_workers<LuaProductionSite>(L, ps, get_valid_workers_for(*ps));
}

/* RST
   .. method:: toggle_start_stop()

      If :any:`ProductionSite.is_stopped`, sends a command to start this productionsite.
      Otherwise, sends a command to stop this productionsite.
*/
int LuaProductionSite::toggle_start_stop(lua_State* L) {
	Game& game = get_game(L);
	ProductionSite* ps = get(L, game);
	game.send_player_start_stop_building(*ps);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

bool LuaProductionSite::create_new_worker(lua_State* /* L */,
                                          ProductionSite& ps,
                                          EditorGameBase& egbase,
                                          const WorkerDescr* wdes) {
	return ps.warp_worker(egbase, *wdes);
}

/* RST
Market
---------

.. class:: Market

   Child of: :class:`Building`, :class:`HasWares`, :class:`HasWorkers`

   A Market used for trading with other players.

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

/* RST
   .. method:: propose_trade(other_market, num_batches, items_to_send, items_to_receive)

      TODO(sirver,trading): document

      :returns: :const:`nil`
*/
int LuaMarket::propose_trade(lua_State* L) {
	if (lua_gettop(L) != 5) {
		report_error(L, "Takes 4 arguments.");
	}
	Game& game = get_game(L);
	Market* self = get(L, game);
	Market* other_market = (*get_user_class<LuaMarket>(L, 2))->get(L, game);
	const int num_batches = luaL_checkinteger(L, 3);

	const BillOfMaterials items_to_send =
	   parse_wares_as_bill_of_material(L, 4, self->owner().tribe());
	// TODO(sirver,trading): unsure if correct. Test inter-tribe trading, i.e.
	// barbarians trading with empire, but shipping atlantean only wares.
	const BillOfMaterials items_to_receive =
	   parse_wares_as_bill_of_material(L, 5, self->owner().tribe());
	const int trade_id = game.propose_trade(
	   Trade{items_to_send, items_to_receive, num_batches, self->serial(), other_market->serial()});

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

   Child of: :class:`Building`, :class:`HasSoldiers`

   Miltary Buildings

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
	MilitarySite* ms = get(L, get_egbase(L));
	return do_get_soldiers(L, *ms->soldier_control(), ms->owner().tribe());
}

// documented in parent class
int LuaMilitarySite::set_soldiers(lua_State* L) {
	MilitarySite* ms = get(L, get_egbase(L));
	return do_set_soldiers(L, ms->get_position(), ms->mutable_soldier_control(), ms->get_owner());
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

   Child of: :class:`ProductionSite`, :class:`HasSoldiers`

   A specialized production site for training soldiers.

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

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int LuaTrainingSite::get_soldiers(lua_State* L) {
	TrainingSite* ts = get(L, get_egbase(L));
	return do_get_soldiers(L, *ts->soldier_control(), ts->owner().tribe());
}

// documented in parent class
int LuaTrainingSite::set_soldiers(lua_State* L) {
	TrainingSite* ts = get(L, get_egbase(L));
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

   Child of: :class:`MapObject`

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

      (RO) The field the bob is located on
*/
// UNTESTED
int LuaBob::get_field(lua_State* L) {

	EditorGameBase& egbase = get_egbase(L);

	Coords coords = get(L, egbase)->get_position();

	return to_lua<LuaMaps::LuaField>(L, new LuaMaps::LuaField(coords.x, coords.y));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
   .. method:: has_caps(capname)

      Similar to :meth:`Field::has_caps`.

      :arg capname: can be either of

      * :const:`swims`: This bob can swim.
      * :const:`walks`: This bob can walk.
*/
// UNTESTED
int LuaBob::has_caps(lua_State* L) {
	std::string query = luaL_checkstring(L, 2);

	uint32_t movecaps = get(L, get_egbase(L))->descr().movecaps();

	if (query == "swims") {
		lua_pushboolean(L, movecaps & MOVECAPS_SWIM);
	} else if (query == "walks") {
		lua_pushboolean(L, movecaps & MOVECAPS_WALK);
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
   {nullptr, nullptr},
};
const PropertyType<LuaShip> LuaShip::Properties[] = {
   PROP_RO(LuaShip, debug_ware_economy),
   PROP_RO(LuaShip, debug_worker_economy),
   PROP_RO(LuaShip, last_portdock),
   PROP_RO(LuaShip, destination),
   PROP_RO(LuaShip, state),
   PROP_RW(LuaShip, scouting_direction),
   PROP_RW(LuaShip, island_explore_direction),
   PROP_RO(LuaShip, shipname),
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
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(wwWARE));
	return 1;
}
int LuaShip::get_debug_worker_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy(wwWORKER));
	return 1;
}

/* RST
   .. attribute:: destination

      (RO) Either :const:`nil` if there is no current destination, otherwise
      the :class:`PortDock`.
*/
// UNTESTED
int LuaShip::get_destination(lua_State* L) {
	return upcasted_map_object_to_lua(L, get(L, get_egbase(L))->get_destination());
}

/* RST
   .. attribute:: last_portdock

      (RO) Either :const:`nil` if no port was ever visited or the last portdock
      was destroyed, otherwise the :class:`PortDock` of the last visited port.
*/
// UNTESTED
int LuaShip::get_last_portdock(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	return upcasted_map_object_to_lua(L, get(L, egbase)->get_lastdock(egbase));
}

/* RST
   .. attribute:: state

   Query which state the ship is in:

   - transport,
   - exp_waiting, exp_scouting, exp_found_port_space, exp_colonizing,
   - sink_request, sink_animation

      (RW) returns the :class:`string` ship's state, or :const:`nil` if there is no valid state.


*/
// UNTESTED sink states
int LuaShip::get_state(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	if (is_a(Game, &egbase)) {
		switch (get(L, egbase)->get_ship_state()) {
		case Ship::ShipStates::kTransport:
			lua_pushstring(L, "transport");
			break;
		case Ship::ShipStates::kExpeditionWaiting:
			lua_pushstring(L, "exp_waiting");
			break;
		case Ship::ShipStates::kExpeditionScouting:
			lua_pushstring(L, "exp_scouting");
			break;
		case Ship::ShipStates::kExpeditionPortspaceFound:
			lua_pushstring(L, "exp_found_port_space");
			break;
		case Ship::ShipStates::kExpeditionColonizing:
			lua_pushstring(L, "exp_colonizing");
			break;
		case Ship::ShipStates::kSinkRequest:
			lua_pushstring(L, "sink_request");
			break;
		case Ship::ShipStates::kSinkAnimation:
			lua_pushstring(L, "sink_animation");
		}
		return 1;
	}
	return 0;
}

int LuaShip::get_scouting_direction(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	if (is_a(Game, &egbase)) {
		switch (get(L, egbase)->get_scouting_direction()) {
		case WalkingDir::WALK_NE:
			lua_pushstring(L, "ne");
			break;
		case WalkingDir::WALK_E:
			lua_pushstring(L, "e");
			break;
		case WalkingDir::WALK_SE:
			lua_pushstring(L, "se");
			break;
		case WalkingDir::WALK_SW:
			lua_pushstring(L, "sw");
			break;
		case WalkingDir::WALK_W:
			lua_pushstring(L, "w");
			break;
		case WalkingDir::WALK_NW:
			lua_pushstring(L, "nw");
			break;
		case WalkingDir::IDLE:
			return 0;
		}
		return 1;
	}
	return 0;
}

int LuaShip::set_scouting_direction(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	if (upcast(Game, game, &egbase)) {
		std::string dirname = luaL_checkstring(L, 3);
		WalkingDir dir = WalkingDir::IDLE;

		if (dirname == "ne") {
			dir = WalkingDir::WALK_NE;
		} else if (dirname == "e") {
			dir = WalkingDir::WALK_E;
		} else if (dirname == "se") {
			dir = WalkingDir::WALK_SE;
		} else if (dirname == "sw") {
			dir = WalkingDir::WALK_SW;
		} else if (dirname == "w") {
			dir = WalkingDir::WALK_W;
		} else if (dirname == "nw") {
			dir = WalkingDir::WALK_NW;
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

      (RW) actual direction if the ship sails around an island.
      Sets/returns cw, ccw or nil

*/
int LuaShip::get_island_explore_direction(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	if (is_a(Game, &egbase)) {
		switch (get(L, egbase)->get_island_explore_direction()) {
		case IslandExploreDirection::kCounterClockwise:
			lua_pushstring(L, "ccw");
			break;
		case IslandExploreDirection::kClockwise:
			lua_pushstring(L, "cw");
			break;
		case IslandExploreDirection::kNotSet:
			return 0;
		}
		return 1;
	}
	return 0;
}

int LuaShip::set_island_explore_direction(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	if (upcast(Game, game, &egbase)) {
		Ship* ship = get(L, egbase);
		std::string dir = luaL_checkstring(L, 3);
		if (dir == "ccw") {
			game->send_player_ship_explore_island(*ship, IslandExploreDirection::kCounterClockwise);
		} else if (dir == "cw") {
			game->send_player_ship_explore_island(*ship, IslandExploreDirection::kClockwise);
		} else {
			return 0;
		}
		return 1;
	}
	return 0;
}

/* RST
   .. attribute:: shipname

   Get name of ship:

      (RO) returns the :class:`string` ship's name.


*/
int LuaShip::get_shipname(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	Ship* ship = get(L, egbase);
	lua_pushstring(L, ship->get_shipname().c_str());
	return 1;
}

/* RST
   .. attribute:: capacity

   The ship's current capacity. Defaults to the capacity defined in the tribe's singleton ship
   description.

   Do not change this value if the ship is currently shipping more items than the new capacity
   allows.

      (RW) returns the current capacity of this ship

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
   .. method:: get_wares()

      Returns the number of wares on this ship. This does not implement
      everything that :class:`HasWares` offers.

      :returns: the number of wares
*/
// UNTESTED
int LuaShip::get_wares(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	int nwares = 0;
	WareInstance* ware;
	Ship* ship = get(L, egbase);
	for (uint32_t i = 0; i < ship->get_nritems(); ++i) {
		const ShippingItem& item = ship->get_item(i);
		item.get(egbase, &ware, nullptr);
		if (ware != nullptr) {
			++nwares;
		}
	}
	lua_pushint32(L, nwares);
	return 1;
}

/* RST
   .. method:: get_workers()

      Returns the number of workers on this ship. This does not implement
      everything that :class:`HasWorkers` offers.

      :returns: the number of workers
*/
// UNTESTED
int LuaShip::get_workers(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	int nworkers = 0;
	Worker* worker;
	Ship* ship = get(L, egbase);
	for (uint32_t i = 0; i < ship->get_nritems(); ++i) {
		const ShippingItem& item = ship->get_item(i);
		item.get(egbase, nullptr, &worker);
		if (worker != nullptr) {
			++nworkers;
		}
	}
	lua_pushint32(L, nworkers);
	return 1;
}

/* RST
   .. method:: build_colonization_port()

      Returns true if port space construction was started (ship was in adequate
      status and a found portspace was nearby)

      :returns: true/false
*/
int LuaShip::build_colonization_port(lua_State* L) {
	EditorGameBase& egbase = get_egbase(L);
	Ship* ship = get(L, egbase);
	if (ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {
		if (upcast(Game, game, &egbase)) {
			game->send_player_ship_construct_port(*ship, ship->exp_port_spaces().front());
			return 1;
		}
	}
	return 0;
}

/* RST
   .. method:: make_expedition([items])

      Turns this ship into an expedition ship without a base port. Creates all necessary
      wares and a builder plus, if desired, the specified additional items.
      Any items previously present in the ship will be deleted.

      The ship must be empty and not an expedition ship when this method is called.

      :returns: nil
*/
int LuaShip::make_expedition(lua_State* L) {
	upcast(Game, game, &get_egbase(L));
	assert(game);
	Ship* ship = get(L, *game);
	assert(ship);
	if (ship->get_ship_state() != Widelands::Ship::ShipStates::kTransport ||
	    ship->get_nritems() > 0) {
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
			Widelands::DescriptionIndex index = game->tribes().ware_index(what);
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
		for (; pair.second; --pair.second) {
			ship->add_item(*game, Widelands::ShippingItem(tribe.get_worker_descr(pair.first)
			                                                 ->create(*game, ship->get_owner(),
			                                                          nullptr, ship->get_position())));
		}
	}

	ship->set_destination(*game, nullptr);
	ship->start_task_expedition(*game);

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

   Child of: :class:`Bob`

   All workers that are visible on the map are of this kind.

   More properties are available through this object's
   :class:`WorkerDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaWorker::className[] = "Worker";
const MethodType<LuaWorker> LuaWorker::Methods[] = {
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
	EditorGameBase& egbase = get_egbase(L);
	return upcasted_map_object_to_lua(
	   L, dynamic_cast<BaseImmovable*>(get(L, egbase)->get_location(egbase)));
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
Soldier
-------

.. class:: Soldier

   Child of: :class:`Worker`

   All soldiers that are on the map are represented by this class.

   More properties are available through this object's
   :class:`SoldierDescription`, which you can access via :any:`MapObject.descr`.
*/

const char LuaSoldier::className[] = "Soldier";
const MethodType<LuaSoldier> LuaSoldier::Methods[] = {
   {nullptr, nullptr},
};
const PropertyType<LuaSoldier> LuaSoldier::Properties[] = {
   PROP_RO(LuaSoldier, attack_level), PROP_RO(LuaSoldier, defense_level),
   PROP_RO(LuaSoldier, health_level), PROP_RO(LuaSoldier, evade_level),
   {nullptr, nullptr, nullptr},
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
   Field has two Triangles associated with itself: the right and the down one.

   You cannot instantiate this directly, access it via
   wl.Game().map.get_field() instead.
*/

const char LuaField::className[] = "Field";
const MethodType<LuaField> LuaField::Methods[] = {
   METHOD(LuaField, __eq),     METHOD(LuaField, __tostring),   METHOD(LuaField, region),
   METHOD(LuaField, has_caps), METHOD(LuaField, has_max_caps), {nullptr, nullptr},
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
   PROP_RO(LuaField, initial_resource_amount),
   PROP_RO(LuaField, claimers),
   PROP_RO(LuaField, owner),
   PROP_RO(LuaField, buildable),
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
int LuaField::get_x(lua_State* L) {
	lua_pushuint32(L, coords_.x);
	return 1;
}
int LuaField::get_y(lua_State* L) {
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
	FCoords f = fcoords(L);

	if (f.field->get_height() == height) {
		return 0;
	}

	if (height > MAX_FIELD_HEIGHT) {
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);
	}

	EditorGameBase& egbase = get_egbase(L);
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
	FCoords f = fcoords(L);

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
      this field for the current interactive player
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
      :const:`nil`

      :see also: :attr:`resource_amount`
*/
int LuaField::get_resource(lua_State* L) {

	const ResourceDescription* rDesc =
	   get_egbase(L).world().get_resource(fcoords(L).field->get_resources());

	lua_pushstring(L, rDesc ? rDesc->name().c_str() : "none");

	return 1;
}
int LuaField::set_resource(lua_State* L) {
	auto& egbase = get_egbase(L);
	DescriptionIndex res = egbase.world().resource_index(luaL_checkstring(L, -1));

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
	EditorGameBase& egbase = get_egbase(L);
	auto c = fcoords(L);
	DescriptionIndex res = c.field->get_resources();
	auto amount = luaL_checkint32(L, -1);
	const ResourceDescription* resDesc = egbase.world().get_resource(res);
	ResourceAmount max_amount = resDesc ? resDesc->max_amount() : 0;

	if (amount < 0 || amount > max_amount) {
		report_error(L, "Illegal amount: %i, must be >= 0 and <= %i", amount,
		             static_cast<unsigned int>(max_amount));
	}

	auto* map = egbase.mutable_map();
	if (is_a(Game, &egbase)) {
		map->set_resources(c, amount);
	} else {
		// in editor, reset also initial amount
		map->initialize_resources(c, res, amount);
	}
	return 0;
}
/* RST
   .. attribute:: initial_resource_amount

      (RO) Starting value of resource. It is set be resource_amount

      :see also: :attr:`resource`
*/
int LuaField::get_initial_resource_amount(lua_State* L) {
	lua_pushuint32(L, fcoords(L).field->get_initial_res_amount());
	return 1;
}
/* RST
   .. attribute:: immovable

      (RO) The immovable that stands on this field or :const:`nil`. If you want
      to remove an immovable, you can use :func:`wl.map.MapObject.remove`.
*/
int LuaField::get_immovable(lua_State* L) {
	BaseImmovable* bi = get_egbase(L).map().get_immovable(coords_);

	if (!bi) {
		return 0;
	} else {
		upcasted_map_object_to_lua(L, bi);
	}
	return 1;
}

/* RST
   .. attribute:: bobs

      (RO) An :class:`array` of :class:`~wl.map.Bob` that are associated
      with this field
*/
// UNTESTED
int LuaField::get_bobs(lua_State* L) {
	Bob* b = fcoords(L).field->get_first_bob();

	lua_newtable(L);
	uint32_t cidx = 1;
	while (b) {
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
	TerrainDescription& td = get_egbase(L).world().terrain_descr(fcoords(L).field->terrain_r());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
int LuaField::set_terr(lua_State* L) {
	const char* name = luaL_checkstring(L, -1);
	EditorGameBase& egbase = get_egbase(L);
	try {
		const DescriptionIndex td = egbase.mutable_world()->load_terrain(name);
		egbase.mutable_map()->change_terrain(
		   egbase, TCoords<FCoords>(fcoords(L), TriangleIndex::R), td);
	} catch (const Widelands::GameDataError& e) {
		report_error(L, "set_terr: %s", e.what());
	}

	lua_pushstring(L, name);
	return 1;
}

int LuaField::get_terd(lua_State* L) {
	TerrainDescription& td = get_egbase(L).world().terrain_descr(fcoords(L).field->terrain_d());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
int LuaField::set_terd(lua_State* L) {
	const char* name = luaL_checkstring(L, -1);
	EditorGameBase& egbase = get_egbase(L);
	try {
		const DescriptionIndex td = egbase.mutable_world()->load_terrain(name);
		egbase.mutable_map()->change_terrain(
		   egbase, TCoords<FCoords>(fcoords(L), TriangleIndex::D), td);
	} catch (const Widelands::GameDataError& e) {
		report_error(L, "set_terd: %s", e.what());
	}

	lua_pushstring(L, name);
	return 1;
}

/* RST
   .. attribute:: rn, ln, brn, bln, trn, tln

      (RO) The neighbour fields of this field. The abbreviations stand for:

      * rn -- Right neighbour
      * ln -- Left neighbour
      * brn -- Bottom right neighbour
      * bln -- Bottom left neighbour
      * trn -- Top right neighbour
      * tln -- Top left neighbour

      Note that the widelands map wraps at its borders, that is the following
      holds:

      .. code-block:: lua

         wl.map.Field(wl.map.get_width()-1, 10).rn == wl.map.Field(0, 10)
*/
#define GET_X_NEIGHBOUR(X)                                                                         \
	int LuaField::get_##X(lua_State* L) {                                                           \
		Coords n;                                                                                    \
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

/* RST
   .. attribute:: owner

      (RO) The current owner of the field or :const:`nil` if noone owns it. See
      also :attr:`claimers`.
*/
int LuaField::get_owner(lua_State* L) {
	PlayerNumber current_owner = fcoords(L).field->get_owned_by();
	if (current_owner) {
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
	const NodeCaps caps = fcoords(L).field->nodecaps();
	const bool is_buildable = (caps & BUILDCAPS_FLAG) || (caps & BUILDCAPS_SMALL) ||
	                          (caps & BUILDCAPS_MEDIUM) || (caps & BUILDCAPS_BIG) ||
	                          (caps & BUILDCAPS_MINE);
	lua_pushboolean(L, is_buildable);
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
	EditorGameBase& egbase = get_egbase(L);
	const Map& map = egbase.map();

	std::vector<PlrInfluence> claimers;

	iterate_players_existing(other_p, map.get_nrplayers(), egbase, plr)
	   claimers.push_back(PlrInfluence(
	      plr->player_number(), plr->military_influence(map.get_index(coords_, map.get_width()))));

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
	lua_pushboolean(L, (*get_user_class<LuaField>(L, -1))->coords_ == coords_);
	return 1;
}

int LuaField::__tostring(lua_State* L) {
	const std::string pushme = (boost::format("Field(%i,%i)") % coords_.x % coords_.y).str();
	lua_pushstring(L, pushme);
	return 1;
}

/* RST
   .. function:: region(r1[, r2])

      Returns an array of all Fields inside the given region. If one argument
      is given it defines the radius of the region. If both arguments are
      specified, the first one defines the outer radius and the second one the
      inner radius and a hollow region is returned, that is all fields in the
      outer radius region minus all fields in the inner radius region.

      A small example:

      .. code-block:: lua

         f:region(1)

      will return an array with the following entries (Note: Ordering of the
      fields inside the array is not guaranteed):

      .. code-block:: lua

         {f, f.rn, f.ln, f.brn, f.bln, f.tln, f.trn}

      :returns: The array of the given fields.
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

      Returns :const:`true` if the field has this caps associated
      with it, otherwise returns false.
      Note: Immovables will hide the caps. If you want to have the caps
      without immovables use has_max_caps instead

      :arg capname: can be either of

      * :const:`small`: Can a small building be built here?
      * :const:`medium`: Can a medium building be built here?
      * :const:`big`: Can a big building be built here?
      * :const:`mine`: Can a mine be built here?
      * :const:`port`: Can a port be built here?
      * :const:`flag`: Can a flag be built here?
      * :const:`walkable`: Is this field passable for walking bobs?
      * :const:`swimmable`: Is this field passable for swimming bobs?
*/
int LuaField::has_caps(lua_State* L) {
	const FCoords& f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);
	lua_pushboolean(L, check_has_caps(L, query, f, f.field->nodecaps(), get_egbase(L).map()));
	return 1;
}

/* RST
   .. method:: has_max_caps(capname)

      Returns :const:`true` if the field has this maximum caps (not taking immovables into account)
      associated with it, otherwise returns false.

      :arg capname: can be either of

      * :const:`small`: Can a small building be built here?
      * :const:`medium`: Can a medium building be built here?
      * :const:`big`: Can a big building be built here?
      * :const:`mine`: Can a mine be built here?
      * :const:`port`: Can a port be built here?
      * :const:`flag`: Can a flag be built here?
      * :const:`walkable`: Is this field passable for walking bobs?
      * :const:`swimmable`: Is this field passable for swimming bobs?
*/
int LuaField::has_max_caps(lua_State* L) {
	const FCoords& f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);
	lua_pushboolean(
	   L, check_has_caps(L, luaL_checkstring(L, 2), f, f.field->maxcaps(), get_egbase(L).map()));
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
int LuaField::region(lua_State* L, uint32_t radius) {
	const Map& map = get_egbase(L).map();
	MapRegion<Area<FCoords>> mr(map, Area<FCoords>(fcoords(L), radius));

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		const FCoords& loc = mr.location();
		to_lua<LuaField>(L, new LuaField(loc.x, loc.y));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

int LuaField::hollow_region(lua_State* L, uint32_t radius, uint32_t inner_radius) {
	const Map& map = get_egbase(L).map();
	HollowArea<Area<>> har(Area<>(coords_, radius), inner_radius);

	MapHollowRegion<Area<>> mr(map, har);

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
   PROP_RO(LuaPlayerSlot, tribe_name),
   PROP_RO(LuaPlayerSlot, name),
   PROP_RO(LuaPlayerSlot, starting_field),
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

      (RO) The name of the tribe suggested for this player in this map
*/
int LuaPlayerSlot::get_tribe_name(lua_State* L) {
	lua_pushstring(L, get_egbase(L).map().get_scenario_player_tribe(player_number_));
	return 1;
}

/* RST
   .. attribute:: name

      (RO) The name for this player as suggested in this map
*/
int LuaPlayerSlot::get_name(lua_State* L) {
	lua_pushstring(L, get_egbase(L).map().get_scenario_player_name(player_number_));
	return 1;
}

/* RST
   .. attribute:: starting_field

      (RO) The starting_field for this player as set in the map.
      Note that it is not guaranteed that the HQ of the player is on this
      field as scenarios and starting conditions are free to place the HQ
      wherever it want. This field is only centered when the game starts.
*/
int LuaPlayerSlot::get_starting_field(lua_State* L) {
	to_lua<LuaField>(L, new LuaField(get_egbase(L).map().get_starting_pos(player_number_)));
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
