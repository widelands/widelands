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

#include "scripting/map/lua_warehouse.h"

#include "economy/portdock.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

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

}  // namespace LuaMaps
