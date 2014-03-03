/*
 * Copyright (C) 2006-2010, 2013 by the Widelands Development Team
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

#include <boost/foreach.hpp>

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
#include "scripting/c_utils.h"
#include "scripting/lua_game.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

namespace LuaMap {

/* RST
:mod:`wl.map`
=============

.. module:: wl.map
   :synopsis: Provides access to Fields and Objects on the map

.. moduleauthor:: The Widelands development team

.. currentmodule:: wl.map

*/

namespace {

struct SoldierDescr {
	SoldierDescr(uint8_t ghp, uint8_t gat, uint8_t gde, uint8_t gev)
	   : hp(ghp), at(gat), de(gde), ev(gev) {
	}
	SoldierDescr() : hp(0), at(0), de(0), ev(0) {
	}

	uint8_t hp;
	uint8_t at;
	uint8_t de;
	uint8_t ev;

	bool operator<(const SoldierDescr& ot) const {
		bool hp_eq = hp == ot.hp;
		bool at_eq = at == ot.at;
		bool de_eq = de == ot.de;
		if (hp_eq && at_eq && de_eq)
			return ev < ot.ev;
		if (hp_eq && at_eq)
			return de < ot.de;
		if (hp_eq)
			return at < ot.at;
		return hp < ot.hp;
	}
	bool operator == (const SoldierDescr& ot) const {
		if (hp == ot.hp && at == ot.at && de == ot.de && ev == ot.ev)
			return true;
		return false;
	}
};

typedef std::map<SoldierDescr, uint32_t> SoldiersMap;
typedef std::map<Widelands::Ware_Index, uint32_t> WaresMap;
typedef std::map<Widelands::Ware_Index, uint32_t> WorkersMap;
typedef std::pair<SoldierDescr, uint32_t> SoldierAmount;
typedef std::pair<Widelands::Ware_Index, uint32_t> WorkerAmount;
typedef std::pair<uint8_t, uint32_t> PlrInfluence;
typedef std::set<Widelands::Ware_Index> WaresSet;
typedef std::set<Widelands::Ware_Index> WorkersSet;
typedef std::vector<Widelands::Soldier *> SoldiersList;

// parses the get argument for all classes that can be asked for their
// current wares. Returns a set with all Ware_Indexes that must be considered.
#define GET_INDEX(type) \
	Ware_Index m_get_ ## type ## _index \
		(lua_State * L, const Tribe_Descr & tribe,  const std::string & what) \
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
btype ##sSet m_parse_get_##type##s_arguments \
		(lua_State * L, const Tribe_Descr & tribe, bool * return_number) \
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
			rv.insert(m_get_##type##_index(L, tribe, what)); \
			*return_number = true; \
		} \
	} else { \
		/* array of names */ \
		luaL_checktype(L, 2, LUA_TTABLE); \
		lua_pushnil(L); \
		while (lua_next(L, 2) != 0) { \
			rv.insert(m_get_##type##_index(L, tribe, luaL_checkstring(L, -1))); \
			lua_pop(L, 1); \
		} \
	} \
	return rv; \
} \
\
btype##sMap m_parse_set_##type##s_arguments \
	(lua_State * L, const Tribe_Descr & tribe) \
{ \
	int32_t nargs = lua_gettop(L); \
	if (nargs != 2 and nargs != 3) \
		report_error(L, "Wrong number of arguments to set_" #type "!"); \
   btype##sMap rv; \
	if (nargs == 3) { \
		/* name amount */ \
		rv.insert(btype##Amount( \
			m_get_##type##_index(L, tribe, luaL_checkstring(L, 2)), \
			luaL_checkuint32(L, 3) \
		)); \
	} else { \
		/* array of (name, count) */ \
		luaL_checktype(L, 2, LUA_TTABLE); \
		lua_pushnil(L); \
		while (lua_next(L, 2) != 0) { \
			rv.insert(btype##Amount( \
				m_get_##type##_index(L, tribe, luaL_checkstring(L, -2)), \
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

WaresMap count_wares_on_flag_(Flag& f, const Tribe_Descr & tribe) {
	WaresMap rv;
	Flag::Wares current_wares = f.get_wares();
	container_iterate_const(Flag::Wares, current_wares, w) {
		Ware_Index i = tribe.ware_index((*w.current)->descr().name());
		if (!rv.count(i))
			rv.insert(Widelands::WareAmount(i, 1));
		else
			rv[i] += 1;
	}
	return rv;
}

// Sort functor to sort the owners claiming a field by their influence.
static int sort_claimers(const PlrInfluence& first, const PlrInfluence& second) {
	return first.second > second.second;
}

// Return the valid workers for a Road.
WorkersMap get_valid_workers_for(const Road& r) {
	WorkersMap valid_workers;
	valid_workers.insert(WorkerAmount(r.owner().tribe().worker_index("carrier"), 1));

	if (r.get_roadtype() == Road_Busy)
		valid_workers.insert(WorkerAmount(r.owner().tribe().carrier2(), 1));

	return valid_workers;
}

// Returns the valid workers allowed in 'pi'.
WorkersMap get_valid_workers_for(const ProductionSite& ps)
{
	WorkersMap rv;
	BOOST_FOREACH(const Widelands::WareAmount& item, ps.descr().working_positions()) {
		rv.insert(WorkerAmount(item.first, item.second));
	}
	return rv;
}

// Translate the given Workers map into a (string, count) Lua table.
int workers_map_to_lua(lua_State * L, const Tribe_Descr& tribe, const WorkersMap& valid_workers) {
	lua_newtable(L);
	BOOST_FOREACH(const WorkersMap::value_type& item, valid_workers) {
		lua_pushstring(L, tribe.get_worker_descr(item.first)->name());
		lua_pushuint32(L, item.second);
		lua_rawset(L, -3);
	}
	return 1;
}

// Does most of the work of get_workers for player immovables (buildings and roads mainly).
int do_get_workers(lua_State* L, const PlayerImmovable& pi, const WorkersMap& valid_workers) {
	const Tribe_Descr& tribe = pi.owner().tribe();

	bool return_number = false;
	WorkersSet set = m_parse_get_workers_arguments(L, tribe, &return_number);

	WorkersMap c_workers;
	BOOST_FOREACH(const Worker* w, pi.get_workers()) {
		Ware_Index i = tribe.worker_index(w->descr().name());
		if (!c_workers.count(i)) {
			c_workers.insert(WorkerAmount(i, 1));
		} else {
			c_workers[i] += 1;
		}
	}

	if (set.size() == tribe.get_nrworkers().value()) {  // Wants all returned
		set.clear();
		BOOST_FOREACH(const WorkersMap::value_type& v, valid_workers) {
			set.insert(v.first);
		}
	}

	if (!return_number)
		lua_newtable(L);

	BOOST_FOREACH(const Ware_Index& i, set) {
		uint32_t cnt = 0;
		if (c_workers.count(i))
			cnt = c_workers[i];

		if (return_number) {
			lua_pushuint32(L, cnt);
			break;
		} else {
			lua_pushstring(L, tribe.get_worker_descr(i)->name());
			lua_pushuint32(L, cnt);
			lua_rawset(L, -3);
		}
	}
	return 1;
}

// Does most of the work of set_workers for player immovables (buildings and roads mainly).
template <typename T>
int do_set_workers(lua_State* L, PlayerImmovable* pi, const WorkersMap& valid_workers) {
	const Tribe_Descr& tribe = pi->owner().tribe();

	WorkersMap setpoints = m_parse_set_workers_arguments(L, tribe);

	WorkersMap c_workers;
	BOOST_FOREACH(const Worker* w, pi->get_workers()) {
		Ware_Index i = tribe.worker_index(w->descr().name());
		if (!c_workers.count(i))
			c_workers.insert(WorkerAmount(i, 1));
		else
			c_workers[i] += 1;
		if (!setpoints.count(i))
			setpoints.insert(WorkerAmount(i, 0));
	}

	// The idea is to change as little as possible
	Editor_Game_Base& egbase = get_egbase(L);
	BOOST_FOREACH(const WorkersMap::value_type sp, setpoints) {
		const Worker_Descr* wdes = tribe.get_worker_descr(sp.first);
		if (!valid_workers.count(sp.first))
			report_error(L, "<%s> can't be employed here!", wdes->name().c_str());

		uint32_t cur = 0;
		WorkersMap::iterator i = c_workers.find(sp.first);
		if (i != c_workers.end())
			cur = i->second;

		int d = sp.second - cur;
		if (d < 0) {
			while (d) {
				BOOST_FOREACH(const Worker* w, pi->get_workers()) {
					if (tribe.worker_index(w->descr().name()) == sp.first) {
						const_cast<Worker*>(w)->remove(egbase);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			for (; d; --d)
				if (T::create_new_worker(*pi, egbase, wdes))
					return report_error(L, "No space left for this worker");
		}
	}
	return 0;
}

// Unpacks the Lua table of the form {hp, at, de, ev} at the stack index
// 'table_index' into a SoldierDescr struct.
SoldierDescr unbox_lua_soldier_description(lua_State* L, int table_index, const Soldier_Descr& sd) {
	SoldierDescr soldier_descr;

	lua_pushuint32(L, 1);
	lua_rawget(L, table_index);
	soldier_descr.hp = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.hp > sd.get_max_hp_level())
		report_error
			(L, "hp level (%i) > max hp level (%i)", soldier_descr.hp, sd.get_max_hp_level());

	lua_pushuint32(L, 2);
	lua_rawget(L, table_index);
	soldier_descr.at = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.at > sd.get_max_attack_level())
		report_error
			(L, "attack level (%i) > max attack level (%i)", soldier_descr.at, sd.get_max_attack_level());

	lua_pushuint32(L, 3);
	lua_rawget(L, table_index);
	soldier_descr.de = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.de > sd.get_max_defense_level())
		report_error
			(L, "defense level (%i) > max defense level (%i)", soldier_descr.de,
			 sd.get_max_defense_level());

	lua_pushuint32(L, 4);
	lua_rawget(L, table_index);
	soldier_descr.ev = luaL_checkuint32(L, -1);
	lua_pop(L, 1);
	if (soldier_descr.ev > sd.get_max_evade_level())
		report_error
			(L, "evade level (%i) > max evade level (%i)", soldier_descr.ev, sd.get_max_evade_level());

	return soldier_descr;
}

// Parser the arguments of set_soldiers() into a setpoint. See the
// documentation in HasSoldiers to understand the valid arguments.
SoldiersMap m_parse_set_soldiers_arguments(lua_State* L, const Soldier_Descr& soldier_descr) {
	SoldiersMap rv;
	if (lua_gettop(L) > 2) {
		// STACK: cls, descr, count
		const uint32_t count = luaL_checkuint32(L, 3);
		const SoldierDescr d = unbox_lua_soldier_description(L, 2, soldier_descr);
		rv.insert(SoldierAmount(d, count));
	} else {
		lua_pushnil(L);
		while (lua_next(L, 2) != 0) {
			const SoldierDescr d = unbox_lua_soldier_description(L, 3, soldier_descr);
			const uint32_t count = luaL_checkuint32(L, -1);
			rv.insert(SoldierAmount(d, count));
			lua_pop(L, 1);
		}
	}
	return rv;
}

// Does most of the work of get_soldiers for buildings.
int do_get_soldiers(lua_State* L, const Widelands::SoldierControl& sc, const Tribe_Descr& tribe) {
	if (lua_gettop(L) != 2)
		return report_error(L, "Invalid arguments!");

	const SoldiersList soldiers = sc.stationedSoldiers();
	if (lua_isstring(L, -1)) {
		if (std::string(luaL_checkstring(L, -1)) != "all")
			return report_error(L, "Invalid arguments!");

		// Return All Soldiers
		SoldiersMap hist;
		BOOST_FOREACH(const Soldier* s, soldiers) {
			SoldierDescr sd
				(s->get_hp_level(), s->get_attack_level(),
				 s->get_defense_level(), s->get_evade_level());

			SoldiersMap::iterator i = hist.find(sd);
			if (i == hist.end())
				hist[sd] = 1;
			else
				i->second += 1;
		}

		// Get this to Lua.
		lua_newtable(L);
		BOOST_FOREACH(const SoldiersMap::value_type& i, hist) {
			lua_createtable(L, 4, 0);
#define PUSHLEVEL(idx, name)                                                                       \
	lua_pushuint32(L, idx);                                                                         \
	lua_pushuint32(L, i.first.name);                                                                \
	lua_rawset(L, -3);
			PUSHLEVEL(1, hp);
			PUSHLEVEL(2, at);
			PUSHLEVEL(3, de);
			PUSHLEVEL(4, ev);
#undef PUSHLEVEL

			lua_pushuint32(L, i.second);
			lua_rawset(L, -3);
		}
	} else {
		const Soldier_Descr& soldier_descr = ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));

		// Only return the number of those requested
		const SoldierDescr wanted = unbox_lua_soldier_description(L, 2, soldier_descr);
		uint32_t rv = 0;
		BOOST_FOREACH(const Soldier* s, soldiers) {
			SoldierDescr sd
				(s->get_hp_level(), s->get_attack_level(), s->get_defense_level(), s->get_evade_level());
			if (sd == wanted)
				++rv;
		}
		lua_pushuint32(L, rv);
	}
	return 1;
}

// Does most of the work of set_soldiers for buildings.
int do_set_soldiers
	(lua_State* L, const Coords& building_position, SoldierControl* sc, Player* owner)
{
	assert(sc != nullptr);
	assert(owner != nullptr);

	const Tribe_Descr& tribe = owner->tribe();
	const Soldier_Descr& soldier_descr =  //  soldiers
	   ref_cast<Soldier_Descr const, Worker_Descr const>
			(*tribe.get_worker_descr(tribe.worker_index("soldier")));
	SoldiersMap setpoints = m_parse_set_soldiers_arguments(L, soldier_descr);

	// Get information about current soldiers
	const std::vector<Soldier*> curs = sc->stationedSoldiers();
	SoldiersMap hist;
	BOOST_FOREACH(const Soldier* s, curs) {
		SoldierDescr sd
			(s->get_hp_level(), s->get_attack_level(),
			 s->get_defense_level(), s->get_evade_level());

		SoldiersMap::iterator i = hist.find(sd);
		if (i == hist.end())
			hist[sd] = 1;
		else
			i->second += 1;
		if (!setpoints.count(sd))
			setpoints[sd] = 0;
	}

	// Now adjust them
	Editor_Game_Base& egbase = get_egbase(L);
	BOOST_FOREACH(const SoldiersMap::value_type& sp, setpoints) {
		uint32_t cur = 0;
		SoldiersMap::iterator i = hist.find(sp.first);
		if (i != hist.end())
			cur = i->second;

		int d = sp.second - cur;
		if (d < 0) {
			while (d) {
				BOOST_FOREACH(Soldier * s, sc->stationedSoldiers()) {
					SoldierDescr is
						(s->get_hp_level(), s->get_attack_level(),
						 s->get_defense_level(), s->get_evade_level());

					if (is == sp.first) {
						sc->outcorporateSoldier(egbase, *s);
						s->remove(egbase);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			for (; d; --d) {
				Soldier& soldier = ref_cast<Soldier, Worker>
					(soldier_descr.create(egbase, *owner, nullptr, building_position));
				soldier.set_level
					(sp.first.hp, sp.first.at, sp.first.de, sp.first.ev);
				if (sc->incorporateSoldier(egbase, soldier)) {
					soldier.remove(egbase);
					return report_error(L, "No space left for soldier!");
				}
			}
		}
	}
	return 0;
}
}  // namespace


/*
 * Upcast the given base immovable to a higher type and hand this to
 * Lua. We use this so that scripters always work with the highest class
 * object available.
 */
#define CAST_TO_LUA(k) to_lua<L_ ##k> \
   (L, new L_ ##k(*static_cast<k *>(mo)))
int upcasted_bob_to_lua(lua_State * L, Bob * mo) {
	if (!mo)
		return 0;

	switch (mo->get_bob_type()) {
		case Bob::CRITTER:
			return to_lua<L_Bob>(L, new L_Bob(*mo));
		case Bob::WORKER:
			if (mo->name() == "soldier")
				return CAST_TO_LUA(Soldier);
			return CAST_TO_LUA(Worker);
		case Bob::SHIP:
			return CAST_TO_LUA(Ship);
	}
	assert(false);  // Never here, hopefully.
}

int upcasted_immovable_to_lua(lua_State * L, BaseImmovable * mo) {
	if (!mo)
		return 0;

	switch  (mo->get_type()) {
		case Map_Object::BUILDING:
		{
			const char * type_name = mo->type_name();
			if (!strcmp(type_name, "constructionsite"))
				return CAST_TO_LUA(ConstructionSite);
			else if (!strcmp(type_name, "productionsite"))
				return CAST_TO_LUA(ProductionSite);
			else if (!strcmp(type_name, "militarysite"))
				return CAST_TO_LUA(MilitarySite);
			else if (!strcmp(type_name, "warehouse"))
				return CAST_TO_LUA(Warehouse);
			else if (!strcmp(type_name, "trainingsite"))
				return CAST_TO_LUA(TrainingSite);
			else
				return CAST_TO_LUA(Building);
		}

		case Map_Object::FLAG:
			return CAST_TO_LUA(Flag);
		case Map_Object::ROAD:
			return CAST_TO_LUA(Road);
		case Map_Object::PORTDOCK:
			return CAST_TO_LUA(PortDock);
		default:
			break;
	}
	return to_lua<L_BaseImmovable>(L, new L_BaseImmovable(*mo));
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
			hitpoints, attack, defense and evade (in this order). An usage example:

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

			l:set_soldiers{
			  [{0,0,0,0}] = 10,
			  [{1,2,3,4}] = 5,
			)

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

	Access to the map and it's objects. You cannot instantiate this directly,
	instead access it via :attr:`wl.Game.map`.
*/
const char L_Map::className[] = "Map";
const MethodType<L_Map> L_Map::Methods[] = {
	METHOD(L_Map, place_immovable),
	METHOD(L_Map, get_field),
	METHOD(L_Map, recalculate),
	{nullptr, nullptr},
};
const PropertyType<L_Map> L_Map::Properties[] = {
	PROP_RO(L_Map, width),
	PROP_RO(L_Map, height),
	PROP_RO(L_Map, player_slots),
	{nullptr, nullptr, nullptr},
};

void L_Map::__persist(lua_State * /* L */) {
}
void L_Map::__unpersist(lua_State * /* L */) {
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
	.. attribute:: width

		(RO) The width of the map in fields.
*/
int L_Map::get_width(lua_State * L) {
	lua_pushuint32(L, get_egbase(L).map().get_width());
	return 1;
}
/* RST
	.. attribute:: height

		(RO) The height of the map in fields.
*/
int L_Map::get_height(lua_State * L) {
	lua_pushuint32(L, get_egbase(L).map().get_height());
	return 1;
}

/* RST
	.. attribute:: player_slots

		(RO) This is an :class:`array` that contains :class:`~wl.map.PlayerSlots`
		for each player defined in the map.
*/
int L_Map::get_player_slots(lua_State * L) {
	Map & m = get_egbase(L).map();

	lua_createtable(L, m.get_nrplayers(), 0);
	for (uint32_t i = 0; i < m.get_nrplayers(); i++) {
		lua_pushuint32(L, i + 1);
		to_lua<LuaMap::L_PlayerSlot>(L, new LuaMap::L_PlayerSlot(i + 1));
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
	.. method:: place_immovable(name, field[, from_where = "world"])

		Creates an immovable that is defined by the world (e.g. trees, stones...)
		or a tribe (field) on a given field. If there is already an immovable on
		the field, an error is reported.

		:arg name: The name of the immovable to create
		:type name: :class:`string`
		:arg field: The immovable is created on this field.
		:type field: :class:`wl.map.Field`
		:arg from_where: a tribe name or "world" that defines where the immovable
			is defined
		:type from_where: :class:`string`

		:returns: The created immovable.
*/
int L_Map::place_immovable(lua_State * const L) {
	std::string from_where = "world";

	char const * const objname = luaL_checkstring(L, 2);
	LuaMap::L_Field * c = *get_user_class<LuaMap::L_Field>(L, 3);
	if (lua_gettop(L) > 3 && !lua_isnil(L, 4))
		from_where = luaL_checkstring(L, 4);

	// Check if the map is still free here
	if
	 (BaseImmovable const * const imm = c->fcoords(L).field->get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(L, "Node is no longer free!");

	Editor_Game_Base & egbase = get_egbase(L);

	BaseImmovable * m = nullptr;
	if (from_where != "world") {
		try {
			const Widelands::Tribe_Descr & tribe =
				egbase.manually_load_tribe(from_where);

			int32_t const imm_idx = tribe.get_immovable_index(objname);
			if (imm_idx < 0)
				return
					report_error
					(L, "Unknown immovable <%s> for tribe <%s>",
					 objname, from_where.c_str());

			m = &egbase.create_immovable(c->coords(), imm_idx, &tribe);
		} catch (game_data_error &) {
			return
				report_error
					(L, "Problem loading tribe <%s>. Maybe not existent?",
					 from_where.c_str());
		}
	} else {
		int32_t const imm_idx = egbase.map().world().get_immovable_index(objname);
		if (imm_idx < 0)
			return report_error(L, "Unknown immovable <%s>", objname);

		m = &egbase.create_immovable(c->coords(), imm_idx, nullptr);
	}

	return LuaMap::upcasted_immovable_to_lua(L, m);
}

/* RST
	.. method:: get_field(x, y)

		Returns a :class:`wl.map.Field` object of the given index.
*/
int L_Map::get_field(lua_State * L) {
	uint32_t x = luaL_checkuint32(L, 2);
	uint32_t y = luaL_checkuint32(L, 3);

	Map & m = get_egbase(L).map();

	if (x >= static_cast<uint32_t>(m.get_width()))
		report_error(L, "x coordinate out of range!");
	if (y >= static_cast<uint32_t>(m.get_height()))
		report_error(L, "y coordinate out of range!");

	return to_lua<LuaMap::L_Field>(L, new LuaMap::L_Field(x, y));
}

/* RST
	.. method:: recalculate()

		This map recalculates the whole map state: height of fields, buildcaps
		and so on. You only need to call this function if you changed
		Field.raw_height in any way.
*/
// TODO: do we really want this function?
int L_Map::recalculate(lua_State * L) {
	get_egbase(L).map().recalc_whole_map();
	return 0;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
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
	METHOD(L_MapObject, has_attribute),
	{nullptr, nullptr},
};
const PropertyType<L_MapObject> L_MapObject::Properties[] = {
	PROP_RO(L_MapObject, __hash),
	PROP_RO(L_MapObject, serial),
	PROP_RO(L_MapObject, type),
	PROP_RO(L_MapObject, name),
	PROP_RO(L_MapObject, descname),
	{nullptr, nullptr, nullptr},
};

void L_MapObject::__persist(lua_State * L) {
	Map_Map_Object_Saver & mos = *get_mos(L);
	Game & game = get_game(L);

	uint32_t idx = 0;
	if (Map_Object* obj = m_ptr.get(game))
		idx = mos.get_object_file_index(*obj);

	PERS_UINT32("file_index", idx);
}
void L_MapObject::__unpersist(lua_State* L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx);

	if (!idx)
		m_ptr = nullptr;
	else {
		Map_Map_Object_Loader& mol = *get_mol(L);
		m_ptr = &mol.get<Map_Object>(idx);
	}
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// Hash is used to identify a class in a Set
int L_MapObject::get___hash(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L))->serial());
	return 1;
}

/* RST
	.. attribute:: serial

		(RO)
		The serial number of this object. Note that this value does not stay
		constant after saving/loading.
*/
int L_MapObject::get_serial(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L))->serial());
	return 1;
}

/* RST
	.. attribute:: type

		(RO) the type name of this map object. You can determine with what kind
		of object you cope by looking at this attribute. Some example types:
		immovable, flag, road, productionsite, warehouse, militarysite...
*/
int L_MapObject::get_type(lua_State * L) {
	lua_pushstring(L, get(L, get_egbase(L))->type_name());
	return 1;
}

/* RST
	.. attribute:: name

		(RO) The internal name of this immovable. This is the same as the
		directory name of this immovable in the tribe or world directory.
*/
int L_MapObject::get_name(lua_State * L) {
	lua_pushstring(L, get(L, get_egbase(L))->descr().name().c_str());
	return 1;
}

/* RST
	.. attribute:: descname

		(RO) The descriptive (and translated) name of this Map Object. Use this
		in messages to the player instead of name.
*/
int L_MapObject::get_descname(lua_State * L) {
	lua_pushstring(L, get(L, get_egbase(L))->descr().descname().c_str());
	return 1;
}




/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
int L_MapObject::__eq(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	L_MapObject * other = *get_base_user_class<L_MapObject>(L, -1);

	Map_Object * me = m_get_or_zero(egbase);
	Map_Object * you = other->m_get_or_zero(egbase);


	// Both objects are destroyed: they are equal
	if (me == you) lua_pushboolean(L, true);
	else if (!me or !you) // One of the objects is destroyed: they are distinct
		lua_pushboolean(L, false);
	else // Compare them
		lua_pushboolean
			(L, other->get(L, egbase)->serial() == get(L, egbase)->serial());

	return 1;
}

/* RST
	.. method:: remove()

		Removes this object immediately. If you want to destroy an
		object as if the player had see :func:`destroy`.
*/
int L_MapObject::remove(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	Map_Object * o = get(L, egbase);

	if (!o)
		return 0;

	o->remove(egbase);
	return 0;
}

/* RST
	.. method:: has_attribute(string)

		returns true, if the map object has this attribute, else false
*/
int L_MapObject::has_attribute(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	Map_Object * obj = m_get_or_zero(egbase);
	if (!obj) {
		lua_pushboolean(L, false);
		return 1;
	}

	// Check if object has the attribute
	std::string attrib = luaL_checkstring(L, 2);
	if (obj->has_attribute(Map_Object_Descr::get_attribute_id(attrib)))
		lua_pushboolean(L, true);
	else
		lua_pushboolean(L, false);
	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
Map_Object* L_MapObject::get(lua_State* L, Editor_Game_Base& egbase, std::string name) {
	Map_Object* o = m_get_or_zero(egbase);
	if (!o)
		report_error(L, "%s no longer exists!", name.c_str());
	return o;
}
Map_Object* L_MapObject::m_get_or_zero(Editor_Game_Base& egbase) {
	return m_ptr.get(egbase);
}

/* RST
BaseImmovable
-------------

.. class:: BaseImmovable

	Child of: :class:`MapObject`

	This is the base class for all Immovables in widelands.
*/
const char L_BaseImmovable::className[] = "BaseImmovable";
const MethodType<L_BaseImmovable> L_BaseImmovable::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_BaseImmovable> L_BaseImmovable::Properties[] = {
	PROP_RO(L_BaseImmovable, size),
	PROP_RO(L_BaseImmovable, name),
	PROP_RO(L_BaseImmovable, fields),
	{nullptr, nullptr, nullptr},
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
	BaseImmovable * o = get(L, get_egbase(L));

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
	.. attribute:: fields

		(RO) An :class:`array` of :class:`~wl.map.Field` that is occupied by this
		Immovable. If the immovable occupies more than one field (roads or big
		buildings for example) the first entry in this list will be the main field
*/
int L_BaseImmovable::get_fields(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);

	BaseImmovable::PositionList pl = get(L, egbase)->get_positions(egbase);

	lua_createtable(L, pl.size(), 0);
	uint32_t idx = 1;
	container_iterate_const(BaseImmovable::PositionList, pl, f) {
		lua_pushuint32(L, idx++);
		to_lua<L_Field>(L, new L_Field(f->x, f->y));
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
*/
const char L_PlayerImmovable::className[] = "PlayerImmovable";
const MethodType<L_PlayerImmovable> L_PlayerImmovable::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_PlayerImmovable> L_PlayerImmovable::Properties[] = {
	PROP_RO(L_PlayerImmovable, owner),
	PROP_RO(L_PlayerImmovable, debug_economy),
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
int L_PlayerImmovable::get_owner(lua_State * L) {
	get_factory(L).push_player
		(L, get(L, get_egbase(L))->get_owner()->player_number());
	return 1;
}

// UNTESTED, for debug only
int L_PlayerImmovable::get_debug_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy());
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
*/
const char L_Flag::className[] = "Flag";
const MethodType<L_Flag> L_Flag::Methods[] = {
	METHOD(L_Flag, set_wares),
	METHOD(L_Flag, get_wares),
	{nullptr, nullptr},
};
const PropertyType<L_Flag> L_Flag::Properties[] = {
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
// Documented in ParentClass
int L_Flag::set_wares(lua_State * L)
{
	Editor_Game_Base & egbase = get_egbase(L);
	Flag * f = get(L, egbase);
	const Tribe_Descr & tribe = f->owner().tribe();

	WaresMap setpoints = m_parse_set_wares_arguments(L, tribe);
	WaresMap c_wares = count_wares_on_flag_(*f, tribe);

	uint32_t nwares = 0;
	container_iterate_const(WaresMap, c_wares, c) {
		// all wares currently on the flag without a setpoint should be removed
		if (!setpoints.count(c->first))
			setpoints.insert(Widelands::WareAmount(c->first, 0));
		nwares += c->second;
	}

	// The idea is to change as little as possible on this flag
	container_iterate_const(WaresMap, setpoints, sp) {
		uint32_t cur = 0;
		WaresMap::iterator i = c_wares.find(sp->first);
		if (i != c_wares.end())
			cur = i->second;

		int d = sp->second - cur;
		nwares += d;

		if (f->total_capacity() < nwares)
			return report_error(L, "Flag has no capacity left!");

		if (d < 0) {
			while (d) {
				Flag::Wares current_wares = f->get_wares();
				container_iterate_const(Flag::Wares, current_wares, w) {
					if (tribe.ware_index((*w.current)->descr().name()) == sp->first) {
						const_cast<WareInstance *>(*w.current)->remove(egbase);
						++d;
						break;
					}
				}
			}
		} else if (d > 0) {
			// add wares
			const WareDescr & wd = *tribe.get_ware_descr(sp->first);
			for (int32_t j = 0; j < d; j++) {
				WareInstance & ware = *new WareInstance(sp->first, &wd);
				ware.init(egbase);
				f->add_ware(egbase, ware);
			}
		}

	}
	return 0;
}

// Documented in parent Class
int L_Flag::get_wares(lua_State * L) {
	const Tribe_Descr & tribe = get(L, get_egbase(L))->owner().tribe();

	bool return_number = false;
	WaresSet wares_set = m_parse_get_wares_arguments(L, tribe, &return_number);

	WaresMap wares = count_wares_on_flag_(*get(L, get_egbase(L)), tribe);

	if (wares_set.size() == tribe.get_nrwares().value()) { // Want all returned
		wares_set.clear();
		container_iterate_const(WaresMap, wares, w)
			wares_set.insert(w->first);
	}

	if (!return_number)
		lua_newtable(L);

	container_iterate_const(WaresSet, wares_set, w) {
		uint32_t count = 0;
		if (wares.count(*w))
			count = wares[*w];

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

	Child of: :class:`PlayerImmovable`, :class:`HasWorkers`

	One flag in the economy of this Player.
*/
const char L_Road::className[] = "Road";
const MethodType<L_Road> L_Road::Methods[] = {
	METHOD(L_Road, get_workers),
	METHOD(L_Road, set_workers),
	{nullptr, nullptr},
};
const PropertyType<L_Road> L_Road::Properties[] = {
	PROP_RO(L_Road, length),
	PROP_RO(L_Road, start_flag),
	PROP_RO(L_Road, end_flag),
	PROP_RO(L_Road, valid_workers),
	PROP_RO(L_Road, road_type),
	{nullptr, nullptr, nullptr},
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
	lua_pushuint32(L, get(L, get_egbase(L))->get_path().get_nsteps());
	return 1;
}

/* RST
	.. attribute:: start_flag

		(RO) The flag were this road starts
*/
int L_Road::get_start_flag(lua_State * L) {
	return
		to_lua<L_Flag>
			(L, new L_Flag(get(L, get_egbase(L))->get_flag(Road::FlagStart)));
}

/* RST
	.. attribute:: end_flag

		(RO) The flag were this road ends
*/
int L_Road::get_end_flag(lua_State * L) {
	return
		to_lua<L_Flag>
			(L, new L_Flag(get(L, get_egbase(L))->get_flag(Road::FlagEnd)));
}

/* RST
	.. attribute:: road_type

		(RO) Type of road. Can be any either of:

		* normal
		* busy
*/
int L_Road::get_road_type(lua_State * L) {
	switch (get(L, get_egbase(L))->get_roadtype()) {
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

// documented in parent class
int L_Road::get_valid_workers(lua_State* L) {
	Road* road = get(L, get_egbase(L));
	return workers_map_to_lua(L, road->owner().tribe(), get_valid_workers_for(*road));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int L_Road::get_workers(lua_State* L) {
	Road* road = get(L, get_egbase(L));
	return do_get_workers(L, *road, get_valid_workers_for(*road));
}

int L_Road::set_workers(lua_State* L) {
	Road* road = get(L, get_egbase(L));
	return do_set_workers<L_Road>(L, road, get_valid_workers_for(*road));
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

int L_Road::create_new_worker
	(PlayerImmovable & pi, Editor_Game_Base & egbase, const Worker_Descr * wdes)
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
		egbase.map().get_neighbour(idle_position, path[i], &idle_position);

	Carrier & carrier = ref_cast<Carrier, Worker>
		(wdes->create (egbase, r.owner(), &r, idle_position));

	if (upcast(Game, game, &egbase))
		carrier.start_task_road(*game);

	r.assign_carrier(carrier, 0);
	return 0;
}


/* RST
PortDock
--------

.. class:: PortDock

	Child of: :class:`PlayerImmovable`

	Each :class:`Warehouse` that is a port has a dock attached to
	it. The PortDock is an immovable that also occupies a field on
	the water near the port.
*/

const char L_PortDock::className[] = "PortDock";
const MethodType<L_PortDock> L_PortDock::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_PortDock> L_PortDock::Properties[] = {
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
*/
const char L_Building::className[] = "Building";
const MethodType<L_Building> L_Building::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_Building> L_Building::Properties[] = {
	PROP_RO(L_Building, building_type),
	PROP_RO(L_Building, flag),
	{nullptr, nullptr, nullptr},
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
	lua_pushstring(L, get(L, get_egbase(L))->type_name());
	return 1;
}

/* RST
	.. attribute:: flag

		(RO) The flag that belongs to this building (that is to the bottom right
		of it's main location).
*/
// UNTESTED
int L_Building::get_flag(lua_State * L) {
	return upcasted_immovable_to_lua(L, &get(L, get_egbase(L))->base_flag());
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
	the moment
*/
const char L_ConstructionSite::className[] = "ConstructionSite";
const MethodType<L_ConstructionSite> L_ConstructionSite::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_ConstructionSite> L_ConstructionSite::Properties[] = {
	PROP_RO(L_ConstructionSite, building),
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
int L_ConstructionSite::get_building(lua_State * L) {
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
	{nullptr, nullptr},
};
const PropertyType<L_Warehouse> L_Warehouse::Properties[] = {
	PROP_RO(L_Warehouse, portdock),
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
int L_Warehouse::get_portdock(lua_State * L) {
	return upcasted_immovable_to_lua(L, get(L, get_egbase(L))->get_portdock());
}


/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */
#define WH_SET(type, btype) \
int L_Warehouse::set_##type##s(lua_State * L) { \
	Warehouse * wh = get(L, get_egbase(L)); \
	const Tribe_Descr & tribe = wh->owner().tribe(); \
	btype##sMap setpoints = m_parse_set_##type##s_arguments(L, tribe); \
 \
	container_iterate_const(btype##sMap, setpoints, i) { \
		int32_t d = i.current->second - \
			wh->get_##type##s().stock(i.current->first); \
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
	Warehouse * wh = get(L, get_egbase(L)); \
	const Tribe_Descr & tribe = wh->owner().tribe(); \
	bool return_number = false; \
	btype##sSet set = m_parse_get_##type##s_arguments \
		(L, tribe, &return_number); \
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

// documented in parent class
int L_Warehouse::get_soldiers(lua_State* L) {
	Warehouse* wh = get(L, get_egbase(L));
	return do_get_soldiers(L, *wh, wh->owner().tribe());
}

// documented in parent class
int L_Warehouse::set_soldiers(lua_State* L) {
	Warehouse* wh = get(L, get_egbase(L));
	return do_set_soldiers(L, wh->get_position(), wh, wh->get_owner());
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

	Child of: :class:`Building`, :class:`HasWares`, :class:`HasWorkers`

	Every building that produces anything.
*/
const char L_ProductionSite::className[] = "ProductionSite";
const MethodType<L_ProductionSite> L_ProductionSite::Methods[] = {
	METHOD(L_ProductionSite, set_wares),
	METHOD(L_ProductionSite, get_wares),
	METHOD(L_ProductionSite, get_workers),
	METHOD(L_ProductionSite, set_workers),
	{nullptr, nullptr},
};
const PropertyType<L_ProductionSite> L_ProductionSite::Properties[] = {
	PROP_RO(L_ProductionSite, valid_workers),
	PROP_RO(L_ProductionSite, valid_wares),
	{nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// documented in parent class
int L_ProductionSite::get_valid_wares(lua_State * L) {
	ProductionSite * ps = get(L, get_egbase(L));

	const Tribe_Descr & tribe = ps->owner().tribe();

	lua_newtable(L);
	container_iterate_const(BillOfMaterials, ps->descr().inputs(), i) {
		lua_pushstring(L, tribe.get_ware_descr(i.current->first)->name());
		lua_pushuint32(L, i.current->second);
		lua_rawset(L, -3);
	}
	return 1;
}

// documented in parent class
int L_ProductionSite::get_valid_workers(lua_State * L) {
	ProductionSite* ps = get(L, get_egbase(L));
	return workers_map_to_lua(L, ps->owner().tribe(), get_valid_workers_for(*ps));
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int L_ProductionSite::set_wares(lua_State * L) {
	ProductionSite * ps = get(L, get_egbase(L));
	const Tribe_Descr & tribe = ps->owner().tribe();

	WaresMap setpoints = m_parse_set_wares_arguments(L, tribe);

	WaresSet valid_wares;
	container_iterate_const(BillOfMaterials, ps->descr().inputs(), i)
		valid_wares.insert(i->first);

	container_iterate_const(WaresMap, setpoints, i) {
		if (!valid_wares.count(i->first))
			return
				report_error
				 (L, "<%s> can't be stored here!",
				  tribe.get_ware_descr(i->first)->name().c_str());

		WaresQueue & wq = ps->waresqueue(i->first);
		if (i->second > wq.get_max_size())
			return
				report_error
					(L, "Not enough space for %u items, only for %i",
					 i->second, wq.get_max_size());

		wq.set_filled(i->second);
	}

	return 0;
}

// documented in parent class
int L_ProductionSite::get_wares(lua_State * L) {
	ProductionSite * ps = get(L, get_egbase(L));
	const Tribe_Descr & tribe = ps->owner().tribe();

	bool return_number = false;
	WaresSet wares_set = m_parse_get_wares_arguments(L, tribe, &return_number);

	WaresSet valid_wares;
	container_iterate_const(BillOfMaterials, ps->descr().inputs(), i)
		valid_wares.insert(i.current->first);

	if (wares_set.size() == tribe.get_nrwares().value()) // Want all returned
		wares_set = valid_wares;

	if (!return_number)
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

// documented in parent class
int L_ProductionSite::get_workers(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	return do_get_workers(L, *ps, get_valid_workers_for(*ps));
}

// documented in parent class
int L_ProductionSite::set_workers(lua_State* L) {
	ProductionSite* ps = get(L, get_egbase(L));
	return do_set_workers<L_ProductionSite>(L, ps, get_valid_workers_for(*ps));
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */

int L_ProductionSite::create_new_worker
	(PlayerImmovable & pi, Editor_Game_Base & egbase, const Worker_Descr * wdes)
{
	ProductionSite & ps = static_cast<ProductionSite &>(pi);
	return ps.warp_worker(egbase, *wdes);
}



/* RST
MilitarySite
--------------

.. class:: MilitarySite

	Child of: :class:`Building`, :class:`HasSoldiers`

	Miltary Buildings
*/
const char L_MilitarySite::className[] = "MilitarySite";
const MethodType<L_MilitarySite> L_MilitarySite::Methods[] = {
	METHOD(L_MilitarySite, get_soldiers),
	METHOD(L_MilitarySite, set_soldiers),
	{nullptr, nullptr},
};
const PropertyType<L_MilitarySite> L_MilitarySite::Properties[] = {
	PROP_RO(L_MilitarySite, max_soldiers),
	{nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

// documented in parent class
int L_MilitarySite::get_max_soldiers(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldierCapacity());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int L_MilitarySite::get_soldiers(lua_State* L) {
	MilitarySite* ms = get(L, get_egbase(L));
	return do_get_soldiers(L, *ms, ms->owner().tribe());
}

// documented in parent class
int L_MilitarySite::set_soldiers(lua_State* L) {
	MilitarySite* ms = get(L, get_egbase(L));
	return do_set_soldiers(L, ms->get_position(), ms, ms->get_owner());
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

	Child of: :class:`Building`, :class:`HasSoldiers`

	Miltary Buildings
*/
const char L_TrainingSite::className[] = "TrainingSite";
const MethodType<L_TrainingSite> L_TrainingSite::Methods[] = {
	METHOD(L_TrainingSite, get_soldiers),
	METHOD(L_TrainingSite, set_soldiers),
	{nullptr, nullptr},
};
const PropertyType<L_TrainingSite> L_TrainingSite::Properties[] = {
	PROP_RO(L_TrainingSite, max_soldiers),
	{nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */

// documented in parent class
int L_TrainingSite::get_max_soldiers(lua_State* L) {
	lua_pushuint32(L, get(L, get_egbase(L))->soldierCapacity());
	return 1;
}

/*
 ==========================================================
 LUA METHODS
 ==========================================================
 */

// documented in parent class
int L_TrainingSite::get_soldiers(lua_State* L) {
	TrainingSite* ts = get(L, get_egbase(L));
	return do_get_soldiers(L, *ts, ts->owner().tribe());
}

// documented in parent class
int L_TrainingSite::set_soldiers(lua_State* L) {
	TrainingSite* ts = get(L, get_egbase(L));
	return do_set_soldiers(L, ts->get_position(), ts, ts->get_owner());
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
*/
const char L_Bob::className[] = "Bob";
const MethodType<L_Bob> L_Bob::Methods[] = {
	METHOD(L_Bob, has_caps),
	{nullptr, nullptr},
};
const PropertyType<L_Bob> L_Bob::Properties[] = {
	{nullptr, nullptr, nullptr},
};

/*
 ==========================================================
 PROPERTIES
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
int L_Bob::has_caps(lua_State * L) {
	std::string query = luaL_checkstring(L, 2);

	uint32_t movecaps = get(L, get_egbase(L))->descr().movecaps();

	if (query == "swims")
		lua_pushboolean(L, movecaps & MOVECAPS_SWIM);
	else if (query == "walks")
		lua_pushboolean(L,  movecaps & MOVECAPS_WALK);
	else
		return report_error(L, "Unknown caps queried: %s!", query.c_str());

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
Ship
----

.. class:: Ship

	This represents a ship in game.
*/

const char L_Ship::className[] = "Ship";
const MethodType<L_Ship> L_Ship::Methods[] = {
	METHOD(L_Ship, get_wares),
	METHOD(L_Ship, get_workers),
	{nullptr, nullptr},
};
const PropertyType<L_Ship> L_Ship::Properties[] = {
	PROP_RO(L_Ship, debug_economy),
	PROP_RO(L_Ship, last_portdock),
	PROP_RO(L_Ship, destination),
	{nullptr, nullptr, nullptr},
};


/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
// UNTESTED, for debug only
int L_Ship::get_debug_economy(lua_State* L) {
	lua_pushlightuserdata(L, get(L, get_egbase(L))->get_economy());
	return 1;
}

/* RST
	.. attribute:: destination

		(RO) Either :const:`nil` if there is no current destination, otherwise
		the :class:`PortDock`.
*/
// UNTESTED
int L_Ship::get_destination(lua_State* L) {
	Editor_Game_Base & egbase = get_egbase(L);
	return upcasted_immovable_to_lua(L, get(L, egbase)->get_destination(egbase));
}

/* RST
	.. attribute:: last_portdock

		(RO) Either :const:`nil` if no port was ever visited or the last portdock
		was destroyed, otherwise the :class:`PortDock` of the last visited port.
*/
// UNTESTED
int L_Ship::get_last_portdock(lua_State* L) {
	Editor_Game_Base & egbase = get_egbase(L);
	return upcasted_immovable_to_lua(L, get(L, egbase)->get_lastdock(egbase));
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
int L_Ship::get_wares(lua_State* L) {
	Editor_Game_Base& egbase = get_egbase(L);
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
int L_Ship::get_workers(lua_State* L) {
	Editor_Game_Base& egbase = get_egbase(L);
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
*/

const char L_Worker::className[] = "Worker";
const MethodType<L_Worker> L_Worker::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_Worker> L_Worker::Properties[] = {
	PROP_RO(L_Worker, owner),
	PROP_RO(L_Worker, location),
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
int L_Worker::get_owner(lua_State * L) {
	get_factory(L).push_player
		(L, get(L, get_egbase(L))->get_owner()->player_number());
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
int L_Worker::get_location(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	return
		upcasted_immovable_to_lua
			(L, static_cast<BaseImmovable *>
			 	(get(L, egbase)->get_location(egbase)));
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
*/

const char L_Soldier::className[] = "Soldier";
const MethodType<L_Soldier> L_Soldier::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_Soldier> L_Soldier::Properties[] = {
	PROP_RO(L_Soldier, attack_level),
	PROP_RO(L_Soldier, defense_level),
	PROP_RO(L_Soldier, hp_level),
	PROP_RO(L_Soldier, evade_level),
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
int L_Soldier::get_attack_level(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_attack_level());
	return 1;
}

/* RST
	.. attribute:: defense_level

		(RO) The current defense level of this soldier
*/
// UNTESTED
int L_Soldier::get_defense_level(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_defense_level());
	return 1;
}

/* RST
	.. attribute:: hp_level

		(RO) The current hp level of this soldier
*/
// UNTESTED
int L_Soldier::get_hp_level(lua_State * L) {
	lua_pushuint32(L, get(L, get_egbase(L))->get_hp_level());
	return 1;
}

/* RST
	.. attribute:: evade_level

		(RO) The current evade level of this soldier
*/
// UNTESTED
int L_Soldier::get_evade_level(lua_State * L) {
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

	You cannot instantiate this class directly, instead use
	:meth:`wl.map.Map.get_field`.
*/

const char L_Field::className[] = "Field";
const MethodType<L_Field> L_Field::Methods[] = {
	METHOD(L_Field, __eq),
	METHOD(L_Field, __tostring),
	METHOD(L_Field, region),
	METHOD(L_Field, has_caps),
	{nullptr, nullptr},
};
const PropertyType<L_Field> L_Field::Properties[] = {
	PROP_RO(L_Field, __hash),
	PROP_RO(L_Field, x),
	PROP_RO(L_Field, y),
	PROP_RO(L_Field, rn),
	PROP_RO(L_Field, ln),
	PROP_RO(L_Field, trn),
	PROP_RO(L_Field, tln),
	PROP_RO(L_Field, bln),
	PROP_RO(L_Field, brn),
	PROP_RO(L_Field, immovable),
	PROP_RO(L_Field, bobs),
	PROP_RW(L_Field, terr),
	PROP_RW(L_Field, terd),
	PROP_RW(L_Field, height),
	PROP_RW(L_Field, raw_height),
	PROP_RO(L_Field, viewpoint_x),
	PROP_RO(L_Field, viewpoint_y),
	PROP_RW(L_Field, resource),
	PROP_RW(L_Field, resource_amount),
	PROP_RO(L_Field, claimers),
	PROP_RO(L_Field, owner),
	{nullptr, nullptr, nullptr},
};


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
// Hash is used to identify a class in a Set
int L_Field::get___hash(lua_State * L) {
	char buf[25];
	snprintf(buf, sizeof(buf), "%i_%i", m_c.x, m_c.y);
	lua_pushstring(L, buf);
	return 1;
}

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
	FCoords f = fcoords(L);

	if (f.field->get_height() == height)
		return 0;

	if (height > MAX_FIELD_HEIGHT)
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);

	get_egbase(L).map().set_height(f, height);

	return 0;
}

/* RST
	.. attribute:: raw_height

		(RW The same as :attr:`height`, but setting this will not trigger a
		recalculation of the surrounding fields. You can use this field to
		change the height of many fields on a map quickly, then use
		:func:`wl.map.recalculate()` to make sure that everything is in order.
*/
// UNTESTED
int L_Field::get_raw_height(lua_State * L) {
	lua_pushuint32(L, fcoords(L).field->get_height());
	return 1;
}
int L_Field::set_raw_height(lua_State * L) {
	uint32_t height = luaL_checkuint32(L, -1);
	FCoords f = fcoords(L);

	if (f.field->get_height() == height)
		return 0;

	if (height > MAX_FIELD_HEIGHT)
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);

	f.field->set_height(height);

	return 0;
}


/* RST
	.. attribute:: viewpoint_x, viewpoint_y

		(RO) Returns the position in pixels to move the view to to center
		this field for the current interactive player
*/
int L_Field::get_viewpoint_x(lua_State * L) {
	int32_t px, py;
	MapviewPixelFunctions::get_save_pix(get_egbase(L).map(), m_c, px, py);
	lua_pushint32(L, px);
	return 1;
}
int L_Field::get_viewpoint_y(lua_State * L) {
	int32_t px, py;
	MapviewPixelFunctions::get_save_pix(get_egbase(L).map(), m_c, px, py);
	lua_pushint32(L, py);
	return 1;
}

/* RST
	.. attribute:: resource

		(RW) The name of the resource that is available in this field or
		:const:`nil`

		:see also: :attr:`resource_amount`
*/
int L_Field::get_resource(lua_State * L) {
	lua_pushstring
		(L, get_egbase(L).map().world().get_resource
			 (fcoords(L).field->get_resources())->name().c_str());

	return 1;
}
int L_Field::set_resource(lua_State * L) {
	Field * field = fcoords(L).field;
	int32_t res = get_egbase(L).map().world().get_resource
		(luaL_checkstring(L, -1));

	if (res == -1)
		return report_error(L, "Illegal resource: '%s'", luaL_checkstring(L, -1));

	field->set_resources(res, field->get_resources_amount());

	return 0;
}

/* RST
	.. attribute:: resource_amount

		(RW) How many items of the resource is available in this field.

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
	int32_t max_amount = get_egbase(L).map().world().get_resource
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
	BaseImmovable * bi = get_egbase(L).map().get_immovable(m_c);

	if (!bi)
		return 0;
	else
		upcasted_immovable_to_lua(L, bi);
	return 1;
}

/* RST
	.. attribute:: bobs

		(RO) An :class:`array` of :class:`~wl.map.Bob` that are associated
		with this field
*/
// UNTESTED
int L_Field::get_bobs(lua_State * L) {
	Bob * b = fcoords(L).field->get_first_bob();

	lua_newtable(L);
	uint32_t cidx = 1;
	while (b) {
		lua_pushuint32(L, cidx++);
		upcasted_bob_to_lua(L, b);
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
		valid name to these variables.
*/
int L_Field::get_terr(lua_State * L) {
	Terrain_Descr & td =
		get_egbase(L).map().world().terrain_descr
			(fcoords(L).field->terrain_r());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
int L_Field::set_terr(lua_State * L) {
	const char * name = luaL_checkstring(L, -1);
	Map & map = get_egbase(L).map();
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
		get_egbase(L).map().world().terrain_descr
			(fcoords(L).field->terrain_d());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
int L_Field::set_terd(lua_State * L) {
	const char * name = luaL_checkstring(L, -1);
	Map & map = get_egbase(L).map();
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
   get_egbase(L).map().get_ ##X(m_c, &n); \
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
	.. attribute:: owner

		(RO) The current owner of the field or :const:`nil` if noone owns it. See
		also :attr:`claimers`.
*/
int L_Field::get_owner(lua_State * L) {
	Player_Number current_owner = fcoords(L).field->get_owned_by();
	if (current_owner) {
		get_factory(L).push_player(L, current_owner);
		return 1;
	}
	return 0;
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
int L_Field::get_claimers(lua_State * L) {
	Editor_Game_Base & egbase = get_egbase(L);
	Map & map = egbase.map();

	std::vector<PlrInfluence> claimers;

	iterate_players_existing(other_p, map.get_nrplayers(), egbase, plr)
		claimers.push_back
			(PlrInfluence(plr->player_number(), plr->military_influence
					(map.get_index(m_c, map.get_width()))
			)
		);

	std::stable_sort (claimers.begin(), claimers.end(), sort_claimers);

	lua_createtable(L, 1, 0); // We mostly expect one claimer per field.

	// Push the players with military influence
	uint32_t cidx = 1;
	container_iterate_const (std::vector<PlrInfluence>, claimers, i) {
		if (i.current->second <= 0)
			continue;
		lua_pushuint32(L, cidx ++);
		get_factory(L).push_player(L, i.current->first);
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
int L_Field::__tostring(lua_State * L) {
	char buf[100];
	snprintf(buf, sizeof(buf), "Field(%i,%i)", m_c.x, m_c.y);
	lua_pushstring(L, buf);
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


/* RST
	.. method:: has_caps(capname)

		Returns :const:`true` if the field has this caps associated
		with it, otherwise returns false.

		:arg capname: can be either of

		* :const:`small`: Can a small building be build here?
		* :const:`medium`: Can a medium building be build here?
		* :const:`big`: Can a big building be build here?
		* :const:`mine`: Can a mine be build here?
		* :const:`port`: Can a port be build here?
		* :const:`flag`: Can a flag be build here?
		* :const:`walkable`: Is this field passable for walking bobs?
		* :const:`swimable`: Is this field passable for swimming bobs?
*/
int L_Field::has_caps(lua_State * L) {
	FCoords f = fcoords(L);
	std::string query = luaL_checkstring(L, 2);

	if (query == "walkable")
		lua_pushboolean(L, f.field->nodecaps() & MOVECAPS_WALK);
	else if (query == "swimmable")
		lua_pushboolean(L, f.field->nodecaps() & MOVECAPS_SWIM);
	else if (query == "small")
		lua_pushboolean(L, f.field->nodecaps() & BUILDCAPS_SMALL);
	else if (query == "medium")
		lua_pushboolean(L, f.field->nodecaps() & BUILDCAPS_MEDIUM);
	else if (query == "big")
		lua_pushboolean(L, (f.field->nodecaps() & BUILDCAPS_BIG) == BUILDCAPS_BIG);
	else if (query == "port") {
		lua_pushboolean
			(L, (f.field->nodecaps() & BUILDCAPS_PORT) && get_egbase(L).map().is_port_space(f));
	} else if (query == "mine")
		lua_pushboolean(L, f.field->nodecaps() & BUILDCAPS_MINE);
	else if (query == "flag")
		lua_pushboolean(L, f.field->nodecaps() & BUILDCAPS_FLAG);
	else
		return report_error(L, "Unknown caps queried: %s!", query.c_str());

	return 1;
}

/*
 ==========================================================
 C METHODS
 ==========================================================
 */
int L_Field::m_region(lua_State * L, uint32_t radius)
{
	Map & map = get_egbase(L).map();
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
	Map & map = get_egbase(L).map();
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
	return get_egbase(L).map().get_fcoords(m_c);
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
const char L_PlayerSlot::className[] = "PlayerSlot";
const MethodType<L_PlayerSlot> L_PlayerSlot::Methods[] = {
	{nullptr, nullptr},
};
const PropertyType<L_PlayerSlot> L_PlayerSlot::Properties[] = {
	PROP_RO(L_PlayerSlot, tribe_name),
	PROP_RO(L_PlayerSlot, name),
	PROP_RO(L_PlayerSlot, starting_field),
	{nullptr, nullptr, nullptr},
};

void L_PlayerSlot::__persist(lua_State * L) {
	PERS_UINT32("player", m_plr);
}
void L_PlayerSlot::__unpersist(lua_State * L) {
	UNPERS_UINT32("player", m_plr);
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
int L_PlayerSlot::get_tribe_name(lua_State * L) {
	lua_pushstring(L, get_egbase(L).get_map()->get_scenario_player_tribe(m_plr));
	return 1;
}

/* RST
	.. attribute:: name

		(RO) The name for this player as suggested in this map
*/
int L_PlayerSlot::get_name(lua_State * L) {
	lua_pushstring(L, get_egbase(L).get_map()->get_scenario_player_name(m_plr));
	return 1;
}

/* RST
	.. attribute:: starting_field

		(RO) The starting_field for this player as set in the map.
		Note that it is not guaranteed that the HQ of the player is on this
		field as scenarios and starting conditions are free to place the HQ
		wherever it want. This field is only centered when the game starts.
*/
int L_PlayerSlot::get_starting_field(lua_State * L) {
	to_lua<L_Field>(L, new L_Field(get_egbase(L).map().get_starting_pos(m_plr)));
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

const static struct luaL_Reg wlmap [] = {
	{nullptr, nullptr}
};

void luaopen_wlmap(lua_State * L) {
	lua_getglobal(L, "wl");  // S: wl_table
	lua_pushstring(L, "map"); // S: wl_table "map"
	luaL_newlib(L, wlmap);  // S: wl_table "map" wl.map_table
	lua_settable(L, -3); // S: wl_table
	lua_pop(L, 1); // S:

	register_class<L_Map>(L, "map");
	register_class<L_Field>(L, "map");
	register_class<L_PlayerSlot>(L, "map");
	register_class<L_MapObject>(L, "map");


	register_class<L_Bob>(L, "map", true);
	add_parent<L_Bob, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Worker>(L, "map", true);
	add_parent<L_Worker, L_Bob>(L);
	add_parent<L_Worker, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Soldier>(L, "map", true);
	add_parent<L_Soldier, L_Worker>(L);
	add_parent<L_Soldier, L_Bob>(L);
	add_parent<L_Soldier, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

	register_class<L_Ship>(L, "map", true);
	add_parent<L_Ship, L_Bob>(L);
	add_parent<L_Ship, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table

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

	register_class<L_PortDock>(L, "map", true);
	add_parent<L_PortDock, L_PlayerImmovable>(L);
	add_parent<L_PortDock, L_BaseImmovable>(L);
	add_parent<L_PortDock, L_MapObject>(L);
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

	register_class<L_ConstructionSite>(L, "map", true);
	add_parent<L_ConstructionSite, L_Building>(L);
	add_parent<L_ConstructionSite, L_PlayerImmovable>(L);
	add_parent<L_ConstructionSite, L_BaseImmovable>(L);
	add_parent<L_ConstructionSite, L_MapObject>(L);
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

	register_class<L_TrainingSite>(L, "map", true);
	add_parent<L_TrainingSite, L_ProductionSite>(L);
	add_parent<L_TrainingSite, L_Building>(L);
	add_parent<L_TrainingSite, L_PlayerImmovable>(L);
	add_parent<L_TrainingSite, L_BaseImmovable>(L);
	add_parent<L_TrainingSite, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table
}

};
