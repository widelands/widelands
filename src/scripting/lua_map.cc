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
 * Map Object
 */
#if 0
class L_MapObject {
	Object_Ptr m_ptr;

public:
	LUNA_CLASS_HEAD(L_MapObject);

	L_MapObject() {}
	L_MapObject(Map_Object & mo) : m_ptr(&mo) {
	}
	L_MapObject(lua_State * L) : m_ptr(0) {
		report_error(L, "Cannot instantiate a '%s' directly!", className);
	}

	/*
	 * Properties
	 */
	int get_serial(lua_State * L) {
		lua_pushinteger(L, m_ptr.serial());
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
		Map_Object * o = m_ptr.get(game);
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
	OPtr<BaseImmovable> m_biptr;
public:
	LUNA_CLASS_HEAD(L_BaseImmovable);

	L_BaseImmovable() {}
	L_BaseImmovable(BaseImmovable & mo) : L_MapObject(mo), m_biptr(&mo) {
	}
	L_BaseImmovable(lua_State * L) : L_MapObject(L) {
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
		BaseImmovable * o = m_biptr.get(game);
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
#endif

/*
 * Coordinates
 */
class L_Coords : public LunaClass {
	Coords m_c;

public:
	const char * get_modulename() {return "map";}

	LUNA_CLASS_HEAD(L_Coords);

	L_Coords() {
		log("Constructed!\n");
	}
	L_Coords(lua_State * L)
	{
		m_c.x = luaL_checknumber(L, 1);
		m_c.y = luaL_checknumber(L, 2);
	}
	L_Coords(Coordinate x, Coordinate y) : m_c(x, y) {}
	~L_Coords() {}

	/*
	 * Properties
	 */
	WRAPPED_PROPERTY_INT(m_c, x);
	WRAPPED_PROPERTY_INT(m_c, y);

	/*
	 * Lua methods
	 */
	virtual void __persist(lua_State * L) {
		log("In persist!");
		lua_pushint32(L, m_c.x);
		lua_setfield(L, -2, "x");

		lua_pushint32(L, m_c.y);
		lua_setfield(L, -2, "y");
		log("Completly done!");
	}
	virtual void __unpersist(lua_State * L) {
		log("In unpersist!\n");

		return;

		lua_getfield(L, -1, "x");
		uint32_t x = luaL_checkint32(L, -1);
		lua_pop(L, 1);
		log("x: %i\n", x);

		lua_getfield(L, -1, "y");
		uint32_t y = luaL_checkint32(L, -1);
		lua_pop(L, 1);
		log("y: %i\n", y);

		lua_pop(L, 1);

		to_lua<L_Coords>(L, this);
	}

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

// TODO: dirty test function
int restore_wl_object(lua_State * L) {
	lua_getfield(L, -1, "module");
	std::string module = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	lua_getfield(L, -1, "class");
	std::string klass = luaL_checkstring(L, -1);
	lua_pop(L, 1);

	// get this classes instantiator
	lua_getglobal(L, "wl"); // table wl
	log("module: %s\n", module.c_str());
	lua_getfield(L, -1, module.c_str()); // table wl module
	std::string instantiator = "__" + klass;
	log("Instantiator: %s\n", instantiator.c_str());
	lua_getfield(L, -1, instantiator.c_str()); // table wl module func

	// TODO: check if this is a function

	log("Now calling: %i\n", lua_gettop(L));
	lua_call(L, 0, 1);
	lua_pushint32(L, 0); // table wl module lua_obj int
	log("Calling done: %i\n", lua_gettop(L));
	lua_gettable(L, -2); // table wl module lua_obj obj
	log("Gettable call done!\n");

	L_Coords ** o = static_cast<L_Coords ** >(lua_touserdata(L, - 1));
	log("To userdata done: %p\n", o);
	lua_pop(L, 4);

	log("Before unpersist: %i\n", lua_gettop(L));
	(*o)->__unpersist(L);
	return 1;
}

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
#if 0
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
#endif
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
#if 0
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
#endif
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
#if 0
	register_class<L_MapObject>(L, "map");

	register_class<L_BaseImmovable>(L, "map", true);
	add_parent<L_BaseImmovable, L_MapObject>(L);
	lua_pop(L, 1); // Pop the meta table
#endif
}

