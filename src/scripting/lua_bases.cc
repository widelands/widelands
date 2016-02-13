/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#include "scripting/lua_bases.h"

#include <boost/format.hpp>

#include "economy/economy.h"
#include "logic/constants.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/player.h"
#include "scripting/factory.h"
#include "scripting/globals.h"
#include "scripting/lua_map.h"


using namespace Widelands;

namespace LuaBases {

/* RST
:mod:`wl.bases`
==================

.. module:: wl.bases
   :synopsis: Base functions for common features in Game and Editor

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.bases

The :mod:`wl.bases` module contains Base functions on which the classes
of :mod:`wl.game` and :mod:`wl.editor` are based. You will not need
to create any of the functions in this module directly, but you might
use their functionality via the child classes.
*/

/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^^^

*/


/* RST
EditorGameBase
--------------

.. class:: EditorGameBase

	Common functionality between Editor and Game.
*/

const char LuaEditorGameBase::className[] = "EditorGameBase";
const MethodType<LuaEditorGameBase> LuaEditorGameBase::Methods[] = {
	METHOD(LuaEditorGameBase, get_building_description),
	METHOD(LuaEditorGameBase, get_tribe_description),
	METHOD(LuaEditorGameBase, get_ware_description),
	METHOD(LuaEditorGameBase, get_worker_description),
	{nullptr, nullptr},
};
const PropertyType<LuaEditorGameBase> LuaEditorGameBase::Properties[] = {
	PROP_RO(LuaEditorGameBase, map),
	PROP_RO(LuaEditorGameBase, players),
	{nullptr, nullptr, nullptr},
};


void LuaEditorGameBase::__persist(lua_State * /* L */) {
}
void LuaEditorGameBase::__unpersist(lua_State * /* L */) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: map

		(RO) The :class:`~wl.map.Map` the game is played on.
*/
int LuaEditorGameBase::get_map(lua_State * L) {
	to_lua<LuaMaps::LuaMap>(L, new LuaMaps::LuaMap());
	return 1;
}

/* RST
	.. attribute:: players

		(RO) An :class:`array` with the defined players. The players are
		either of type :class:`wl.game.Player` or :class:`wl.editor.Player`.

		In game, there might be less players then defined in
		:attr:`wl.map.Map.player_slots` because some slots might not be taken.
		Also note that for the same reason you cannot index this array with
		:attr:`wl.bases.PlayerBase.number`, but the players are ordered with
		increasing number in this array.

		The editor always creates all players that are defined by the map.
*/
int LuaEditorGameBase::get_players(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);

	lua_newtable(L);

	uint32_t idx = 1;
	for (PlayerNumber i = 1; i <= MAX_PLAYERS; i++) {
		Player * rv = egbase.get_player(i);
		if (!rv)
			continue;

		lua_pushuint32(L, idx++);
		get_factory(L).push_player(L, i);
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
	.. function:: get_building_description(building_description.name)

		:arg building_name: the name of the building

		Registers a building description so Lua can reference it from the game.

		(RO) The :class:`~wl.Game.Building_description`.
*/
int LuaEditorGameBase::get_building_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string building_name = luaL_checkstring(L, 2);
	const DescriptionIndex building_index = tribes.building_index(building_name);
	if (!tribes.building_exists(building_index)) {
		report_error(L, "Building %s does not exist", building_name.c_str());
	}
	const BuildingDescr* building_description = tribes.get_building_descr(building_index);

	return LuaMaps::upcasted_map_object_descr_to_lua(L, building_description);
}

/* RST
	.. function:: get_tribe_description(tribe_name)

		:arg tribe_name: the name of the tribe

		Registers a tribe description so Lua can reference it from the game.

		(RO) The :class:`~wl.Game.Tribe_description`.
*/
int LuaEditorGameBase::get_tribe_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	EditorGameBase& egbase = get_egbase(L);
	const std::string tribe_name = luaL_checkstring(L, 2);
	if (!egbase.tribes().tribe_exists(tribe_name)) {
		report_error(L, "Tribe %s does not exist", tribe_name.c_str());
	}
	const TribeDescr* descr = egbase.tribes().get_tribe_descr(egbase.tribes().tribe_index(tribe_name));
	return to_lua<LuaMaps::LuaTribeDescription>(L, new LuaMaps::LuaTribeDescription(descr));
}



/* RST
	.. function:: get_ware_description(ware_description.name)

		:arg ware_name: the name of the ware

		Registers a ware description so Lua can reference it from the game.

		(RO) The :class:`~wl.Game.Ware_description`.
*/
int LuaEditorGameBase::get_ware_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string ware_name = luaL_checkstring(L, 2);
	DescriptionIndex ware_index = tribes.ware_index(ware_name);
	if (!tribes.ware_exists(ware_index)) {
		report_error(L, "Ware %s does not exist", ware_name.c_str());
	}
	const WareDescr* ware_description = tribes.get_ware_descr(ware_index);
	return LuaMaps::upcasted_map_object_descr_to_lua(L, ware_description);
}


/* RST
	.. function:: get_worker_description(worker_description.name)

		:arg worker_name: the name of the worker

		Registers a worker description so Lua can reference it from the game.

		(RO) The :class:`~wl.Game.Worker_description`.
*/
int LuaEditorGameBase::get_worker_description(lua_State* L) {
	if (lua_gettop(L) != 2) {
		report_error(L, "Wrong number of arguments");
	}
	const Tribes& tribes = get_egbase(L).tribes();
	const std::string worker_name = luaL_checkstring(L, 2);
	const DescriptionIndex worker_index = tribes.worker_index(worker_name);
	if (!tribes.worker_exists(worker_index)) {
		report_error(L, "Worker %s does not exist", worker_name.c_str());
	}
	const WorkerDescr* worker_description = tribes.get_worker_descr(worker_index);
	return LuaMaps::upcasted_map_object_descr_to_lua(L, worker_description);
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */


/* RST
PlayerBase
----------

.. class:: PlayerBase

	The Base class for the Player objects in Editor and Game
*/

const char LuaPlayerBase::className[] = "PlayerBase";
const MethodType<LuaPlayerBase> LuaPlayerBase::Methods[] = {
	METHOD(LuaPlayerBase, __eq),
	METHOD(LuaPlayerBase, __tostring),
	METHOD(LuaPlayerBase, conquer),
	METHOD(LuaPlayerBase, get_wares),
	METHOD(LuaPlayerBase, get_workers),
	METHOD(LuaPlayerBase, place_building),
	METHOD(LuaPlayerBase, place_flag),
	METHOD(LuaPlayerBase, place_road),
	METHOD(LuaPlayerBase, place_ship),
	{nullptr, nullptr},
};
const PropertyType<LuaPlayerBase> LuaPlayerBase::Properties[] = {
	PROP_RO(LuaPlayerBase, number),
	PROP_RO(LuaPlayerBase, tribe_name),
	{nullptr, nullptr, nullptr},
};

void LuaPlayerBase::__persist(lua_State * L) {
	PERS_UINT32("player", player_number_);
}
void LuaPlayerBase::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", player_number_);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: number

		(RO) The number of this Player.
*/
int LuaPlayerBase::get_number(lua_State * L) {
	lua_pushuint32(L, player_number_);
	return 1;
}

/* RST
	.. attribute:: tribe_name

		(RO) The name of the tribe of this player.
*/
int LuaPlayerBase::get_tribe_name(lua_State * L) {
	lua_pushstring(L, get(L, get_egbase(L)).tribe().name());
	return 1;
}


/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int LuaPlayerBase::__eq(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);
	const Player & me = get(L, egbase);
	const Player & you =
		(*get_base_user_class<LuaPlayerBase>(L, 2))->get(L, egbase);

	lua_pushboolean
		(L, (me.player_number() == you.player_number()));
	return 1;
}

int LuaPlayerBase::__tostring(lua_State * L) {
	const std::string pushme =
			(boost::format("Player(%i)")
			 % static_cast<unsigned int>(get(L, get_egbase(L)).player_number())).str();
	lua_pushstring(L, pushme.c_str());
	return 1;
}
/* RST
	.. function:: place_flag(field[, force])

		Builds a flag on a given field if it is legal to do so. If not,
		reports an error

		:arg field: where the flag should be created
		:type field: :class:`wl.map.Field`
		:arg force: If this is :const:`true` then the map is created with
			pure force:

				* if there is an immovable on this field, it will be
				  removed
				* if there are flags too close by to this field, they will be
				  ripped
				* if the player does not own the territory, it is conquered
				  for him.
		:type force: :class:`boolean`
		:returns: :class:`wl.map.Flag` object created or :const:`nil`.
*/
int LuaPlayerBase::place_flag(lua_State * L) {
	uint32_t n = lua_gettop(L);
	LuaMaps::LuaField * c = *get_user_class<LuaMaps::LuaField>(L, 2);
	bool force = false;
	if (n > 2)
		force = luaL_checkboolean(L, 3);

	Flag * f;
	if (!force) {
		f = get(L, get_egbase(L)).build_flag(c->fcoords(L));
		if (!f)
			report_error(L, "Couldn't build flag!");
	} else {
		f = &get(L, get_egbase(L)).force_flag(c->fcoords(L));
	}

	return to_lua<LuaMaps::LuaFlag>(L, new LuaMaps::LuaFlag(*f));
}

/* RST
	.. method:: place_road(f1, dir1, dir2, ...[, force=false])

		Start a road at the given field, then walk the directions
		given. Places a flag at the last field.

		If the last argument to this function is :const:`true` the road will
		be created by force: all immovables in the way are removed and land
		is conquered.

		:arg f1: fields to connect with this road
		:type f1: :class:`wl.map.Field`
		:arg dirs: direction, can be either ("r", "l", "br", "bl", "tr", "tl") or
			("e", "w", "ne", "nw", "se", "sw").
		:type dirs: :class:`string`

		:returns: the road created
*/
int LuaPlayerBase::place_road(lua_State * L) {
	EditorGameBase & egbase = get_egbase(L);
	Map & map = egbase.map();

	Flag * starting_flag =
		(*get_user_class<LuaMaps::LuaFlag> (L, 2))->get(L, egbase);
	Coords current = starting_flag->get_position();
	Path path(current);

	bool force_road = false;
	if (lua_isboolean(L, -1)) {
		force_road = luaL_checkboolean(L, -1);
		lua_pop(L, 1);
	}

	// Construct the path
	CheckStepLimited cstep;
	for (int32_t i = 3; i <= lua_gettop(L); i++) {
		std::string d = luaL_checkstring(L, i);

		if (d == "ne" || d == "tr") {
			path.append(map, 1);
			map.get_trn(current, &current);
		} else if (d == "e" || d == "r") {
			path.append(map, 2);
			map.get_rn(current, &current);
		} else if (d == "se" || d == "br") {
			path.append(map, 3);
			map.get_brn(current, &current);
		} else if (d == "sw" || d == "bl") {
			path.append(map, 4);
			map.get_bln(current, &current);
		} else if (d == "w" || d == "l") {
			path.append(map, 5);
			map.get_ln(current, &current);
		} else if (d == "nw" || d == "tl") {
			path.append(map, 6);
			map.get_tln(current, &current);
		} else
			report_error(L, "Illegal direction: %s", d.c_str());

		cstep.add_allowed_location(current);
	}

	// Make sure that the road cannot cross itself
	Path optimal_path;
	map.findpath
		(path.get_start(), path.get_end(),
		 0,
		 optimal_path,
		 cstep,
		 Map::fpBidiCost);
	if (optimal_path.get_nsteps() != path.get_nsteps())
		report_error(L, "Cannot build a road that crosses itself!");

	Road * r = nullptr;
	if (force_road) {
		r = &get(L, egbase).force_road(path);
	} else {
		BaseImmovable * bi = map.get_immovable(current);
		if (!bi || bi->descr().type() != MapObjectType::FLAG) {
			if (!get(L, egbase).build_flag(current))
				report_error(L, "Could not place end flag!");
		}
		if (bi && bi == starting_flag)
		  report_error(L, "Cannot build a closed loop!");

		r = get(L, egbase).build_road(path);
	}

	if (!r) {
		report_error(L,
		             "Error while creating Road. May be: something is in "
		             "the way or you do not own the territory were you want to build "
		             "the road");
	}

	return to_lua<LuaMaps::LuaRoad>(L, new LuaMaps::LuaRoad(*r));
}

/* RST
	.. method:: place_building(name, field[, cs = false, force = false])

		Immediately creates a building on the given field. The building starts
		out completely empty. If :const:`cs` is set, the building
		is not created directly, instead a constructionsite for this building is
		placed.

		If the :const:`force` argument is set, the building is forced into
		existence: the same action is taken as for :meth:`place_flag` when force
		is :const:`true`. Additionally, all buildings that are too close to the
		new one are ripped.

		:returns: a subclass of :class:`wl.map.Building`
*/
int LuaPlayerBase::place_building(lua_State * L) {
	const std::string& name = luaL_checkstring(L, 2);
	LuaMaps::LuaField * c = *get_user_class<LuaMaps::LuaField>(L, 3);
	bool constructionsite = false;
	bool force = false;

	if (lua_gettop(L) >= 4)
		constructionsite = luaL_checkboolean(L, 4);
	if (lua_gettop(L) >= 5)
		force = luaL_checkboolean(L, 5);

	const Tribes& tribes = get_egbase(L).tribes();

	if (!tribes.building_exists(name)) {
		report_error(L, "Unknown Building: '%s'", name.c_str());
	}
	DescriptionIndex building_index = tribes.building_index(name);

	BuildingDescr::FormerBuildings former_buildings;
	find_former_buildings(tribes, building_index, &former_buildings);
	if (constructionsite) {
		former_buildings.pop_back();
	}

	Building * b = nullptr;
	if (force) {
		if (constructionsite) {
			b = &get(L, get_egbase(L)).force_csite
				(c->coords(), building_index, former_buildings);
		} else {
			b = &get(L, get_egbase(L)).force_building
				(c->coords(), former_buildings);
		}
	} else {
		b = get(L, get_egbase(L)).build
			(c->coords(), building_index, constructionsite, former_buildings);
	}
	if (!b)
		report_error(L, "Couldn't place building!");

	LuaMaps::upcasted_map_object_to_lua(L, b);
	return 1;
}

/* RST
	.. method:: place_ship(field)

		Places a ship for the player's tribe, which will be
		owned by the player.

		:arg field: where the ship should be placed.
		:type field: :class:`wl.map.Field`

		:returns: The new ship that was created.
*/
// UNTESTED
int LuaPlayerBase::place_ship(lua_State * L) {
	LuaMaps::LuaField* c = *get_user_class<LuaMaps::LuaField>(L, 2);

	EditorGameBase & egbase = get_egbase(L);
	Player& player = get(L, egbase);

	const ShipDescr* descr = egbase.tribes().get_ship_descr(player.tribe().ship());
	Bob& ship = egbase.create_ship(c->coords(), descr->name(), &player);

	LuaMaps::upcasted_map_object_to_lua(L, &ship);

	return 1;
}

/* RST
	.. method:: conquer(f[, radius=1])

		Conquer this area around the given field if it does not belong to the
		player already. This will conquer the fields no matter who owns it at the
		moment.

		:arg f: center field for conquering
		:type f: :class:`wl.map.Field`
		:arg radius: radius to conquer around. Default value makes this call
			conquer 7 fields
		:type radius: :class:`integer`
		:returns: :const:`nil`
*/
int LuaPlayerBase::conquer(lua_State * L) {
	uint32_t radius = 1;
	if (lua_gettop(L) > 2)
		radius = luaL_checkuint32(L, 3);

	get_egbase(L).conquer_area_no_building
		(PlayerArea<Area<FCoords> >
			(player_number_, Area<FCoords>
				((*get_user_class<LuaMaps::LuaField>(L, 2))->fcoords(L), radius))
	);
	return 0;
}

/* RST
	.. method:: get_workers(name)

		Returns the number of workers of this type in the players stock. This does not implement
		everything that :class:`HasWorkers` offers.

		:arg name: name of the worker to get
		:type name: :class:`string`.
		:returns: the number of wares
*/
// UNTESTED
int LuaPlayerBase::get_workers(lua_State * L) {
	Player& player = get(L, get_egbase(L));
	const std::string workername = luaL_checkstring(L, -1);

	const DescriptionIndex worker = player.tribe().worker_index(workername);

	uint32_t nworkers = 0;
	for (uint32_t i = 0; i < player.get_nr_economies(); ++i) {
		 nworkers += player.get_economy_by_number(i)->stock_worker(worker);
	}
	lua_pushuint32(L, nworkers);
	return 1;
}

/* RST
	.. method:: get_wares(name)

		Returns the number of wares of this type in the players stock. This does not implement
		everything that :class:`HasWorkers` offers.

		:arg name: name of the worker to get
		:type name: :class:`string`.
		:returns: the number of wares
*/
// UNTESTED
int LuaPlayerBase::get_wares(lua_State * L) {
	EditorGameBase& egbase = get_egbase(L);
	Player& player = get(L, egbase);
	const std::string warename = luaL_checkstring(L, -1);

	const DescriptionIndex ware = egbase.tribes().ware_index(warename);

	uint32_t nwares = 0;
	for (uint32_t i = 0; i < player.get_nr_economies(); ++i) {
		 nwares += player.get_economy_by_number(i)->stock_ware(ware);
	}
	lua_pushuint32(L, nwares);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Player & LuaPlayerBase::get
		(lua_State * L, Widelands::EditorGameBase & egbase)
{
	if (player_number_ > MAX_PLAYERS)
		report_error(L, "Illegal player number %i",  player_number_);
	Player * rv = egbase.get_player(player_number_);
	if (!rv)
		report_error(L, "Player with the number %i does not exist", player_number_);
	return *rv;
}


/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */


const static struct luaL_Reg wlbases [] = {
	{nullptr, nullptr}
};

void luaopen_wlbases(lua_State * const L) {
	lua_getglobal(L, "wl");  // S: wl_table
	lua_pushstring(L, "bases"); // S: wl_table "bases"
	luaL_newlib(L, wlbases);  // S: wl_table "bases" wl.bases_table
	lua_settable(L, -3); // S: wl_table
	lua_pop(L, 1); // S:

	register_class<LuaEditorGameBase>(L, "bases");
	register_class<LuaPlayerBase>(L, "bases");
}

}
