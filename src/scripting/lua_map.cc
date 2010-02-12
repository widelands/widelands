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

#include "log.h"
#include "logic/checkstep.h"
#include "logic/findimmovable.h"
#include "logic/immovable.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "logic/warelist.h"
#include "logic/widelands_geometry.h"
#include "wui/mapviewpixelconstants.h"

#include "c_utils.h"
#include "lua_game.h"

#include "lua_map.h"

// TODO: get_width, get_height

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
			else
				return CAST_TO_LUA(Building);

		case Map_Object::FLAG:
			return CAST_TO_LUA(Flag);
		case Map_Object::ROAD:
			return CAST_TO_LUA(PlayerImmovable);
		// TODO: Handle ROAD
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
			(L, other->m_get(game, L)->serial() == m_get(game, L)->serial());

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
	Map_Object * o = m_get(game, L);

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
Map_Object * L_MapObject::m_get
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

	Bases: :class:`MapObject`

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
// TODO: document me
int L_BaseImmovable::get_size(lua_State * L) {
	Game & game = get_game(L);
	BaseImmovable * o = m_get(game, L);

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

// TODO: document me
int L_BaseImmovable::get_name(lua_State * L) {
	Game & game = get_game(L);
	BaseImmovable * o = m_get(game, L);

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

	Bases: :class:`BaseImmovable`

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
			(L, new L_Player (m_get(get_game(L), L)->get_owner()->player_number())
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
		report_error(L, "Invalid " #capname ": %s", what.c_str()); \
	return idx; \
}
GET_INDEX(ware, Ware);
GET_INDEX(worker, Worker);
#undef GET_INDEX


/* RST
Flag
--------

.. class:: Flag

	Bases: :class:`PlayerImmovable`

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
	Flag * f = m_get(game, L);

	if (not f->has_capacity())
		return report_error(L, "Flag has no capacity left!");

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
Building
--------

.. class:: Building

	Bases: :class:`PlayerImmovable`

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
	lua_pushstring(L, m_get(get_game(L), L)->type_name());
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

	Bases: :class:`Building`

	Every Headquarter or Warehouse on the Map is of this type.
*/
const char L_Warehouse::className[] = "Warehouse";
const MethodType<L_Warehouse> L_Warehouse::Methods[] = {
	METHOD(L_Warehouse, set_wares),
	METHOD(L_Warehouse, get_wares),
	METHOD(L_Warehouse, set_workers),
	METHOD(L_Warehouse, get_workers),
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
	Warehouse * o = m_get(get_game(L), L); \
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
	Warehouse * o = m_get(get_game(L), L); \
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
// TODO: document me
int L_Field::get_x(lua_State * L) {lua_pushuint32(L, m_c.x); return 1;}
// TODO: document me
int L_Field::get_y(lua_State * L) {lua_pushuint32(L, m_c.y); return 1;}

// TODO: document me
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

// TODO: document me
int L_Field::get_immovable(lua_State * L) {
	BaseImmovable * bi = get_game(L).map().get_immovable(m_c);

	if (!bi)
		return 0;
	else
		upcasted_immovable_to_lua(L, bi);
	return 1;
}

// TODO: document me
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

// TODO: document me
int L_Field::get_terd(lua_State * L) {
	Terrain_Descr & td =
		get_game(L).map().world().terrain_descr
			(fcoords(L).field->terrain_d());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
// TODO: document me
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

#define GET_X_NEIGHBOUR(X) int L_Field::get_ ##X(lua_State* L) { \
   Coords n; \
   get_game(L).map().get_ ##X(m_c, &n); \
   to_lua<L_Field>(L, new L_Field(n.x, n.y)); \
	return 1; \
}
// TODO: document me
	GET_X_NEIGHBOUR(rn);
// TODO: document me
	GET_X_NEIGHBOUR(ln);
// TODO: document me
	GET_X_NEIGHBOUR(trn);
// TODO: document me
	GET_X_NEIGHBOUR(tln);
// TODO: document me
	GET_X_NEIGHBOUR(bln);
// TODO: document me
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
// TODO: document me
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

/*
 * TODO: document this properly
 * Create a World immovable object immediately
 *
 * name: name of object to create
 * field: L_Field, position where to create this immovable
 *
 */
static int L_create_immovable(lua_State * const L) {
	char const * const objname = luaL_checkstring(L, 1);
	L_Field * c = *get_user_class<L_Field>(L, 2);

	Game & game = get_game(L);

	// Check if the map is still free here
	// TODO: this exact code is duplicated in worker.cc
	if
	 (BaseImmovable const * const imm = c->fcoords(L).field->get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(L, "Node is no longer free!");

	int32_t const imm_idx = game.map().world().get_immovable_index(objname);
	if (imm_idx < 0)
		return report_error(L, "Unknown immovable <%s>", objname);

	BaseImmovable & m = game.create_immovable(c->coords(), imm_idx, 0);

	// Send this to lua
	return upcasted_immovable_to_lua(L, &m);
}


/*
 * Find a world immovable
 * TODO: document this properly
 * TODO: this function should return a list of all immovables found
 *
 * x, y, radius - position to search for
 * attrib - attribute to use
 *
 * Returns: x, y position of object
 */
static int L_find_immovable(lua_State * const L) {
	uint32_t     const x       = luaL_checkint32(L, 1);
	uint32_t     const y       = luaL_checkint32(L, 2);
	uint32_t     const radius  = luaL_checkint32(L, 3);
	const char *  const attrib  = luaL_checkstring(L, 4);

	Coords pos(x, y);
	Game & game = get_game(L);
	Map & map = game.map();

	Area<FCoords> area (map.get_fcoords(pos), 0);
	CheckStepWalkOn cstep(MOVECAPS_WALK, false);
	int attribute = Map_Object_Descr::get_attribute_id(attrib);

	for (;; ++area.radius) {
		if (radius < area.radius)
			return report_error(L, "No suitable object in radius %i", radius);

		std::vector<ImmovableFound> list;
		//  if (action.iparam2 < 0)
		//          map.find_reachable_immovables
		//                  (area, &list, cstep);
		//  else
		map.find_reachable_immovables
			(area, &list, cstep, FindImmovableAttribute(attribute));

		if (list.size()) {
			Coords & rv = list[game.logic_rand() % list.size()].coords;
			lua_pushinteger(L, rv.x);
			lua_pushinteger(L, rv.y);
			return 2;
		}
	}

	return 0;
}

const static struct luaL_reg wlmap [] = {
	{"create_immovable", &L_create_immovable},
	{"find_immovable", &L_find_immovable},
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

	register_class<L_Warehouse>(L, "map", true);
	add_parent<L_Warehouse, L_Building>(L);
	add_parent<L_Warehouse, L_PlayerImmovable>(L);
	add_parent<L_Warehouse, L_BaseImmovable>(L);
	add_parent<L_Warehouse, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table
}

