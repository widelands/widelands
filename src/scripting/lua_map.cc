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

#include "container_iterate.h"
#include "log.h"
#include "logic/carrier.h"
#include "logic/checkstep.h"
#include "logic/findimmovable.h"
#include "logic/immovable.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/warelist.h"
#include "logic/widelands_geometry.h"
#include "wui/mapviewpixelconstants.h"

#include "c_utils.h"
#include "lua_game.h"

#include "lua_map.h"

using namespace Widelands;

/* RST
:mod:`wl.map`
=============

.. module:: wl.map
   :synopsis: Provides access on maps and fields

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.map

*/

/*
 * ========================================================================
 *                         HELPER FUNCTIONS
 * ========================================================================
 */
/*
 * Upcast the given base immovable to a higher type and hand this to
 * Lua. We use this so that scripters always work with the highest class
 * object available.
 */
#define CAST_TO_LUA(k) to_lua<L_ ##k> \
   (L, new L_ ##k(*static_cast<k *>(bi)))
int upcasted_immovable_to_lua(lua_State * L, BaseImmovable * bi) {
	if (!bi)
		return 0;

	switch  (bi->get_type()) {
		case Map_Object::BUILDING:
			if (!strcmp(bi->type_name(), "warehouse"))
				return CAST_TO_LUA(Warehouse);
			else if (!strcmp(bi->type_name(), "productionsite"))
				return CAST_TO_LUA(ProductionSite);
			else
				return CAST_TO_LUA(Building);

		case Map_Object::FLAG:
			return CAST_TO_LUA(Flag);
		case Map_Object::ROAD:
			return CAST_TO_LUA(Road);
	}
	return to_lua<L_BaseImmovable>(L, new L_BaseImmovable(*bi));
}
#undef CAST_TO_LUA



/*
 * ========================================================================
 *                         MODULE CLASSES
 * ========================================================================
 */

/* RST
Module Classes
^^^^^^^^^^^^^^

*/

/* RST
MapObject
----------

.. class:: MapObject

	This is the base class for all Objects in widelands, including immovables
	and Bobs. This class can't be instantiated directly, but provides the base
	for all others.
*/
const char L_MapObject::className[] = "MapObject";
const MethodType<L_MapObject> L_MapObject::Methods[] = {
	METHOD(L_MapObject, remove),
	METHOD(L_MapObject, __eq),
	{0, 0},
};
const PropertyType<L_MapObject> L_MapObject::Properties[] = {
	PROP_RO(L_MapObject, serial),
	{0, 0, 0},
};

void L_MapObject::__persist(lua_State * L) {
		Map_Map_Object_Saver & mos = *get_mos(L);
		Game & game = get_game(L);

		uint32_t idx = 0;
		if (m_ptr && m_ptr->get(game))
			idx = mos.get_object_file_index(*m_ptr->get(game));

		PERS_UINT32("file_index", idx);
	}
void L_MapObject::__unpersist(lua_State * L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx);

	if (!idx)
		m_ptr = 0;
	else {
		Map_Map_Object_Loader & mol = *get_mol(L);

		m_ptr = new Object_Ptr(&mol.get<Map_Object>(idx));
	}
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: serial

		(RO)
		The serial number of this object. Note that this value does not stay
		constant after saving/loading.
*/
int L_MapObject::get_serial(lua_State * L) {
	Game & game = get_game(L);
	lua_pushuint32(L, m_ptr->get(game)->serial());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_MapObject::__eq(lua_State * L) {
	Game & game = get_game(L);
	L_MapObject * other = *get_base_user_class<L_MapObject>(L, -1);

	Map_Object * me = m_get_or_zero(game);
	Map_Object * you = other->m_get_or_zero(game);


	// Both objects are destroyed: they are equal
	if (me == you) lua_pushboolean(L, true);
	else if (!me or !you) // One of the objects is destroyed: they are distinct
		lua_pushboolean(L, false);
	else // Compare them
		lua_pushboolean
			(L, other->get(game, L)->serial() == get(game, L)->serial());

	return 1;
}

/* RST
	.. method:: remove()

		Removes this object from the game immediately. If you want to destroy an
		object as if the player had see :func:`destroy`.

		:returns: :const:`nil`
*/
int L_MapObject::remove(lua_State * L) {
	Game & game = get_game(L);
	Map_Object * o = get(game, L);

	if (!o)
		return 0;

	o->remove(game);
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Map_Object * L_MapObject::get
	(Game & game, lua_State * L, std::string name)
{
	Map_Object * o = m_get_or_zero(game);
	if (!o)
		luaL_error(L, "%s no longer exists!", name.c_str());
	return o;
}
Map_Object * L_MapObject::m_get_or_zero (Game & game)
{
	Map_Object * o = 0;
	if (m_ptr)
		o = m_ptr->get(game);
	return o;
}



/* RST
BaseImmovable
-------------

.. class:: BaseImmovable

	Child of: :class:`MapObject`

	This is the base class for all Objects in widelands, including immovables
	and Bobs. This class can't be instantiated directly, but provides the base
	for all others.
*/
const char L_BaseImmovable::className[] = "BaseImmovable";
const MethodType<L_BaseImmovable> L_BaseImmovable::Methods[] = {
	{0, 0},
};
const PropertyType<L_BaseImmovable> L_BaseImmovable::Properties[] = {
	PROP_RO(L_BaseImmovable, size),
	PROP_RO(L_BaseImmovable, name),
	{0, 0, 0},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: size

		(RO) The size of this immovable. Can be either of

		* :const:`none` -- Example: mushrooms. Immovables will be destroyed when
			something else is build on this field.
		* :const:`small` -- Example: trees or flags
		* :const:`medium` -- Example: Medium sized buildings
		* :const:`big` -- Example: Big sized buildings or stones
*/
int L_BaseImmovable::get_size(lua_State * L) {
	Game & game = get_game(L);
	BaseImmovable * o = get(game, L);

	switch (o->get_size()) {
		case BaseImmovable::NONE: lua_pushstring(L, "none"); break;
		case BaseImmovable::SMALL: lua_pushstring(L, "small"); break;
		case BaseImmovable::MEDIUM: lua_pushstring(L, "medium"); break;
		case BaseImmovable::BIG: lua_pushstring(L, "big"); break;
		default:
			return
				luaL_error
					(L, "Unknown size in L_BaseImmovable::get_size: %i",
					 o->get_size());
			break;
	}
	return 1;
}

/* RST
	.. attribute:: name

		(RO) The internal name of this immovable. This is the same as the
		directory name of this immovable in the tribe or world directory.
*/
int L_BaseImmovable::get_name(lua_State * L) {
	Game & game = get_game(L);
	BaseImmovable * o = get(game, L);

	lua_pushstring(L, o->name().c_str());
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
*/
const char L_PlayerImmovable::className[] = "PlayerImmovable";
const MethodType<L_PlayerImmovable> L_PlayerImmovable::Methods[] = {
	{0, 0},
};
const PropertyType<L_PlayerImmovable> L_PlayerImmovable::Properties[] = {
	PROP_RO(L_PlayerImmovable, player),
	{0, 0, 0},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: player

		(RO) The :class:`wl.game.Player` who owns this object.
*/
int L_PlayerImmovable::get_player(lua_State * L) {
	return
		to_lua<L_Player>
			(L, new L_Player (get(get_game(L), L)->get_owner()->player_number())
	);
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
#define GET_INDEX(name, capname) \
Ware_Index L_PlayerImmovable::m_get_ ## name ## _index \
	(lua_State * L, PlayerImmovable * i, const std::string & what) \
{ \
	Ware_Index idx = i->get_owner()->tribe(). name ## _index(what); \
	if (!idx) \
		luaL_error(L, "Invalid " #capname ": %s", what.c_str()); \
	return idx; \
}
GET_INDEX(ware, Ware);
GET_INDEX(worker, Worker);
#undef GET_INDEX


/* RST
Flag
--------

.. class:: Flag

	Child of: :class:`PlayerImmovable`

	One flag in the economy of this Player.
*/
const char L_Flag::className[] = "Flag";
const MethodType<L_Flag> L_Flag::Methods[] = {
	METHOD(L_Flag, add_ware),
	{0, 0},
};
const PropertyType<L_Flag> L_Flag::Properties[] = {
	{0, 0, 0},
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
	.. method:: add_ware(ware)

		Adds a ware to this flag. The ware is created from thin air
		and added to the players economy. Reports an error If there is no
		capacity on this flag.

		:arg ware: name of ware to create
		:type ware: :class:`string`
		:returns: :const:`nil`
*/
int L_Flag::add_ware(lua_State * L)
{
	Game & game = get_game(L);
	Flag * f = get(game, L);

	if (not f->has_capacity())
		return luaL_error(L, "Flag has no capacity left!");

	Ware_Index idx = m_get_ware_index(L, f, luaL_checkstring(L, 2));


	Item_Ware_Descr const & wd = *f->get_owner()->tribe().get_ware_descr(idx);
	WareInstance & item = *new WareInstance(idx, &wd);
	item.init(game);

	f->add_item(game, item);
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */


/* RST
Road
--------

.. class:: Road

	Child of: :class:`PlayerImmovable`

	One flag in the economy of this Player.
*/
const char L_Road::className[] = "Road";
const MethodType<L_Road> L_Road::Methods[] = {
	METHOD(L_Road, warp_workers),
	{0, 0},
};
const PropertyType<L_Road> L_Road::Properties[] = {
	PROP_RO(L_Road, length),
	PROP_RO(L_Road, start_flag),
	PROP_RO(L_Road, end_flag),
	PROP_RO(L_Road, workers),
	PROP_RO(L_Road, valid_workers),
	PROP_RO(L_Road, type),
	{0, 0, 0},
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
int L_Road::get_length(lua_State * L) {
	lua_pushuint32(L, get(get_game(L), L)->get_path().get_nsteps());
	return 1;
}

/* RST
	.. attribute:: start_flag

		(RO) The flag were this road starts
*/
int L_Road::get_start_flag(lua_State * L) {
	return
		to_lua<L_Flag>
			(L, new L_Flag(get(get_game(L), L)->get_flag(Road::FlagStart)));
}

/* RST
	.. attribute:: end_flag

		(RO) The flag were this road ends
*/
int L_Road::get_end_flag(lua_State * L) {
	return
		to_lua<L_Flag>
			(L, new L_Flag(get(get_game(L), L)->get_flag(Road::FlagEnd)));
}

/* RST
	.. attribute:: workers

		(RO) An array of names of carriers that work on this road.  Note that you
		cannot change this directly, use :meth:`warp_workers` to overwrite any
		worker.
*/
int L_Road::get_workers(lua_State * L) {
	const PlayerImmovable::Workers & ws = get(get_game(L), L)->get_workers();

	lua_createtable(L, ws.size(), 0);
	uint32_t widx = 1;
	container_iterate_const(PlayerImmovable::Workers, ws, w) {
		lua_pushuint32(L, widx++);
		lua_pushstring(L, (*w.current)->descr().name().c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

/* RST
	.. attribute:: valid_workers

		(RO) an array of names of workers that are allowed to work in this
		productionsite. If of one type more than one is allowed, it will appear
		more than once.
*/
int L_Road::get_valid_workers(lua_State * L) {
	lua_newtable(L);
	lua_pushuint32(L, 1);
	lua_pushstring(L, "carrier");
	lua_rawset(L, -3);
	return 1;
}

/* RST
	.. attribute:: type

		(RO) Type of road. Can be any either of:

		* normal
		* busy
*/
int L_Road::get_type(lua_State * L) {
	switch (get(get_game(L), L)->get_roadtype()) {
		case Road_Normal:
			lua_pushstring(L, "normal"); break;
		case Road_Busy:
			lua_pushstring(L, "busy"); break;
		default:
			return
				luaL_error(L, "Unknown Roadtype! This is a bug in widelands!");
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

/* RST
	.. method:: warp_workers(workers)

		Immediately creates a worker out of thin air and
		assigns it the to the road. Currently only carriers can
		be created like this (e.g. no 'oxes' and therelike).

		:arg names: array of names of workers, e.g. "carrier", "ox"
		:type name: :class:`array`

		:seealso: wl.map.ProductionSite.warp_workers
*/
int L_Road::warp_workers(lua_State * L) {
	if (lua_gettop(L) != 2)
		return luaL_error(L, "Only one carrier can be warped here!");

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushuint32(L, 1);
	lua_rawget(L, -2);
	std::string name = luaL_checkstring(L, -1);

	if (name != "carrier")
		return luaL_error(L, "Only 'carrier' is allowed currently!\n");

	Game & g = get_game(L);
	Map & map = g.map();

	Road * r = get(g, L);

	if (r->get_workers().size())
		return luaL_error(L, "No space for this worker!");

	Flag & start = r->get_flag(Road::FlagStart);

	Coords idle_position = start.get_position();
	const Path & path = r->get_path();

	// Determine Idle position.
	Path::Step_Vector::size_type idle_index = r->get_idle_index();
	for (Path::Step_Vector::size_type i = 0; i < idle_index; ++i)
		map.get_neighbour(idle_position, path[i], &idle_position);

	Player & owner = r->owner();

	Tribe_Descr const & tribe = owner.tribe();
	const Worker_Descr * wd = tribe.get_worker_descr(tribe.worker_index(name));
	if (!wd)
		return luaL_error(L, "%s is not a valid worker name!", name.c_str());

	if (wd->get_worker_type() != Worker_Descr::CARRIER)
		return luaL_error(L, "%s is not a carrier type!", name.c_str());

	Carrier & carrier = ref_cast<Carrier, Worker>
		(wd->create (g, owner, r, idle_position));

	carrier.start_task_road(g);

	r->assign_carrier(carrier, 0);

	return 0;
}

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
*/
const char L_Building::className[] = "Building";
const MethodType<L_Building> L_Building::Methods[] = {
	{0, 0},
};
const PropertyType<L_Building> L_Building::Properties[] = {
	PROP_RO(L_Building, building_type),
	{0, 0, 0},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: building_type

		(RO) What type of building is this. Can be either of:

		* constructionsite
		* militarysite
		* productionsite
		* trainingsite
		* warehouse
*/
int L_Building::get_building_type(lua_State * L) {
	lua_pushstring(L, get(get_game(L), L)->type_name());
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

	Child of: :class:`Building`

	Every Headquarter or Warehouse on the Map is of this type.
*/
const char L_Warehouse::className[] = "Warehouse";
const MethodType<L_Warehouse> L_Warehouse::Methods[] = {
	METHOD(L_Warehouse, set_wares),
	METHOD(L_Warehouse, get_wares),
	METHOD(L_Warehouse, set_workers),
	METHOD(L_Warehouse, get_workers),
	METHOD(L_Warehouse, set_soldiers),
	// METHOD(L_Warehouse, get_soldiers),
	{0, 0},
};
const PropertyType<L_Warehouse> L_Warehouse::Properties[] = {
	{0, 0, 0},
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
	.. method:: set_wares(which[, amount])

		Sets the wares available in this warehouse. Either takes two arguments,
		an ware name and an amount to set it too. Or it takes a table of name,
		amount pairs.

		:arg which: name of ware or name, amount table
		:type which: :class:`string` or :class:`table`
		:arg amount: this many units will be available after the call
		:type amount: :class:`integer`

		:returns: :const:`nil`
*/
#define SET_X(what) \
int L_Warehouse::set_ ##what ## s(lua_State * L) { \
	Warehouse * o = get(get_game(L), L); \
	int n = lua_gettop(L); \
 \
	if (n == 3) { \
		/* String, count combination */ \
		Ware_Index idx = m_get_ ## what ## _index(L, o, luaL_checkstring(L, 2)); \
		uint32_t set_point = luaL_checkuint32(L, 3); \
 \
		uint32_t current = o->get_ ## what ## s().stock(idx); \
		int32_t diff = set_point - current; \
		if (diff < 0) \
			o->remove_ ## what ## s(idx, -diff); \
		else if (diff > 0) \
			o->insert_ ## what ## s(idx, diff); \
 \
	} else { \
		luaL_checktype(L, 2, LUA_TTABLE); \
 \
		/* Table, iterate over */ \
		lua_pushnil(L);  /* first key */ \
		while (lua_next(L, 2) != 0) { \
			std::string which = luaL_checkstring(L, -2); \
			uint32_t set_point = luaL_checkuint32(L, -1); \
			Ware_Index idx = m_get_ ## what ## _index(L, o, which); \
 \
			uint32_t current = o->get_ ## what ## s().stock(idx); \
			int32_t diff = set_point - current; \
			if (diff < 0) \
				o->remove_ ## what ## s(idx, -diff); \
			else if (diff > 0) \
				o->insert_ ## what ## s(idx, diff); \
 \
			lua_pop(L, 1); /* pop value, keep key for next iteration */ \
		} \
	} \
	return 0; \
}
SET_X(ware);

/* RST
	.. method:: get_wares(which)

		Gets the number of wares in this warehouse. If :const:`which` is
		a :class:`string`, returns a single integer. If :const:`which` is a
		:class:`table`, you can query more than one ware. The return value then
		will be a table with ware names as key and integers as values.

		:arg which: which ware(s) to query
		:type which: :class:`string` or :class:`table` in array form

		:returns: :class:`integer` or :class:`table`
*/
#define GET_X(what) \
int L_Warehouse::get_ ##what ## s(lua_State * L) { \
	Warehouse * o = get(get_game(L), L); \
 \
	if (lua_isstring(L, 2)) { \
		/* One string as argument */ \
		lua_pushuint32 \
			(L, o->get_ ##what ## s().stock \
				(m_get_ ##what ## _index(L, o, luaL_checkstring(L, 2))) \
		); \
	} else { \
		/* Table as argument, iterate over it */ \
		luaL_checktype(L, 2, LUA_TTABLE); \
 \
		/* create return value table */ \
		lua_newtable(L); \
		int table_idx = lua_gettop(L); \
 \
		lua_pushnil(L);  /* first key */ \
		while (lua_next(L, 2) != 0) { \
			std::string which = luaL_checkstring(L, -1); \
 \
			lua_pushuint32(L, o->get_##what## s().stock \
					(m_get_## what ##_index(L, o, which))); \
			lua_settable(L, table_idx); /* pops name count */ \
		} \
	} \
	return 1; \
}
GET_X(ware);

/* RST
	.. method:: set_workers(which[, amount])

		Analogous to :meth:`set_wares`, but for workers.
*/
SET_X(worker);
#undef SET_X

/* RST
	.. method:: get_workers(which)

		Analogous to :meth:`get_wares` but for workers.
*/
GET_X(worker);
#undef GET_X

/* RST
	.. method:: set_soldiers(which[, amount])

		Analogous to :meth:`set_wares`, but for soldiers. Instead of
		a name a :class:`array` is used to define the soldier. The array contains
		the level for hitpoints, attack, defense, evade in this order.  A usage
		example:

		.. code-block:: lua

			w:set_soldiers({0,0,0,0}, 100)

		would add 100 level 0 soldiers. While

		.. code-block:: lua

			w:set_soldiers{
			  [{0,0,0,0}] = 10,
			  [{1,2,3,4}] = 5,
			)

		would add 10 level 0 soldier and 5 soldiers with hit point level 1,
		attack level 2, defense level 3 and evade level 4 (as long as this is
		legal for the players tribe).
*/
static int _get_soldier_levels
	(lua_State * L, int tidx, uint32_t * hp, uint32_t * a,
	 uint32_t * d, uint32_t * e, const Soldier_Descr & sd)
{
	lua_pushuint32(L, 1);
	lua_rawget(L, tidx);
	*hp = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (*hp > sd.get_max_hp_level())
		return
			luaL_error
			(L, "hp level (%i) > max hp level (%i)", *hp, sd.get_max_hp_level());

	lua_pushuint32(L, 2);
	lua_rawget(L, tidx);
	*a = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (*a > sd.get_max_attack_level())
		return
			luaL_error
			(L, "attack level (%i) > max attack level (%i)",
			 *a, sd.get_max_attack_level());

	lua_pushuint32(L, 3);
	lua_rawget(L, tidx);
	*d = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (*d > sd.get_max_defense_level())
		return
			luaL_error
			(L, "defense level (%i) > max defense level (%i)",
			 *d, sd.get_max_defense_level());

	lua_pushuint32(L, 4);
	lua_rawget(L, tidx);
	*e = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (*e > sd.get_max_evade_level())
		return
			luaL_error
			(L, "evade level (%i) > max evade level (%i)",
			 *e, sd.get_max_evade_level());

	return 0;
}
int L_Warehouse::set_soldiers(lua_State * L) {
	Game & game = get_game(L);
	Warehouse * wh = get(game, L);
	Tribe_Descr const & tribe = wh->owner().tribe();

	Soldier_Descr const & soldier_descr =  //  soldiers
		ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));

	uint32_t hp, a, d, e, count;
	if (lua_gettop(L) > 2) {
		// STACK: cls, descr, count
		count = luaL_checkuint32(L, 3);
		_get_soldier_levels(L, 2, &hp, &a, &d, &e, soldier_descr);

		for (uint32_t j = count; j; --j) {
			Soldier & soldier =
				ref_cast<Soldier, Worker>
				(soldier_descr.create(game, wh->owner(), wh, wh->get_position()));
			soldier.set_level(hp, a, d, e);
			wh->incorporate_worker(game, soldier);
		}
	} else {
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			_get_soldier_levels(L, 3, &hp, &a, &d, &e, soldier_descr);
			count = luaL_checkuint32(L, -1);

			for (uint32_t j = count; j; --j) {
				Soldier & soldier =
					ref_cast<Soldier, Worker>
					(soldier_descr.create
					 (game, wh->owner(), wh, wh->get_position()));
				soldier.set_level(hp, a, d, e);
				wh->incorporate_worker(game, soldier);
			}

			lua_pop(L, 1);
		}
	}
	return 0;
}

#if 0
// This is currently not available because Requirements do not allow
// to check for more then one attribute. This seems easy to add, but
// alters many things in the source, so I left sensing soldiers out
// for the moment
int L_Warehouse::get_soldiers(lua_State * L) {

	Requirements req;

	Game & game = get_game(L);
	Warehouse * wh = get(game, L);
	Tribe_Descr const & tribe = wh->owner().tribe();

	Soldier_Descr const & soldier_descr =  //  soldiers
		ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));

	uint32_t hp, a, d, e, count;

	luaL_checktype(L, 2, LUA_TTABLE);

	lua_pushuint32(L, 1);
	lua_rawget(L, 2);


	if (lua_isnumber(L, -1)) {
		lua_pop(L, 1);
		// STACK: cls, descr, count
		count = luaL_checkuint32(L, 3);
		_get_soldier_levels(L, 2, &hp, &a, &d, &e, soldier_descr);

		for (uint32_t j = count; j; --j) {
			Soldier & soldier =
				ref_cast<Soldier, Worker>
				(soldier_descr.create(game, wh->owner(), wh, wh->get_position()));
			soldier.set_level(hp, a, d, e);
			wh->incorporate_worker(game, soldier);
		}
	} else {
		lua_pop(L, 1);

// lua_pushnil(L);
// while (lua_next(L, 2) != 0) {
//                   _get_soldier_levels(L, 3, &hp, &a,
//                   &d, &e, soldier_descr);
//                   count = luaL_checkuint32(L, -1);
//
//                   for (uint32_t j = count; j; --j) {
//                           Soldier & soldier =
//                                   ref_cast<Soldier, Worker>
//                                   (soldier_descr.create
//                                    (game, wh->owner(), wh,
//                                    wh->get_position()));
//                           soldier.set_level(hp, a, d, e);
//                           wh->incorporate_worker(game, soldier);
//                   }
//
//                   lua_pop(L, 1);
//           }
	}

	m_warehouse->count_workers
				(game, req.get_index(), req.get_requirements());
	return 1;
}
#endif

/*
 ==========================================================
 C METHODS
 ==========================================================
 */


/* RST
ProductionSite
--------------

.. class:: ProductionSite

	Child of: :class:`Building`

	Every building that produces anything.
*/
const char L_ProductionSite::className[] = "ProductionSite";
const MethodType<L_ProductionSite> L_ProductionSite::Methods[] = {
	METHOD(L_ProductionSite, warp_workers),
	{0, 0},
};
const PropertyType<L_ProductionSite> L_ProductionSite::Properties[] = {
	PROP_RO(L_ProductionSite, valid_workers),
	PROP_RO(L_ProductionSite, workers),
	{0, 0, 0},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: valid_workers

		(RO) an array of names of workers that are allowed to work in this
		productionsite. If of one type more than one is allowed, it will appear
		more than once.
*/
int L_ProductionSite::get_valid_workers(lua_State * L) {
	Game & g = get_game(L);
	ProductionSite * ps = get(g, L);

	lua_newtable(L);

	Tribe_Descr const & tribe = ps->owner().tribe();
	uint32_t idx = 1;
	container_iterate_const(Ware_Types, ps->descr().working_positions(), i) {
		std::string name = tribe.get_worker_descr(i.current->first)->name();

		for (uint32_t j = 0; j < i.current->second; j++) {
			lua_pushuint32(L, idx++);
			lua_pushstring(L, name);
			lua_rawset(L, -3);
		}
	}
	return 1;
}

/* RST
	.. attribute:: workers

		(RO) An array of names of workers that work here.  Note that you
		cannot change this directly, use warp_workers to overwrite any worker.
*/
int L_ProductionSite::get_workers(lua_State * L) {
	const PlayerImmovable::Workers & ws = get(get_game(L), L)->get_workers();

	lua_createtable(L, ws.size(), 0);
	uint32_t widx = 1;
	container_iterate_const(PlayerImmovable::Workers, ws, w) {
		lua_pushuint32(L, widx++);
		lua_pushstring(L, (*w.current)->descr().name().c_str());
		lua_rawset(L, -3);
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
	.. method:: warp_workers(workers)

		Warp workers into this building. The workers are created from thin air.

		:arg workers: array of worker names. If more than one worker of this type
			should be warped, the worker has to be named more than once.

		:seealso: wl.map.Road.warp_workers
*/
int L_ProductionSite::warp_workers(lua_State * L) {

	luaL_checktype(L, 2, LUA_TTABLE);

	Game & g = get_game(L);
	ProductionSite * ps = get(g, L);
	Tribe_Descr const & tribe = ps->owner().tribe();

	Ware_Types const & working_positions = ps->descr().working_positions();

	lua_pushnil(L);  /* first key */
	while (lua_next(L, 2) != 0) {
		std::string name = luaL_checkstring(L, -1);
		const Worker_Descr * wdes = tribe.get_worker_descr
			(tribe.worker_index(name));
		if (!wdes)
			return luaL_error(L, "%s is not a valid worker name!", name.c_str());

		bool success = false;
		container_iterate_const(Ware_Types, working_positions, i) {
			if (i.current->first == wdes->worker_index()) {
				if (!ps->warp_worker(g, *wdes)) {
					success = true;
					break;
				} else
					return luaL_error(L, "No space left for this worker");
			}
		}
		if (!success)
			return
				luaL_error
				  (L, "%s is not a valid worker for this site!", name.c_str());

		lua_pop(L, 1); /* pop value, keep key for next iteration */
	}

	return 0;
}

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
*/

const char L_Field::className[] = "Field";
const MethodType<L_Field> L_Field::Methods[] = {
	METHOD(L_Field, __eq),
	METHOD(L_Field, region),
	{0, 0},
};
const PropertyType<L_Field> L_Field::Properties[] = {
	PROP_RO(L_Field, x),
	PROP_RO(L_Field, y),
	PROP_RO(L_Field, rn),
	PROP_RO(L_Field, ln),
	PROP_RO(L_Field, trn),
	PROP_RO(L_Field, tln),
	PROP_RO(L_Field, bln),
	PROP_RO(L_Field, brn),
	PROP_RO(L_Field, immovable),
	PROP_RW(L_Field, terr),
	PROP_RW(L_Field, terd),
	PROP_RW(L_Field, height),
	PROP_RO(L_Field, viewpoint_x),
	PROP_RO(L_Field, viewpoint_y),
	PROP_RW(L_Field, resource),
	PROP_RW(L_Field, resource_amount),
	{0, 0, 0},
};


L_Field::L_Field(lua_State * L) {
	Map & m = get_game(L).map();
	uint32_t rv = luaL_checkuint32(L, 1);
	if (rv >= static_cast<uint32_t>(m.get_width()))
		luaL_error(L, "x coordinate out of range!");
	m_c.x = rv;
	rv = luaL_checkuint32(L, 2);
	if (rv >= static_cast<uint32_t>(m.get_height()))
		luaL_error(L, "y coordinate out of range!");
	m_c.y = rv;
}

void L_Field::__persist(lua_State * L) {
	PERS_INT32("x", m_c.x); PERS_INT32("y", m_c.y);
}
void L_Field::__unpersist(lua_State * L) {
	UNPERS_INT32("x", m_c.x); UNPERS_INT32("y", m_c.y);
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: x, y

		(RO) The x/y coordinate of this field
*/
int L_Field::get_x(lua_State * L) {lua_pushuint32(L, m_c.x); return 1;}
int L_Field::get_y(lua_State * L) {lua_pushuint32(L, m_c.y); return 1;}

/* RST
	.. attribute:: height

		(RW) The height of this field. The default height is 10, you can increase
		or decrease this value to build mountains. Note though that if you change
		this value too much, all surrounding fields will also change their
		heights because the slope is constrained.
*/
int L_Field::get_height(lua_State * L) {
	lua_pushuint32(L, fcoords(L).field->get_height());
	return 1;
}
int L_Field::set_height(lua_State * L) {
	uint32_t height = luaL_checkuint32(L, -1);
	if (height > MAX_FIELD_HEIGHT)
		luaL_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);

	get_game(L).map().set_height(fcoords(L), height);

	return get_height(L);
}

/* RST
	.. attribute:: viewpoint_x, viewpoint_y

		(RO) Returns the position in pixels to move the view to to center
		this field for the current interactive player
*/
int L_Field::get_viewpoint_x(lua_State * L) {
	lua_pushuint32(L, m_c.x * TRIANGLE_WIDTH);
	return 1;
}
int L_Field::get_viewpoint_y(lua_State * L) {
	lua_pushuint32(L, m_c.y * TRIANGLE_HEIGHT);
	return 1;
}

/* RST
	.. attribute:: resource

		(RO) The name of the resource that is available in this field or
		:const:`nil`

		:see also: :attr:`resource_amount`
*/
int L_Field::get_resource(lua_State * L) {
	lua_pushstring
		(L, get_game(L).map().world().get_resource
			 (fcoords(L).field->get_resources())->name().c_str());

	return 1;
}
int L_Field::set_resource(lua_State * L) {
	Field * field = fcoords(L).field;
	int32_t res = get_game(L).map().world().get_resource
		(luaL_checkstring(L, -1));

	if (res == -1)
		return luaL_error(L, "Illegal resource: '%s'", luaL_checkstring(L, -1));

	field->set_resources(res, field->get_resources_amount());

	return 0;
}

/* RST
	.. attribute:: resource_amount

		(RO) How many items of the resource is available in this field.

		:see also: :attr:`resource`
*/
int L_Field::get_resource_amount(lua_State * L) {
	lua_pushuint32(L, fcoords(L).field->get_resources_amount());
	return 1;
}
int L_Field::set_resource_amount(lua_State * L) {
	Field * field = fcoords(L).field;
	int32_t res = field->get_resources();
	int32_t amount = luaL_checkint32(L, -1);
	int32_t max_amount = get_game(L).map().world().get_resource
		(res)->get_max_amount();

	if (amount < 0 or amount > max_amount)
		return
			luaL_error
			(L, "Illegal amount: %i, must be >= 0 and <= %i", amount, max_amount);

	field->set_resources(res, amount);

	return 0;
}

/* RST
	.. attribute:: immovable

		(RO) The immovable that stands on this field or :const:`nil`. If you want
		to remove an immovable, you can use :func:`wl.map.MapObject.remove`.
*/
int L_Field::get_immovable(lua_State * L) {
	BaseImmovable * bi = get_game(L).map().get_immovable(m_c);

	if (!bi)
		return 0;
	else
		upcasted_immovable_to_lua(L, bi);
	return 1;
}

/* RST
	.. attribute:: terr, terd

		(RW) The terrain of the right/down triangle. This is a string value
		containing the name of the terrain as it is defined in the world
		configuration. You can change the terrain by simply assigning another
		valid name to these variables.
*/
int L_Field::get_terr(lua_State * L) {
	Terrain_Descr & td =
		get_game(L).map().world().terrain_descr
			(fcoords(L).field->terrain_r());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
int L_Field::set_terr(lua_State * L) {
	const char * name = luaL_checkstring(L, -1);
	Map & map = get_game(L).map();
	Terrain_Index td =
		map.world().index_of_terrain(name);
	if (td == static_cast<Terrain_Index>(-1))
		luaL_error(L, "Unknown terrain '%s'", name);

	map.change_terrain(TCoords<FCoords>(fcoords(L), TCoords<FCoords>::R), td);


	lua_pushstring(L, name);
	return 1;
}

int L_Field::get_terd(lua_State * L) {
	Terrain_Descr & td =
		get_game(L).map().world().terrain_descr
			(fcoords(L).field->terrain_d());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
int L_Field::set_terd(lua_State * L) {
	const char * name = luaL_checkstring(L, -1);
	Map & map = get_game(L).map();
	Terrain_Index td =
		map.world().index_of_terrain(name);
	if (td == static_cast<Terrain_Index>(-1))
		luaL_error(L, "Unknown terrain '%s'", name);

	map.change_terrain
		(TCoords<FCoords> (fcoords(L), TCoords<FCoords>::D), td);

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
#define GET_X_NEIGHBOUR(X) int L_Field::get_ ##X(lua_State* L) { \
   Coords n; \
   get_game(L).map().get_ ##X(m_c, &n); \
   to_lua<L_Field>(L, new L_Field(n.x, n.y)); \
	return 1; \
}
GET_X_NEIGHBOUR(rn);
GET_X_NEIGHBOUR(ln);
GET_X_NEIGHBOUR(trn);
GET_X_NEIGHBOUR(tln);
GET_X_NEIGHBOUR(bln);
GET_X_NEIGHBOUR(brn);

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_Field::__eq(lua_State * L) {
	lua_pushboolean(L, (*get_user_class<L_Field>(L, -1))->m_c == m_c);
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
int L_Field::region(lua_State * L) {
	uint32_t n = lua_gettop(L);

	if (n == 3) {
		uint32_t radius = luaL_checkuint32(L, -2);
		uint32_t inner_radius = luaL_checkuint32(L, -1);
		return m_hollow_region(L, radius, inner_radius);
	} else {
		uint32_t radius = luaL_checkuint32(L, -1);
		return m_region(L, radius);
	}
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
int L_Field::m_region(lua_State * L, uint32_t radius)
{
	Map & map = get_game(L).map();
	MapRegion<Area<FCoords> > mr
		(map, Area<FCoords>(fcoords(L), radius));

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		const FCoords & loc = mr.location();
		to_lua<L_Field>(L, new L_Field(loc.x, loc.y));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

int L_Field::m_hollow_region
	(lua_State * L, uint32_t radius, uint32_t inner_radius)
{
	Map & map = get_game(L).map();
	HollowArea<Area<> > har(Area<>(m_c, radius), inner_radius);

	MapHollowRegion<Area<> > mr(map, har);

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		to_lua<L_Field>(L, new L_Field(mr.location()));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
}

const Widelands::FCoords L_Field::fcoords(lua_State * L) {
	return get_game(L).map().get_fcoords(m_c);
}

/*
 * ========================================================================
 *                            MODULE FUNCTIONS
 * ========================================================================
 */

/* RST
Module Functions
^^^^^^^^^^^^^^^^

*/

/* RST
.. function:: create_immovable(name, field)

	Creates an immovable that is defined by the world (e.g. trees, stones...)
	on a given field. If there is already an immovable on the field, an error
	is reported.

	:arg name: The name of the immovable to create
	:type name: :class:`string`
	:arg field: The immovable is created on this field.
	:type field: :class:`wl.map.Field`

	:returns: The created immovable object, most likely a
		:class:`wl.map.BaseImmovable`
*/
static int L_create_immovable(lua_State * const L) {
	char const * const objname = luaL_checkstring(L, 1);
	L_Field * c = *get_user_class<L_Field>(L, 2);

	Game & game = get_game(L);

	// Check if the map is still free here
	if
	 (BaseImmovable const * const imm = c->fcoords(L).field->get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return luaL_error(L, "Node is no longer free!");

	int32_t const imm_idx = game.map().world().get_immovable_index(objname);
	if (imm_idx < 0)
		return luaL_error(L, "Unknown immovable <%s>", objname);

	BaseImmovable & m = game.create_immovable(c->coords(), imm_idx, 0);

	return upcasted_immovable_to_lua(L, &m);
}

/* RST
.. function:: get_width()

	Returns the width of the map in fields

	:returns: width of map in nr of fields
	:rtype: :class:`integer`
*/
int L_get_width(lua_State * L) {
	lua_pushuint32(L, get_game(L).map().get_width());
	return 1;
}
/* RST
.. function:: get_height()

	Returns the height of the map in fields

	:returns: height of map in nr of fields
	:rtype: :class:`integer`
*/
int L_get_height(lua_State * L) {
	lua_pushuint32(L, get_game(L).map().get_height());
	return 1;
}

const static struct luaL_reg wlmap [] = {
	{"create_immovable", &L_create_immovable},
	{"get_width", &L_get_width},
	{"get_height", &L_get_height},
	{0, 0}
};

void luaopen_wlmap(lua_State * L) {
	luaL_register(L, "wl.map", wlmap);
	lua_pop(L, 1); // pop the table from the stack

	register_class<L_Field>(L, "map");
	register_class<L_MapObject>(L, "map");

	register_class<L_BaseImmovable>(L, "map", true);
	add_parent<L_BaseImmovable, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_PlayerImmovable>(L, "map", true);
	add_parent<L_PlayerImmovable, L_BaseImmovable>(L);
	add_parent<L_PlayerImmovable, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Building>(L, "map", true);
	add_parent<L_Building, L_PlayerImmovable>(L);
	add_parent<L_Building, L_BaseImmovable>(L);
	add_parent<L_Building, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Flag>(L, "map", true);
	add_parent<L_Flag, L_PlayerImmovable>(L);
	add_parent<L_Flag, L_BaseImmovable>(L);
	add_parent<L_Flag, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Road>(L, "map", true);
	add_parent<L_Road, L_PlayerImmovable>(L);
	add_parent<L_Road, L_BaseImmovable>(L);
	add_parent<L_Road, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Warehouse>(L, "map", true);
	add_parent<L_Warehouse, L_Building>(L);
	add_parent<L_Warehouse, L_PlayerImmovable>(L);
	add_parent<L_Warehouse, L_BaseImmovable>(L);
	add_parent<L_Warehouse, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_ProductionSite>(L, "map", true);
	add_parent<L_ProductionSite, L_Building>(L);
	add_parent<L_ProductionSite, L_PlayerImmovable>(L);
	add_parent<L_ProductionSite, L_BaseImmovable>(L);
	add_parent<L_ProductionSite, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table
}

