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
#include "logic/mapregion.h"
#include "logic/maphollowregion.h"
#include "logic/widelands_geometry.h"

#include "c_utils.h"

#include "lua_map.h"

// TODO: make map a class and add attributes width, height

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

		uint32_t idx = mos.get_object_file_index(*m_ptr->get(game));
		PERS_UINT32("file_index", idx);
	}
void L_MapObject::__unpersist(lua_State * L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx);
	Map_Map_Object_Loader & mol = *get_mol(L);

	m_ptr = new Object_Ptr(&mol.get<Map_Object>(idx));
}

/*
 ==========================================================
 PROPERTIES
 ==========================================================
 */
/* RST
.. attribute:: serial

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
Map_Object * L_MapObject::m_get(Game & game, lua_State * L) {
	Map_Object * o = m_ptr->get(game);
	if (!o)
		report_error(L, "Object no longer exists!");
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

void L_BaseImmovable::__persist(lua_State * L) {
	Map_Map_Object_Saver & mos = *get_mos(L);
	Game & game = get_game(L);

	uint32_t idx = mos.get_object_file_index(*m_biptr->get(game));
	PERS_UINT32("file_index", idx);

	L_MapObject::__persist(L);
}
void L_BaseImmovable::__unpersist(lua_State * L) {
	uint32_t idx;
	UNPERS_UINT32("file_index", idx);
	Map_Map_Object_Loader & mol = *get_mol(L);

	m_biptr = new OPtr<BaseImmovable>(&mol.get<BaseImmovable>(idx));

	L_MapObject::__unpersist(L);
}

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
BaseImmovable * L_BaseImmovable::m_get(Game & game, lua_State * L) {
	BaseImmovable * o = m_biptr->get(game);
	if (!o)
		report_error(L, "BaseImmovable no longer exists!");
	return o;
}


/*
 * TODO: document me
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
	m_c.field = &get_game(L).map()[m_c];
}

void L_Field::__persist(lua_State * L) {
	PERS_INT32("x", m_c.x); PERS_INT32("y", m_c.y);
}
void L_Field::__unpersist(lua_State * L) {
	UNPERS_INT32("x", m_c.x); UNPERS_INT32("y", m_c.y);
	m_c.field = &get_game(L).map()[m_c];
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
	lua_pushuint32(L, m_c.field->get_height());
	return 1;
}
int L_Field::set_height(lua_State * L) {
	uint32_t height = luaL_checkuint32(L, -1);
	if (height > MAX_FIELD_HEIGHT)
		report_error(L, "height must be <= %i", MAX_FIELD_HEIGHT);

	get_game(L).map().set_height(m_c, height);

	return get_height(L);
}

// TODO: document me
int L_Field::get_immovable(lua_State * L) {
	BaseImmovable * bi = m_c.field->get_immovable();

	if (!bi)
		lua_pushnil(L);
	else
		to_lua<L_BaseImmovable>(L, new L_BaseImmovable(*bi));
	return 1;
}

// TODO: document me
int L_Field::get_terr(lua_State * L) {
	Terrain_Descr & td =
		get_game(L).map().world().terrain_descr(m_c.field->terrain_r());
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

	map.change_terrain(TCoords<FCoords>(m_c, TCoords<FCoords>::R), td);


	lua_pushstring(L, name);
	return 1;
}

// TODO: document me
int L_Field::get_terd(lua_State * L) {
	Terrain_Descr & td =
		get_game(L).map().world().terrain_descr(m_c.field->terrain_d());
	lua_pushstring(L, td.name().c_str());
	return 1;
}
// TODO: changing the terrain only works when the user has vision on the
// TODO: triangles changed. this is surely not intentional but must be coped
// TODO: with
int L_Field::set_terd(lua_State * L) {
	const char * name = luaL_checkstring(L, -1);
	Map & map = get_game(L).map();
	Terrain_Index td =
		map.world().index_of_terrain(name);
	if (td == static_cast<Terrain_Index>(-1))
		report_error(L, "Unknown terrain '%s'", name);

	map.change_terrain(TCoords<FCoords>(m_c, TCoords<FCoords>::D), td);

	lua_pushstring(L, name);
	return 1;
}

#define GET_X_NEIGHBOUR(X) int L_Field::get_ ##X(lua_State* L) { \
   FCoords n; \
   get_game(L).map().get_ ##X(m_c, &n); \
   to_lua<L_Field>(L, new L_Field(n.x, n.y, n.field)); \
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
		(map, Area<FCoords>(m_c, radius));

	lua_newtable(L);
	uint32_t idx = 1;
	do {
		lua_pushuint32(L, idx++);
		const FCoords & loc = mr.location();
		to_lua<L_Field>(L, new L_Field(loc.x, loc.y, loc.field));
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
		const Coords & loc = mr.location();
		to_lua<L_Field>(L, new L_Field(loc.x, loc.y, &map[loc]));
		lua_settable(L, -3);
	} while (mr.advance(map));

	return 1;
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
	 (BaseImmovable const * const imm = c->coords().field->get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(L, "Node is no longer free!");

	int32_t const imm_idx = game.map().world().get_immovable_index(objname);
	if (imm_idx < 0)
		return report_error(L, "Unknown immovable <%s>", objname);

	BaseImmovable & m = game.create_immovable(c->coords(), imm_idx, 0);

	// Send this to lua
	return to_lua<L_BaseImmovable>(L, new L_BaseImmovable(m));
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
}

