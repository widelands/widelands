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

#include "scripting/map/lua_production_site_description.h"

#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker_descr.h"
#include "logic/map_objects/world/critter.h"
#include "scripting/globals.h"
#include "scripting/map/lua_immovable_description.h"
#include "scripting/map/lua_resource_description.h"
#include "scripting/map/lua_ware_description.h"
#include "scripting/map/lua_worker_description.h"

namespace LuaMaps {

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

}  // namespace LuaMaps
