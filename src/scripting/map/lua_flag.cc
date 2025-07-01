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

#include "scripting/map/lua_flag.h"

#include "economy/road.h"
#include "logic/player.h"
#include "scripting/globals.h"
#include "scripting/map/lua_economy.h"

namespace LuaMaps {

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
	parse_wares_workers_counted(L, f->owner().tribe(), &setpoints, true, false);
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

}  // namespace LuaMaps
