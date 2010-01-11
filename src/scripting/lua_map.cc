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


extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}
using namespace Widelands;

/*
 * Helper functions
 */

#define luaL_checkint32(L, n)  static_cast<int32_t>(luaL_checkinteger(L, (n)))

#define WRAPPED_PROPERTY_SET_INT(object, name) int set_##name(lua_State* L) { \
	object.name = luaL_checkint32(L, -1); \
	return 0; \
}
#define WRAPPED_PROPERTY_GET_INT(object, name) int get_##name(lua_State* L) { \
	lua_pushinteger(L, object.name); \
	return 1; \
}
#define WRAPPED_PROPERTY_INT(object, name) \
	WRAPPED_PROPERTY_GET_INT(object, name) \
	WRAPPED_PROPERTY_SET_INT(object, name)

#define PROP_RO(klass, name) {#name, &klass::get_##name, 0}
#define PROP_RW(klass, name) {#name, &klass::get_##name, &klass::set_##name}
#define METHOD(klass, name) {#name, &klass::name}

/*
 * Map Object
 */
// class L_MapObject {
//         Map_Object& mo;
//
// public:
//         LUA_WRAPPED_CALL_R_INT(serial, mo);
//
//         static const char className[];
//         static Luna<L_Coords>::RegType methods[];
// }
// const char L_MapObject::className[] = "MapObject";
// Luna<L_Coords>::RegType L_MapObject::methods[] = {
//         method(L_MapObject, serial),
//         {0, 0},
// };
//
/*
 * Coordinates
 */
class L_Coords {
	Coords m_c;

public:
	static const char className[];
	static const Luna<L_Coords>::FunctionType Functions[];
	static const Luna<L_Coords>::PropertyType Properties[];


	L_Coords(lua_State * L)
	{
		m_c.x = luaL_checknumber(L, 1);
		m_c.y = luaL_checknumber(L, 2);
	}
	~L_Coords() {}

	/*
	 * Properties
	 */
	WRAPPED_PROPERTY_INT(m_c, x);
	WRAPPED_PROPERTY_INT(m_c, y);

	/*
	 * Lua functions
	 */

	/*
	 * C Functions
	 */
	inline Coords & coords() {return m_c;}
};
const char L_Coords::className[] = "Coords";
const Luna <L_Coords>::FunctionType L_Coords::Functions[] = {
	{0, 0},
};
const Luna <L_Coords>::PropertyType L_Coords::Properties[] = {
	PROP_RW(L_Coords, x),
	PROP_RW(L_Coords, y),
	{0, 0, 0},
};
// LUAMODULE wl.map


/*
 * Intern definitions of Lua Functions
 */
/*
 * Create a World immovable object immediately
 *
 * name: name ob object to create
 * posx: int, x position
 * posy: int, y position
 *
 */
static int L_create_immovable(lua_State * const l) {
	char const * const objname = luaL_checkstring(l, 1);
	uint32_t     const x       = luaL_checkint32(l, 2);
	uint32_t     const y       = luaL_checkint32(l, 3);

	Game & game = *get_game(l);
	Coords pos(x, y);

	// Check if the map is still free here
	// TODO: this exact code is duplicated in worker.cc
	if (BaseImmovable const * const imm = game.map()[pos].get_immovable())
		if (imm->get_size() >= BaseImmovable::SMALL)
			return report_error(l, "Field is no longer free!");

	int32_t const imm_idx = game.map().world().get_immovable_index(objname);
	if (imm_idx < 0)
		return report_error(l, "Unknown immovable <%s>", objname);

	Map_Object & m = game.create_immovable (pos, imm_idx, 0);

	lua_pushlightuserdata(l, static_cast<void * >(&m));

	return 1;
}


/*
 * Find a world immovable
 *
 * x, y, radius - position to search for
 * attrib - attribute to use
 *
 * Returns: x, y position of object
 */
static int L_find_immovable(lua_State * const l) {
	uint32_t     const x       = luaL_checkint32(l, 1);
	uint32_t     const y       = luaL_checkint32(l, 2);
	uint32_t     const radius  = luaL_checkint32(l, 3);
	const char *  const attrib  = luaL_checkstring(l, 4);

	Coords pos(x, y);
	Game & game = *get_game(l);
	Map & map = game.map();

	Area<FCoords> area (map.get_fcoords(pos), 0);
	CheckStepWalkOn cstep(MOVECAPS_WALK, false);
	int attribute = Map_Object_Descr::get_attribute_id(attrib);

	for (;; ++area.radius) {
		if (radius < area.radius)
			return report_error(l, "No suitable object in radius %i", radius);

			std::vector<ImmovableFound> list;
                        // if (action.iparam2 < 0)
                        //         map.find_reachable_immovables
                        //                 (area, &list, cstep);
                        // else
			log("Finding immovables: %i\n", area.radius);
			map.find_reachable_immovables
				(area, &list, cstep, FindImmovableAttribute(attribute));

			log("Found:  %zu\n", list.size());
			if (list.size()) {
				// TODO: if this is called from the console, it will screw
				// network gaming
				Coords & rv = list[game.logic_rand() % list.size()].coords;
				lua_pushinteger(l, rv.x);
				lua_pushinteger(l, rv.y);
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

	Luna<L_Coords>::register_class(L, "map");

}
