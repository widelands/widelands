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

#include "lua_map.h"

#include "log.h"
#include "logic/widelands_geometry.h"
#include "logic/immovable.h"
#include "logic/game.h"
#include "logic/checkstep.h"
#include "logic/findimmovable.h"
#include "c_utils.h"

#include "luna.h"

#include <lua.hpp>

using namespace Widelands;

/*
 * Helper functions
 */
#define WRAPPED_PROPERTY_SET_INT(object, name) int set_##name(lua_State* L) { \
   object.name = luaL_checkint32(L, -1);                                      \
   return 0;                                                                  \
}
#define WRAPPED_PROPERTY_GET_INT(object, name) int get_##name(lua_State* L) { \
   lua_pushinteger(L, object.name);                                           \
   return 1;                                                                  \
}
#define WRAPPED_PROPERTY_INT(object, name)                                    \
   WRAPPED_PROPERTY_GET_INT(object, name)                                     \
   WRAPPED_PROPERTY_SET_INT(object, name)                                     \


/*
 * Base class for all classes in wl.map
 */
class L_MapModuleClass : public LunaClass {
	public:
		const char * get_modulename() {return "map";}
};


/*
 * Map Object
 */
class L_MapObject : public L_MapModuleClass {
	Object_Ptr * m_ptr;

public:
	LUNA_CLASS_HEAD(L_MapObject);

	L_MapObject() : m_ptr(0) {}
	L_MapObject(Map_Object & mo) {m_ptr = new Object_Ptr(&mo);}
	L_MapObject(lua_State * L) : m_ptr(0) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}
	virtual ~L_MapObject() {
		log("m_ptr: %p, this: %p\n", m_ptr, this);
		if (m_ptr) {
			delete m_ptr;
			m_ptr = 0;
		}
	}

	virtual void __persist(lua_State * L) {
		Map_Map_Object_Saver & mos = *get_mos(L);
		Game & game = *get_game(L);

		uint32_t idx = mos.get_object_file_index(*m_ptr->get(game));
		PERS_UINT32("file_index", idx);
	}
	virtual void __unpersist(lua_State * L) {
		uint32_t idx;
		UNPERS_UINT32("file_index", idx);
		Map_Map_Object_Loader & mol = *get_mol(L);

		m_ptr = new Object_Ptr(&mol.get<Map_Object>(idx));
	}

	/*
	 * Properties
	 */
	int get_serial(lua_State * L) {
		Game & game = *get_game(L);
		lua_pushuint32(L, m_ptr->get(game)->serial());
		return 1;
	}

	/*
	 * Lua Methods
	 */
	int remove(lua_State * L) {
		Game & game = *get_game(L);
		Map_Object * o = m_get(game, L);

		if (!o)
			return 0;

		o->remove(game);
		return 0;
	}

	/*
	 * C Methods
	 */

private:
	Map_Object * m_get(Game & game, lua_State * L) {
		Map_Object * o = m_ptr->get(game);
		if (!o)
			report_error(L, "Object no longer exists!");
		return o;
	}
};
const char L_MapObject::className[] = "MapObject";
const char L_MapObject::parentName[] = "";
const MethodType<L_MapObject> L_MapObject::Methods[] = {
	METHOD(L_MapObject, remove),
	{0, 0},
};
const PropertyType<L_MapObject> L_MapObject::Properties[] = {
	PROP_RO(L_MapObject, serial),
	{0, 0, 0},
};

/*
 * BaseImmovable
 */
class L_BaseImmovable : public L_MapObject {
	OPtr<BaseImmovable>* m_biptr;

public:
	LUNA_CLASS_HEAD(L_BaseImmovable);

	L_BaseImmovable() : m_biptr(0) {}
	L_BaseImmovable(BaseImmovable & mo) : L_MapObject(mo) {
		m_biptr = new OPtr<BaseImmovable>(&mo);
	}
	L_BaseImmovable(lua_State * L) : L_MapObject(L), m_biptr(0) {}
	virtual ~L_BaseImmovable() {
		log("m_biptr: %p, this: %p\n", m_biptr, this);
		if (m_biptr) {
			delete m_biptr;
			m_biptr = 0;
		}
	}

	virtual void __persist(lua_State * L) {
		Map_Map_Object_Saver & mos = *get_mos(L);
		Game & game = *get_game(L);

		uint32_t idx = mos.get_object_file_index(*m_biptr->get(game));
		PERS_UINT32("file_index", idx);

		L_MapObject::__persist(L);
	}
	virtual void __unpersist(lua_State * L) {
		log("Unpersisting object: %p\n", this);
		uint32_t idx;
		UNPERS_UINT32("file_index", idx);
		Map_Map_Object_Loader & mol = *get_mol(L);

		m_biptr = new OPtr<BaseImmovable>(&mol.get<BaseImmovable>(idx));

		L_MapObject::__unpersist(L);
	}

	/*
	 * Properties
	 */
	int get_size(lua_State * L) {
		Game & game = *get_game(L);
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
	int get_name(lua_State * L) {
		Game & game = *get_game(L);
		BaseImmovable * o = m_get(game, L);

		lua_pushstring(L, o->name().c_str());
		return 1;
	}

	/*
	 * Lua Methods
	 */

	/*
	 * C Methods
	 */
private:
	BaseImmovable * m_get(Game & game, lua_State * L) {
		BaseImmovable * o = m_biptr->get(game);
		if (!o)
			report_error(L, "BaseImmovable no longer exists!");
		return o;
	}
};
const char L_BaseImmovable::className[] = "BaseImmovable";
const char L_BaseImmovable::parentName[] = "MapObject";
const MethodType<L_BaseImmovable> L_BaseImmovable::Methods[] = {
	{0, 0},
};
const PropertyType<L_BaseImmovable> L_BaseImmovable::Properties[] = {
	PROP_RO(L_BaseImmovable, size),
	PROP_RO(L_BaseImmovable, name),
	{0, 0, 0},
};

/*
 * Coordinates
 */
class L_Coords : public L_MapModuleClass {
	Coords m_c;

public:
	LUNA_CLASS_HEAD(L_Coords);

	/*
	 * Needed functionality
	 */
	L_Coords() {}
	L_Coords(Coordinate x, Coordinate y) : m_c(x, y) {}
	L_Coords(lua_State * L)
	{
		m_c.x = luaL_checknumber(L, 1);
		m_c.y = luaL_checknumber(L, 2);
	}
	~L_Coords() {}

	virtual void __persist(lua_State * L) {
		PERS_INT32("x", m_c.x); PERS_INT32("y", m_c.y);
	}
	virtual void __unpersist(lua_State * L) {
		UNPERS_INT32("x", m_c.x); UNPERS_INT32("y", m_c.y);
	}

	/*
	 * Properties
	 */
	WRAPPED_PROPERTY_INT(m_c, x);
	WRAPPED_PROPERTY_INT(m_c, y);

	/*
	 * Lua methods
	 */
	/*
	 * C methods
	 */
	inline Coords & coords() {return m_c;}

};
const char L_Coords::className[] = "Coords";
const char L_Coords::parentName[] = "";
const MethodType<L_Coords> L_Coords::Methods[] = {
	{0, 0},
};
const PropertyType<L_Coords> L_Coords::Properties[] = {
	PROP_RW(L_Coords, x),
	PROP_RW(L_Coords, y),
	{0, 0, 0},
};

/*
 * Intern definitions of Lua Functions
 */
/*
 * Create a World immovable object immediately
 *
 * name: name of object to create
 * posx: int, x position
 * posy: int, y position
 *
 */
static int L_create_immovable(lua_State * const L) {
	char const * const objname = luaL_checkstring(L, 1);
	uint32_t     const x       = luaL_checkint32(L, 2);
	uint32_t     const y       = luaL_checkint32(L, 3);

	Game & game = *get_game(L);
	Coords pos(x, y);

	// Check if the map is still free here
	// TODO: this exact code is duplicated in worker.cc
	if (BaseImmovable const * const imm = game.map()[pos].get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(L, "Node is no longer free!");

	int32_t const imm_idx = game.map().world().get_immovable_index(objname);
	if (imm_idx < 0)
		return report_error(L, "Unknown immovable <%s>", objname);

	BaseImmovable & m = game.create_immovable (pos, imm_idx, 0);

	// Send this to lua
	return to_lua<L_BaseImmovable>(L, new L_BaseImmovable(m));
}


/*
 * Find a world immovable
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
	Game & game = *get_game(L);
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
		log("Finding immovables: %i\n", area.radius);
		map.find_reachable_immovables
			(area, &list, cstep, FindImmovableAttribute(attribute));

		log("Found:  %zu\n", list.size());
		if (list.size()) {
			//  TODO If this is called from the console, it will screw network
			//  TODO gaming.
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

	register_class<L_Coords>(L, "map");
	register_class<L_MapObject>(L, "map");

	register_class<L_BaseImmovable>(L, "map", true);
	add_parent<L_BaseImmovable, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table
}

