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
#include "economy/wares_queue.h"
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
		{
			const char * type_name = bi->type_name();
			if (!strcmp(type_name, "warehouse"))
				return CAST_TO_LUA(Warehouse);
			else if (!strcmp(type_name, "productionsite"))
				return CAST_TO_LUA(ProductionSite);
			else if (!strcmp(type_name, "militarysite"))
				return CAST_TO_LUA(MilitarySite);
			else if (!strcmp(type_name, "trainingsite"))
				return CAST_TO_LUA(ProductionSite);
			else
				return CAST_TO_LUA(Building);
		}

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
 *                         HELPER CLASSES
 * ========================================================================
 */
int _Employer::set_workers(lua_State * L)
{
	Game & g = get_game(L);
	PlayerImmovable * pi = get(L, g);
	const Tribe_Descr & tribe = pi->owner().tribe();

	WorkersMap setpoints = m_parse_set_workers_arguments(L, tribe);

	WorkersMap c_workers;
	container_iterate_const(PlayerImmovable::Workers, pi->get_workers(), w) {
		Ware_Index i = tribe.worker_index((*w.current)->descr().name());
		if (not c_workers.count(i))
			c_workers.insert(L_ProductionSite::WorkerAmount(i, 1));
		else
			c_workers[i] += 1;
		if (not setpoints.count(i))
			setpoints.insert(WorkerAmount(i, 0));
	}

	WorkersMap valid_workers = _valid_workers(*pi);

	// The idea is to change as little as possible
	container_iterate_const(WorkersMap, setpoints, sp) {
		const Worker_Descr * wdes = tribe.get_worker_descr(sp->first);
		if (not valid_workers.count(sp->first))
				  report_error
					  (L, "<%s> can't be employed here!",
					   wdes->name().c_str());

		uint32_t cur = 0;
		WorkersMap::iterator i = c_workers.find(sp->first);
		if (i != c_workers.end())
			cur = i->second;

		int d = sp->second - cur;
		if (d < 0) {
			while (d) {
				container_iterate_const
					(PlayerImmovable::Workers, pi->get_workers(), w)
				{
					Ware_Index i = tribe.worker_index((*w.current)->descr().name());
					if(i == sp->first) {
						const_cast<Worker *>(*w.current)->remove(g);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			for ( ; d; --d)
				if (_new_worker(*pi, g, wdes))
						return report_error(L, "No space left for this worker");
		}
	}
	return 0;
}

int _Employer::get_workers(lua_State * L)
{
	PlayerImmovable * pi = get(L, get_game(L));
	Tribe_Descr const & tribe = pi->owner().tribe();

	bool return_number = false;
	WorkersSet set = m_parse_get_workers_arguments(L, tribe, &return_number);

	WorkersMap valid_workers = _valid_workers(*pi);

	WorkersMap c_workers;
	container_iterate_const(PlayerImmovable::Workers, pi->get_workers(), w) {
		Ware_Index i = tribe.worker_index((*w.current)->descr().name());
		if (not c_workers.count(i))
			c_workers.insert(WorkerAmount(i, 1));
		else
			c_workers[i] += 1;
	}

	if (set.size() == tribe.get_nrworkers().value()) { // Wants all returned
		set.clear();
		container_iterate(WorkersMap, valid_workers, i)
			set.insert(i.current->first);
	}

	if (not return_number)
		lua_newtable(L);

	container_iterate_const(WorkersSet, set, i) {
		uint32_t cnt = 0;
		if (c_workers.count(*i.current))
			cnt = c_workers[*i.current];

		if (return_number) {
			lua_pushuint32(L, cnt);
			break;
		} else {
			lua_pushstring(L, tribe.get_worker_descr(*i.current)->name());
			lua_pushuint32(L, cnt);
			lua_rawset(L, -3);
		}
	}
	return 1;
}

int _Employer::get_valid_workers(lua_State * L) {
	PlayerImmovable * pi = get(L, get_game(L));
	Tribe_Descr const & tribe = pi->owner().tribe();

	WorkersMap valid_workers = _valid_workers(*pi);

	lua_newtable(L);
	container_iterate_const(WorkersMap, valid_workers, i) {
		lua_pushstring
			(L, tribe.get_worker_descr(i.current->first)->name().c_str());
		lua_pushuint32(L, i.current->second);
		lua_rawset(L, -3);
	}
	return 1;
}

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

	HasWares is an interface that all :class:`PlayerImmovable` objects
	that can contain wares implement. This is at the time of this writing
	:class:`~wl.map.Flag`, :class:`~wl.map.Warehouse` and
	:class:`~wl.map.ProductionSite`.
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
			:const:`lumberjacks_hut` for the number of :const:`raw_stone` he has
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
		a ware name and an amount to set it too. Or it takes a table of
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
HasWorkers
----------

.. class:: HasWorkers

	Analogon to :class:`HasWares`, but for Workers. Supported at the time of
	this writing by :class:`~wl.map.Road`, :class:`~wl.map.Warehouse` and
	:class:`~wl.map.ProductionSite`.
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

/*
 ==========================================================
 C Methods
 ==========================================================
 */
// parses the get argument for all classes that can be asked for their
// current wares. Returns a set with all Ware_Indexes that must be considered.

#define GET_INDEX(type) \
	static Ware_Index _get_ ## type ## _index \
		(lua_State * L, Tribe_Descr const & tribe,  const std::string & what) \
	{ \
		Ware_Index idx = tribe. type ## _index(what); \
		if (!idx) \
			report_error(L, "Invalid " #type ": <%s>", what.c_str()); \
		return idx; \
	}
GET_INDEX(ware);
GET_INDEX(worker);
#undef GET_INDEX

#define PARSERS(type, btype) \
L_Has ##btype ## s:: btype ##sSet L_Has ## btype ##s \
	::m_parse_get_##type##s_arguments \
		(lua_State * L, Tribe_Descr const & tribe, bool * return_number) \
{ \
	 /* takes either "all", a name or an array of names */ \
	int32_t nargs = lua_gettop(L); \
	if (nargs != 2) \
		report_error(L, "Wrong number of arguments to get_" #type "!"); \
	*return_number = false; \
	btype ## sSet rv; \
	if (lua_isstring(L, 2)) { \
		std::string what = luaL_checkstring(L, -1); \
		if (what == "all") { \
			for (Ware_Index i = Ware_Index::First(); \
					i < tribe.get_nr##type##s (); ++i) \
				rv.insert(i); \
		} else { \
			/* Only one item requested */ \
			rv.insert(_get_##type##_index(L, tribe, what)); \
			*return_number = true; \
		} \
	} else { \
		/* array of names */ \
		luaL_checktype(L, 2, LUA_TTABLE); \
		lua_pushnil(L); \
		while (lua_next(L, 2) != 0) { \
			rv.insert(_get_##type##_index(L, tribe, luaL_checkstring(L, -1))); \
			lua_pop(L, 1); \
		} \
	} \
	return rv; \
} \
\
L_Has##btype##s::btype##sMap L_Has##btype##s:: \
	m_parse_set_##type##s_arguments \
	(lua_State * L, Tribe_Descr const & tribe) \
{ \
	int32_t nargs = lua_gettop(L); \
	if (nargs != 2 and nargs != 3) \
		report_error(L, "Wrong number of arguments to set_" #type "!"); \
   btype##sMap rv; \
	if (nargs == 3) { \
		/* name amount */ \
		rv.insert(btype##Amount( \
			_get_##type##_index(L, tribe, luaL_checkstring(L, 2)), \
			luaL_checkuint32(L, 3) \
		)); \
	} else { \
		/* array of (name, count) */ \
		lua_pushnil(L); \
		while (lua_next(L, 2) != 0) { \
			rv.insert(btype##Amount( \
				_get_##type##_index(L, tribe, luaL_checkstring(L, -2)), \
				luaL_checkuint32(L, -1) \
			)); \
			lua_pop(L, 1); \
		} \
	} \
	return rv; \
}
PARSERS(ware, Ware);
PARSERS(worker, Worker);
#undef PARSERS


// TODO: document this class
int L_HasSoldiers::m_get_soldier_levels
	(lua_State * L, int tidx, const Soldier_Descr & sd, SoldierDescr & rv)
{
	lua_pushuint32(L, 1);
	lua_rawget(L, tidx);
	rv.hp = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (rv.hp > sd.get_max_hp_level())
		return
			report_error
				(L, "hp level (%i) > max hp level (%i)",
				 rv.hp, sd.get_max_hp_level());

	lua_pushuint32(L, 2);
	lua_rawget(L, tidx);
	rv.at = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (rv.at > sd.get_max_attack_level())
		return
			report_error
			(L, "attack level (%i) > max attack level (%i)",
			 rv.at, sd.get_max_attack_level());

	lua_pushuint32(L, 3);
	lua_rawget(L, tidx);
	rv.de = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (rv.de > sd.get_max_defense_level())
		return
			report_error
			(L, "defense level (%i) > max defense level (%i)",
			 rv.de, sd.get_max_defense_level());

	lua_pushuint32(L, 4);
	lua_rawget(L, tidx);
	rv.ev = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (rv.ev > sd.get_max_evade_level())
		return
			report_error
			(L, "evade level (%i) > max evade level (%i)",
			 rv.ev, sd.get_max_evade_level());

	return 0;
}
L_HasSoldiers::SoldiersMap L_HasSoldiers::m_parse_set_soldiers_arguments
		(lua_State * L, Soldier_Descr const & soldier_descr)
{
	SoldiersMap rv;
	uint32_t count;

	if (lua_gettop(L) > 2) {
		// STACK: cls, descr, count
		count = luaL_checkuint32(L, 3);
		SoldierDescr d;
		m_get_soldier_levels(L, 2, soldier_descr, d);

		rv.insert(SoldierAmount(d, count));
	} else {
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			SoldierDescr d;
			m_get_soldier_levels(L, 3, soldier_descr, d);
			count = luaL_checkuint32(L, -1);
			rv.insert(SoldierAmount(d, count));
			lua_pop(L, 1);
		}
	}
	return rv;
}
int L_HasSoldiers::m_handle_get_soldiers
		(lua_State * L, Soldier_Descr const & soldier_descr,
		 SoldiersList const & soldiers)
{
	if (lua_gettop(L) != 2)
		return report_error(L, "Invalid arguments!");

	if (lua_isstring(L, -1)) {
		if (std::string(luaL_checkstring(L, -1)) != "all")
			return report_error(L, "Invalid arguments!");

		// Return All Soldiers
		SoldiersMap hist;
		container_iterate_const(SoldiersList, soldiers, s) {
			SoldierDescr sd
				((*s.current)->get_hp_level(),
				 (*s.current)->get_attack_level(),
				 (*s.current)->get_defense_level(),
				 (*s.current)->get_evade_level());

			SoldiersMap::iterator i = hist.find(sd);
			if (i == hist.end())
				hist[sd] = 1;
			else
				i->second += 1;
		}

		// Get this to lua
		lua_newtable(L);
		container_iterate_const(SoldiersMap, hist, i) {
			lua_createtable(L, 4, 0);
#define PUSHLEVEL(idx, name) \
			lua_pushuint32(L, idx); lua_pushuint32(L, i.current->first.name); \
			lua_rawset(L, -3);
			PUSHLEVEL(1, hp);
			PUSHLEVEL(2, at);
			PUSHLEVEL(3, de);
			PUSHLEVEL(4, ev);
#undef PUSHLEVEL

			lua_pushuint32(L, i.current->second);
			lua_rawset(L, -3);
		}
	} else {
		// Only return the number of those requested
		SoldierDescr wanted;
		m_get_soldier_levels(L, 2, soldier_descr, wanted);

		uint32_t rv = 0;
	  container_iterate_const(SoldiersList, soldiers, s) {
		  SoldierDescr sd
			  ((*s.current)->get_hp_level(), (*s.current)->get_attack_level(),
				(*s.current)->get_defense_level(), (*s.current)->get_evade_level());
		  if (sd == wanted)
			  ++ rv;
	  }
	  lua_pushuint32(L, rv);

	}

	return 1;
}

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
		report_error(L, "%s no longer exists!", name.c_str());
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
	BaseImmovable * o = get(L, game);

	switch (o->get_size()) {
		case BaseImmovable::NONE: lua_pushstring(L, "none"); break;
		case BaseImmovable::SMALL: lua_pushstring(L, "small"); break;
		case BaseImmovable::MEDIUM: lua_pushstring(L, "medium"); break;
		case BaseImmovable::BIG: lua_pushstring(L, "big"); break;
		default:
			return
				report_error
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
	BaseImmovable * o = get(L, game);

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
			(L, new L_Player (get(L, get_game(L))->get_owner()->player_number())
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

/* RST
Flag
--------

.. class:: Flag

	Child of: :class:`PlayerImmovable`, :class:`HasWares`

	One flag in the economy of this Player.
*/
const char L_Flag::className[] = "Flag";
const MethodType<L_Flag> L_Flag::Methods[] = {
	METHOD(L_Flag, set_wares),
	METHOD(L_Flag, get_wares),
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
static L_Flag::WaresMap _count_wares(Flag & f, Tribe_Descr const & tribe) {
	L_Flag::WaresMap rv;
	Flag::Wares current_wares = f.get_items();
	container_iterate_const(Flag::Wares, current_wares, w) {
		Ware_Index i = tribe.ware_index((*w.current)->descr().name());
		if (not rv.count(i))
			rv.insert(L_Flag::WareAmount(i, 1));
		else
			rv[i] += 1;
	}
	return rv;
}
// Documented in ParentClass
int L_Flag::set_wares(lua_State * L)
{
	Game & game = get_game(L);
	Flag * f = get(L, game);
	Tribe_Descr const & tribe = f->owner().tribe();

	WaresMap setpoints = m_parse_set_wares_arguments(L, tribe);
	WaresMap c_items = _count_wares(*f, tribe);

	uint32_t nitems = 0;
	container_iterate_const(WaresMap, c_items, c) {
		// all wares currently on the flag without a setpoint should be removed
		if (not setpoints.count(c->first))
			setpoints.insert(WareAmount(c->first, 0));
		nitems += c->second;
	}

	// The idea is to change as little as possible on this flag
	container_iterate_const(WaresMap, setpoints, sp) {
		uint32_t cur = 0;
		WaresMap::iterator i = c_items.find(sp->first);
		if (i != c_items.end())
			cur = i->second;

		int d = sp->second - cur;
		nitems += d;

		if (f->total_capacity() < nitems)
			return report_error(L, "Flag has no capacity left!");

		if (d < 0) {
			while (d) {
				Flag::Wares current_items = f->get_items();
				container_iterate_const(Flag::Wares, current_items, w) {
					Ware_Index i = tribe.ware_index((*w.current)->descr().name());
					if(i == sp->first) {
						const_cast<WareInstance *>(*w.current)->remove(game);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			// add items
			Item_Ware_Descr const & wd = *tribe.get_ware_descr(sp->first);
			for (int32_t i = 0; i < d; i++) {
				WareInstance & item = *new WareInstance(sp->first, &wd);
				item.init(game);
				f->add_item(game, item);
			}
		}

	}
	return 0;
}

// Documented in parent Class
int L_Flag::get_wares(lua_State * L) {
	Tribe_Descr const & tribe = get(L, get_game(L))->owner().tribe();

	bool return_number = false;
	WaresSet wares_set = m_parse_get_wares_arguments(L, tribe, &return_number);

	WaresMap items = _count_wares(*get(L, get_game(L)), tribe);

	if (wares_set.size() == tribe.get_nrwares().value()) { // Want all returned
		wares_set.clear();
		container_iterate_const(WaresMap, items, w)
			wares_set.insert(w->first);
	}

	if (not return_number)
		lua_newtable(L);

	container_iterate_const(WaresSet, wares_set, w) {
		uint32_t count = 0;
		if (items.count(*w))
			count = items[*w];

		if (return_number) {
			lua_pushuint32(L, count);
			break;
		} else {
		   lua_pushstring(L, tribe.get_ware_descr(*w)->name());
			lua_pushuint32(L, count);
			lua_rawset(L, -3);
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
--------

.. class:: Road

	Child of: :class:`PlayerImmovable`

	One flag in the economy of this Player.
*/
const char L_Road::className[] = "Road";
const MethodType<L_Road> L_Road::Methods[] = {
	METHOD(L_Road, get_workers),
	METHOD(L_Road, set_workers),
	{0, 0},
};
const PropertyType<L_Road> L_Road::Properties[] = {
	PROP_RO(L_Road, length),
	PROP_RO(L_Road, start_flag),
	PROP_RO(L_Road, end_flag),
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
	lua_pushuint32(L, get(L, get_game(L))->get_path().get_nsteps());
	return 1;
}

/* RST
	.. attribute:: start_flag

		(RO) The flag were this road starts
*/
int L_Road::get_start_flag(lua_State * L) {
	return
		to_lua<L_Flag>
			(L, new L_Flag(get(L, get_game(L))->get_flag(Road::FlagStart)));
}

/* RST
	.. attribute:: end_flag

		(RO) The flag were this road ends
*/
int L_Road::get_end_flag(lua_State * L) {
	return
		to_lua<L_Flag>
			(L, new L_Flag(get(L, get_game(L))->get_flag(Road::FlagEnd)));
}

/* RST
	.. attribute:: type

		(RO) Type of road. Can be any either of:

		* normal
		* busy
*/
int L_Road::get_type(lua_State * L) {
	switch (get(L, get_game(L))->get_roadtype()) {
		case Road_Normal:
			lua_pushstring(L, "normal"); break;
		case Road_Busy:
			lua_pushstring(L, "busy"); break;
		default:
			return
				report_error(L, "Unknown Roadtype! This is a bug in widelands!");
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// This is for get_/set_ workers which is implemented in _Employer
L_HasWorkers::WorkersMap L_Road::_valid_workers
		(PlayerImmovable & pi)
{
	WorkersMap valid_workers;
	valid_workers.insert
		(WorkerAmount(pi.owner().tribe().worker_index("carrier"), 1));
	return valid_workers;
}
int L_Road::_new_worker
	(PlayerImmovable & pi, Game & g, const Worker_Descr * wdes)
{
	Road & r = static_cast<Road &>(pi);

	if (r.get_workers().size())
		return -1; // No space

	// Determine Idle position.
	Flag & start = r.get_flag(Road::FlagStart);
	Coords idle_position = start.get_position();
	const Path & path = r.get_path();
	Path::Step_Vector::size_type idle_index = r.get_idle_index();
	for (Path::Step_Vector::size_type i = 0; i < idle_index; ++i)
		g.map().get_neighbour(idle_position, path[i], &idle_position);

	Carrier & carrier = ref_cast<Carrier, Worker>
		(wdes->create (g, r.owner(), &r, idle_position));
	carrier.start_task_road(g);

	r.assign_carrier(carrier, 0);
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
	lua_pushstring(L, get(L, get_game(L))->type_name());
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

	Child of: :class:`Building`, :class:`HasWares`

	Every Headquarter or Warehouse on the Map is of this type.
*/
const char L_Warehouse::className[] = "Warehouse";
const MethodType<L_Warehouse> L_Warehouse::Methods[] = {
	METHOD(L_Warehouse, set_wares),
	METHOD(L_Warehouse, get_wares),
	METHOD(L_Warehouse, set_workers),
	METHOD(L_Warehouse, get_workers),
	METHOD(L_Warehouse, set_soldiers),
	METHOD(L_Warehouse, get_soldiers),
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
// Documented in parent class
#define WH_SET(type, btype) \
int L_Warehouse::set_##type##s(lua_State * L) { \
	Warehouse * wh = get(L, get_game(L)); \
	Tribe_Descr const & tribe = wh->owner().tribe(); \
	btype##sMap setpoints = m_parse_set_##type##s_arguments(L, tribe); \
 \
	container_iterate_const(btype##sMap, setpoints, i) { \
		int32_t d = i.current->second - wh->get_##type##s().stock(i.current->first); \
		if (d < 0) \
			wh->remove_##type##s(i.current->first, -d); \
		else if (d > 0) \
			wh->insert_##type##s(i.current->first, d); \
	} \
	return 0; \
}
// documented in parent class
WH_SET(ware, Ware);
// documented in parent class
WH_SET(worker, Worker);
#undef WH_SET

#define WH_GET(type, btype) \
int L_Warehouse::get_##type##s(lua_State * L) { \
	Warehouse * wh = get(L, get_game(L)); \
	Tribe_Descr const & tribe = wh->owner().tribe(); \
	bool return_number = false; \
	btype##sSet set = m_parse_get_##type##s_arguments(L, tribe, &return_number); \
	lua_newtable(L); \
	if (return_number) \
		lua_pushuint32(L, wh->get_##type##s().stock(*set.begin())); \
	else { \
		lua_newtable(L); \
		container_iterate_const(btype##sSet, set, i) { \
			lua_pushstring(L, tribe.get_##type##_descr(*i.current)->name()); \
			lua_pushuint32(L, wh->get_##type##s().stock(*i.current)); \
			lua_rawset(L, -3); \
		} \
	} \
	return 1; \
}
// documented in parent class
WH_GET(ware, Ware);
// documented in parent class
WH_GET(worker, Worker);
#undef GET

/* RST
 * TODO: docu should not be here.
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
int L_Warehouse::set_soldiers(lua_State * L) {
	Game & game = get_game(L);
	Warehouse * wh = get(L, game);
	Tribe_Descr const & tribe = wh->owner().tribe();

	Soldier_Descr const & soldier_descr =  //  soldiers
		ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));

	SoldiersMap setpoints = m_parse_set_soldiers_arguments(L, soldier_descr);

	container_iterate_const(SoldiersMap, setpoints, s) {
		for (uint32_t i = 0; i < s.current->second; i++) {
			Soldier & soldier =
				ref_cast<Soldier, Worker>
				(soldier_descr.create(game, wh->owner(), wh, wh->get_position()));
			soldier.set_level
				(s.current->first.hp, s.current->first.at,
				 s.current->first.de, s.current->first.ev);
			wh->incorporate_worker(game, soldier);
		}
	}
	return 0;
}

// document in base class
int L_Warehouse::get_soldiers(lua_State * L) {
	Game & game = get_game(L);
	Warehouse * wh = get(L, game);
	Tribe_Descr const & tribe = wh->owner().tribe();

	Soldier_Descr const & soldier_descr =  //  soldiers
			 ref_cast<Soldier_Descr const, Worker_Descr const>
						(*tribe.get_worker_descr(tribe.worker_index("soldier")));

	SoldiersList in_warehouse = wh->get_soldiers(game);
	return m_handle_get_soldiers(L, soldier_descr, in_warehouse);

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

	Child of: :class:`Building`, :class:`HasWares`

	Every building that produces anything.
*/
const char L_ProductionSite::className[] = "ProductionSite";
const MethodType<L_ProductionSite> L_ProductionSite::Methods[] = {
	METHOD(L_ProductionSite, set_wares),
	METHOD(L_ProductionSite, get_wares),
	METHOD(L_ProductionSite, get_workers),
	METHOD(L_ProductionSite, set_workers),
	{0, 0},
};
const PropertyType<L_ProductionSite> L_ProductionSite::Properties[] = {
	PROP_RO(L_ProductionSite, valid_workers),
	PROP_RO(L_ProductionSite, valid_wares),
	{0, 0, 0},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// documented in parent class
int L_ProductionSite::get_valid_wares(lua_State * L) {
	Game & g = get_game(L);
	ProductionSite * ps = get(L, g);

	Tribe_Descr const & tribe = ps->owner().tribe();

	lua_newtable(L);
	container_iterate_const(Ware_Types, ps->descr().inputs(), i) {
		lua_pushstring(L, tribe.get_ware_descr(i.current->first)->name());
		lua_pushuint32(L, i.current->second);
		lua_rawset(L, -3);
	}
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// This is for get_/set_ workers which is implemented in _Employer
L_HasWorkers::WorkersMap L_ProductionSite::_valid_workers
		(PlayerImmovable & pi)
{
	ProductionSite & ps = static_cast<ProductionSite &>(pi);
	WorkersMap rv;
	container_iterate_const(Ware_Types, ps.descr().working_positions(), i)
			rv.insert(WorkerAmount(i.current->first, i.current->second));
	return rv;
}
int L_ProductionSite::_new_worker
	(PlayerImmovable & pi, Game & g, const Worker_Descr * wdes)
{
	ProductionSite & ps = static_cast<ProductionSite &>(pi);
	return ps.warp_worker(g, *wdes);
}

// documented in parent class
int L_ProductionSite::set_wares(lua_State * L) {
	ProductionSite * ps = get(L, get_game(L));
	const Tribe_Descr & tribe = ps->owner().tribe();

	WaresMap setpoints = m_parse_set_wares_arguments(L, tribe);

	WaresSet valid_wares;
	container_iterate_const(Ware_Types, ps->descr().inputs(), i)
		valid_wares.insert(i->first);

	container_iterate_const(WaresMap, setpoints, i) {
		if (not valid_wares.count(i->first))
			return
				report_error
				 (L, "<%s> can't be stored here!",
				  tribe.get_ware_descr(i->first)->name().c_str());

		WaresQueue & wq = ps->waresqueue(i->first);
		if (i->second > wq.get_size())
			return
				report_error
					(L, "Not enough space for %u items, only for %i",
					 i->second, wq.get_size());

		wq.set_filled(i->second);
		wq.update();
	}

	return 0;
}

// documented in parent class
int L_ProductionSite::get_wares(lua_State * L) {
	ProductionSite * ps = get(L, get_game(L));
	Tribe_Descr const & tribe = ps->owner().tribe();

	bool return_number = false;
	WaresSet wares_set = m_parse_get_wares_arguments(L, tribe, &return_number);

	WaresSet valid_wares;
	container_iterate_const(Ware_Types, ps->descr().inputs(), i)
		valid_wares.insert(i.current->first);

	if (wares_set.size() == tribe.get_nrwares().value()) // Want all returned
		wares_set = valid_wares;

	if (not return_number)
		lua_newtable(L);

	container_iterate_const(WaresSet, wares_set, i) {
		uint32_t cnt = 0;
		if (valid_wares.count(*i.current))
			cnt = ps->waresqueue(*i.current).get_filled();

		if (return_number) { // this is the only thing the customer wants to know
			lua_pushuint32(L, cnt);
			break;
		} else {
			lua_pushstring(L, tribe.get_ware_descr(*i.current)->name());
			lua_pushuint32(L, cnt);
			lua_rawset(L, -3);
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
MilitarySite
--------------

.. class:: MilitarySite

	Child of: :class:`Building`

	Miltary Buildings
*/
const char L_MilitarySite::className[] = "MilitarySite";
const MethodType<L_MilitarySite> L_MilitarySite::Methods[] = {
	METHOD(L_MilitarySite, warp_soldiers),
	METHOD(L_MilitarySite, get_soldiers),
	{0, 0},
};
const PropertyType<L_MilitarySite> L_MilitarySite::Properties[] = {
	PROP_RO(L_MilitarySite, max_soldiers),
	{0, 0, 0},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: max_soldiers

		(RO) The maximum number of soldiers that can be inside this building
			at one time.
*/
int L_MilitarySite::get_max_soldiers(lua_State * L) {
	lua_pushuint32
		(L, get(L, get_game(L))->descr().get_max_number_of_soldiers());

	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
/* RST
	.. method:: warp_soldiers(list)

		The equivalent to :meth:`~wl.map.HasWorkers.set_workers`.
		Creates soldiers out of thin air. ``list`` is a table of soldier
		descriptions and counts, see :meth:`wl.map.Warehouse.set_soldiers`
		for a description.
*/
// TODO: fix me
int L_MilitarySite::set_soldiers(lua_State * L) {
	return 0;
}
int L_MilitarySite::warp_soldiers(lua_State * L) {
	luaL_checktype(L, 2, LUA_TTABLE);

	// Check that it is not called int the wrong way: warp_soldiers({0,0,0,0}, 1)
	lua_pushuint32(L, 1);
	lua_rawget(L, 2);
	if (lua_isnumber(L, -1))
		return report_error
			(L, "Expects an array of (Soldier_descr,count) pairs!");
	lua_pop(L, 1);

	Game & game = get_game(L);
	MilitarySite * ms = get(L, game);
	Tribe_Descr const & tribe = ms->owner().tribe();

	Soldier_Descr const & soldier_descr =  //  soldiers
		ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));

	lua_pushnil(L);
	while (lua_next(L, 2) != 0) {
		SoldierDescr de;
		m_get_soldier_levels(L, 3, soldier_descr, de);
		uint32_t count = luaL_checkuint32(L, -1);

		for (uint32_t j = count; j; --j) {
			Soldier & soldier =
				ref_cast<Soldier, Worker>
				(soldier_descr.create
				 (game, ms->owner(), 0, ms->get_position()));
			soldier.set_level(de.hp, de.at, de.de, de.ev);

			if (ms->add_soldier(game, soldier)) {
				return report_error(L, "No space left for soldier!");
				soldier.remove(game);
			}
		}

		lua_pop(L, 1);
	}
	return 0;
}

/* RST
	.. method:: get_soldiers([descr])

		Returns the number of soldiers of the given type in this building. See
		:meth:`warp_soldiers` for a description of the description of soldiers.
		If descr is not given or :const:`nil` the total number of soldiers is
		returned.

		:returns: Number of soldiers that match descr
		:rtype: :class:`integer`
*/
int L_MilitarySite::get_soldiers(lua_State * L) {
	Game & game = get_game(L);
	MilitarySite * ms = get(L, game);
	std::vector<Soldier *> vec = ms->stationedSoldiers();

	if (lua_gettop(L) == 1 or lua_isnil(L, 2)) {
		lua_pushuint32(L, vec.size());
		return 1;
	}

	Tribe_Descr const & tribe = ms->owner().tribe();

	Soldier_Descr const & soldier_descr =  //  soldiers
		ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));

	uint32_t count = 0;
	SoldierDescr de;
	m_get_soldier_levels(L, 2, soldier_descr, de);
#define CHECK(type, val) \
		((*i.current)->get_ ##type ## _level() == de.val)
	container_iterate_const(std::vector<Soldier *>, vec, i)
		if
			(CHECK(hp, hp) and CHECK(attack, at) and
			 CHECK(defense, de) and CHECK(evade, ev))
				++count;
#undef CHECK

	lua_pushuint32(L, count);

	return 1;
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
	PROP_RO(L_Field, owners),
	{0, 0, 0},
};


L_Field::L_Field(lua_State * L) {
	Map & m = get_game(L).map();
	uint32_t rv = luaL_checkuint32(L, 1);
	if (rv >= static_cast<uint32_t>(m.get_width()))
		report_error(L, "x coordinate out of range!");
	m_c.x = rv;
	rv = luaL_checkuint32(L, 2);
	if (rv >= static_cast<uint32_t>(m.get_height()))
		report_error(L, "y coordinate out of range!");
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
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);

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
		return report_error(L, "Illegal resource: '%s'", luaL_checkstring(L, -1));

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
			report_error
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
		report_error(L, "Unknown terrain '%s'", name);

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
		report_error(L, "Unknown terrain '%s'", name);

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

/* RST
	.. attribute:: owners

		(RO) An :class:`array` of owners of this field sorted by their military
		influence. That is owners[1] will really own the fields. This can
		also return an empty list if the field is neutral at the moment.
*/
// UNTESTED
typedef std::pair<uint8_t, uint32_t> _PlrInfluence;
static int _sort_owners
		(const _PlrInfluence & first,
		 const _PlrInfluence & second)
{
	return first.second > second.second;
}
int L_Field::get_owners(lua_State * L) {
	Game & game = get_game(L);
	Map & map = game.map();

	std::vector<_PlrInfluence> owners;

	iterate_players_existing(other_p, map.get_nrplayers(), game, plr)
		owners.push_back
			(_PlrInfluence(plr->player_number(), plr->military_influence
					(map.get_index(m_c, map.get_width()))
			)
		);

	std::sort (owners.begin(), owners.end(), _sort_owners);

	lua_newtable(L);
	uint32_t cidx = 1;
	container_iterate_const (std::vector<_PlrInfluence>, owners, i) {
		if (i.current->second <= 0)
			continue;

		lua_pushuint32(L, cidx ++);
		to_lua<L_Player>
			(L, new L_Player (i.current->first));
		lua_rawset(L, -3);
	}

	return 1;
}

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
.. function:: create_immovable(name, field[, from_where = "world"])

	Creates an immovable that is defined by the world (e.g. trees, stones...) or
	a tribe (field) on a given field. If there is already an immovable on the
	field, an error is reported.

	:arg name: The name of the immovable to create
	:type name: :class:`string`
	:arg field: The immovable is created on this field.
	:type field: :class:`wl.map.Field`
	:arg from_where: a tribe name or "world" that defines where the immovable
		is defined
	:type from_where: :class:`string`

	:returns: The created immovable object, most likely a
		:class:`wl.map.BaseImmovable`
*/
static int L_create_immovable(lua_State * const L) {
	std::string from_where = "world";

	char const * const objname = luaL_checkstring(L, 1);
	L_Field * c = *get_user_class<L_Field>(L, 2);
	if (lua_gettop(L) > 2 and not lua_isnil(L, 3))
		from_where = luaL_checkstring(L, 3);

	// Check if the map is still free here
	if
	 (BaseImmovable const * const imm = c->fcoords(L).field->get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(L, "Node is no longer free!");

	Game & game = get_game(L);

	BaseImmovable * m = 0;
	if (from_where != "world") {
		try {
			Widelands::Tribe_Descr const & tribe =
				game.manually_load_tribe(from_where);

			int32_t const imm_idx = tribe.get_immovable_index(objname);
			if (imm_idx < 0)
				return
					report_error
					(L, "Unknown immovable <%s> for tribe <%s>",
					 objname, from_where.c_str());

			m = &game.create_immovable(c->coords(), imm_idx, &tribe);
		} catch (game_data_error & gd) {
			return
				report_error
					(L, "Problem loading tribe <%s>. Maybe not existent?",
					 from_where.c_str());
		}
	} else {
		int32_t const imm_idx = game.map().world().get_immovable_index(objname);
		if (imm_idx < 0)
			return report_error(L, "Unknown immovable <%s>", objname);

		m = &game.create_immovable(c->coords(), imm_idx, 0);
	}

	return upcasted_immovable_to_lua(L, m);
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

	register_class<L_MilitarySite>(L, "map", true);
	add_parent<L_MilitarySite, L_Building>(L);
	add_parent<L_MilitarySite, L_PlayerImmovable>(L);
	add_parent<L_MilitarySite, L_BaseImmovable>(L);
	add_parent<L_MilitarySite, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table
}

