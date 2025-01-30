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

#include "scripting/map/lua_construction_site.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

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

}  // namespace LuaMaps
