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

#include "scripting/map/lua_ship.h"

#include "economy/portdock.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/pinned_note.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "scripting/globals.h"

namespace LuaMaps {

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
	if (Widelands::PinnedNote* note = ship->get_destination_note(egbase); note != nullptr) {
		return upcasted_map_object_to_lua(L, note);
	}

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
			const Widelands::Coords portspace = ship->current_portspace();
			assert(static_cast<bool>(portspace));
			game->send_player_ship_construct_port(*ship, portspace);
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

}  // namespace LuaMaps
