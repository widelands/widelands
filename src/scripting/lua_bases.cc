/*
 * Copyright (C) 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <lua.hpp>

#include "logic/checkstep.h"
#include "logic/player.h"

#include "lua_map.h"

#include "lua_bases.h"

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

const char L_EditorGameBase::className[] = "EditorGameBase";
const MethodType<L_EditorGameBase> L_EditorGameBase::Methods[] = {
	{0, 0},
};
const PropertyType<L_EditorGameBase> L_EditorGameBase::Properties[] = {
	PROP_RO(L_EditorGameBase, map),
	PROP_RO(L_EditorGameBase, players),
	{0, 0, 0},
};


void L_EditorGameBase::__persist(lua_State * L) {
}
void L_EditorGameBase::__unpersist(lua_State * L) {
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
int L_EditorGameBase::get_map(lua_State * L) {
	to_lua<LuaMap::L_Map>(L, new LuaMap::L_Map());
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
int L_EditorGameBase::get_players(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);

	lua_newtable(L);

	uint32_t idx = 1;
	for(Player_Number i = 1; i <= MAX_PLAYERS; i++) {
		Player * rv = egbase.get_player(i);
		if (not rv)
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

const char L_PlayerBase::className[] = "PlayerBase";
const MethodType<L_PlayerBase> L_PlayerBase::Methods[] = {
	METHOD(L_PlayerBase, __eq),
	METHOD(L_PlayerBase, place_flag),
	METHOD(L_PlayerBase, place_road),
	METHOD(L_PlayerBase, place_building),
	METHOD(L_PlayerBase, conquer),
	{0, 0},
};
const PropertyType<L_PlayerBase> L_PlayerBase::Properties[] = {
	PROP_RO(L_PlayerBase, number),
	PROP_RO(L_PlayerBase, tribe),
	{0, 0, 0},
};

void L_PlayerBase::__persist(lua_State * L) {
	PERS_UINT32("player", m_pl);
}
void L_PlayerBase::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", m_pl);
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
int L_PlayerBase::get_number(lua_State * L) {
	lua_pushuint32(L, m_pl);
	return 1;
}

/* RST
	.. attribute:: tribe

		(RO) The name of the tribe of this player.
*/
int L_PlayerBase::get_tribe(lua_State *L) {
	lua_pushstring(L, get(L, get_egbase(L)).tribe().name());
	return 1;
}


/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_PlayerBase::__eq(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	const Player & me = get(L, egbase);
	const Player & you =
		(*get_base_user_class<L_PlayerBase>(L, 2))->get(L, egbase);

	lua_pushboolean
		(L, (me.player_number() == you.player_number()));
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
int L_PlayerBase::place_flag(lua_State * L) {
	uint32_t n = lua_gettop(L);
	LuaMap::L_Field * c = *get_user_class<LuaMap::L_Field>(L, 2);
	bool force = false;
	if (n > 2)
		force = luaL_checkboolean(L, 3);

	Flag * f;
	if (not force) {
		f = get(L, get_egbase(L)).build_flag(c->fcoords(L));
		if (!f)
			return report_error(L, "Couldn't build flag!");
	} else {
		f = &get(L, get_egbase(L)).force_flag(c->fcoords(L));
	}

	return to_lua<LuaMap::L_Flag>(L, new LuaMap::L_Flag(*f));
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
int L_PlayerBase::place_road(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	Map & map = egbase.map();

	Flag * starting_flag =
		(*get_user_class<LuaMap::L_Flag> (L, 2))->get(L, egbase);
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

		if (d == "ne" or d == "tr") {
			path.append(map, 1);
			map.get_trn(current, &current);
		} else if (d == "e" or d == "r") {
			path.append(map, 2);
			map.get_rn(current, &current);
		} else if (d == "se" or d == "br") {
			path.append(map, 3);
			map.get_brn(current, &current);
		} else if (d == "sw" or d == "bl") {
			path.append(map, 4);
			map.get_bln(current, &current);
		} else if (d == "w" or d == "l") {
			path.append(map, 5);
			map.get_ln(current, &current);
		} else if (d == "nw" or d == "tl") {
			path.append(map, 6);
			map.get_tln(current, &current);
		} else
			return report_error(L, "Illegal direction: %s", d.c_str());

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
		return report_error(L, "Cannot build a road that crosses itself!");

	Road * r = 0;
	if (force_road) {
		r = &get(L, egbase).force_road(path);
	} else {
		BaseImmovable * bi = map.get_immovable(current);
		if (!bi or bi->get_type() != Map_Object::FLAG) {
			if (!get(L, egbase).build_flag(current))
				return report_error(L, "Could not place end flag!");
		}
		if (bi and bi == starting_flag)
		  return report_error(L, "Cannot build a closed loop!");

		r = get(L, egbase).build_road(path);
	}

	if (!r)
		return
			report_error
			  (L, "Error while creating Road. May be: something is in "
					"the way or you do not own the territory were you want to build "
					"the road");

	return to_lua<LuaMap::L_Road>(L, new LuaMap::L_Road(*r));
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
int L_PlayerBase::place_building(lua_State * L) {
	const char * name = luaL_checkstring(L, 2);
	LuaMap::L_Field * c = *get_user_class<LuaMap::L_Field>(L, 3);
	bool constructionsite = false;
	bool force = false;

	if (lua_gettop(L) >= 4)
		constructionsite = luaL_checkboolean(L, 4);
	if (lua_gettop(L) >= 5)
		force = luaL_checkboolean(L, 5);

	Building_Index i = get(L, get_egbase(L)).tribe().building_index(name);
	if (i == Building_Index::Null())
		return report_error(L, "Unknown Building: '%s'", name);

	Building * b = 0;
	if (force)
		b = &get(L, get_egbase(L)).force_building
			(c->coords(), i, constructionsite);
	else
		b = get(L, get_egbase(L)).build
			(c->coords(), i, constructionsite);

	if (not b)
		return report_error(L, "Couldn't place building!");

	LuaMap::upcasted_immovable_to_lua(L, b);
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
int L_PlayerBase::conquer(lua_State * L) {
	uint32_t radius = 1;
	if (lua_gettop(L) > 2)
		radius = luaL_checkuint32(L, 3);

	get_egbase(L).conquer_area_no_building
		(Player_Area<Area<FCoords> >
			(m_pl, Area<FCoords>
				((*get_user_class<LuaMap::L_Field>(L, 2))->fcoords(L), radius))
	);
	return 0;
}



/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Player & L_PlayerBase::get
		(lua_State * L, Widelands::Editor_Game_Base & egbase)
{
	if (m_pl > MAX_PLAYERS)
		report_error(L, "Illegal player number %i",  m_pl);
	Player * rv = egbase.get_player(m_pl);
	if (!rv)
		report_error(L, "Player with the number %i does not exist", m_pl);
	return *rv;
}


/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */


const static struct luaL_reg wlbases [] = {
	{0, 0}
};

void luaopen_wlbases(lua_State * const L) {
	luaL_register(L, "wl.bases", wlbases);
	lua_pop(L, 1); // pop the table from the stack again

	register_class<L_EditorGameBase>(L, "bases");
	register_class<L_PlayerBase>(L, "bases");
}

};

