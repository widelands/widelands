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
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"

#include "pluto.h"
#include "c_utils.h"

#include "persistence.h"

/*
 * ========================================================================
 *                            PRIVATE CLASSES
 * ========================================================================
 */

/**
 * Add one object to the table of objects that should not
 * be touched. Returns true if the object was non nil and
 * therefore stored, false otherwise.
 */
static bool m_add_object_to_not_persist
	(lua_State * L, std::string name, uint32_t nidx)
{
	// Search for a dot. If one is found, we first have
	// to get the global module.
	std::string::size_type pos = name.find('.');

	if (pos != std::string::npos) {
		std::string table = name.substr(0, pos);
		name = name.substr(pos + 1);

		lua_getglobal(L, table.c_str()); // table object table
		assert(!lua_isnil(L, -1));

		lua_getfield(L, -1, name.c_str()); // table object table function
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			return false;
		}

		lua_pushint32(L, nidx); // table object table function int
		lua_settable(L, 1); //  newtable[function] = int
		lua_pop(L, 1); // pop tabltable
	} else {
		lua_getglobal(L, name.c_str()); // stack: table object value
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			return false;
		}
		lua_pushint32(L, nidx); // stack: table object value int
		lua_settable(L, 1); //  table[symbol] = integer
	}
	return true;
}

// Special handling for the upvalues of pairs and ipairs which are iterator
// functions, but always the same and therefor need not be persisted (in fact
// they are c functions, so they can't be persisted all the same)
static void m_add_iterator_function_to_not_persist
	(lua_State * L, std::string global, uint32_t idx)
{
	lua_getglobal(L, global.c_str());
	lua_newtable(L);
	lua_call(L, 1, 1); // pairs{}, stack now contains iterator function
	lua_pushuint32(L, idx);
	lua_settable(L, 1); //  table[function] = integer
}

static bool m_add_object_to_not_unpersist
	(lua_State * L, std::string name, uint32_t idx) {
	// Search for a dot. If one is found, we first have
	// to get the global module.
	std::string::size_type pos = name.find('.');

	if (pos != std::string::npos) {
		std::string table = name.substr(0, pos);
		name = name.substr(pos + 1);

		lua_getglobal(L, table.c_str()); // gtables table
		assert(!lua_isnil(L, -1)); // table must already exist!

		lua_pushint32(L, idx); // gtables table int

		lua_getfield(L, -2, name.c_str()); // gtables table int function
		assert(!lua_isnil(L, -1)); // function must already exist

		lua_settable(L, -4); //  table[int] = function
		lua_pop(L, 1); // pop table
	} else {
		lua_pushint32(L, idx); // stack: table int
		lua_getglobal(L, name.c_str()); // stack: table int value
		lua_settable(L, -3); //  table[int] = object
	}
	return true;
}

static void m_add_iterator_function_to_not_unpersist
	(lua_State * L, std::string global, uint32_t idx)
{
	lua_pushuint32(L, idx); // integer
	lua_getglobal(L, global.c_str());
	lua_newtable(L);
	lua_call(L, 1, 1); // pairs{}, stack now contains iterator function
	lua_settable(L, -3); //  table[int] = function
}

/*
 * ========================================================================
 *                            PUBLIC INTERFACE
 * ========================================================================
 */

static const char * m_persistent_globals[] = {
	"_VERSION", "assert", "collectgarbage", "coroutine", "debug",
	"dofile", "error", "gcinfo", "getfenv", "getmetatable", "io", "ipairs",
	"load", "loadfile", "loadstring", "math", "module", "newproxy", "next",
	"os", "package", "pairs", "pcall", "print", "rawequal",
	"rawget", "rawset", "require", "select", "setfenv", "setmetatable",
	"table", "tonumber", "tostring", "type", "unpack", "wl", "xpcall",
	"string", "use", "_", "set_textdomain", "coroutine.yield", 0
};

/**
 * Does all the persisting work. Returns the number of bytes
 * written
 */
uint32_t persist_object
	(lua_State * L,
	 Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos)
{
	assert(lua_gettop(L) == 2); // table object

	// Save a reference to the object saver
	lua_pushlightuserdata(L, &mos);
	lua_setfield(L, LUA_REGISTRYINDEX, "mos");

	// Push objects that should not be touched while persisting into the empty
	// table at stack position 1
	uint32_t i = 0;
	for (i = 0; m_persistent_globals[i]; i++)
		m_add_object_to_not_persist(L, m_persistent_globals[i], i + 1);

	++i;
	m_add_iterator_function_to_not_persist(L, "pairs", i++);
	m_add_iterator_function_to_not_persist(L, "ipairs", i++);


	size_t cpos = fw.GetPos();
	pluto_persist(L, fw);
	uint32_t nwritten = fw.GetPos() - cpos;

	log("nwritten: %u\n", nwritten);

	lua_pop(L, 2); // pop the object and the table

	// Delete the entry in the registry
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "mos");

	return nwritten;
}

/**
 * Does all the unpersisting work. Returns the number of bytes
 * written
 */
uint32_t unpersist_object
	(lua_State * L,
	 Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	// Save the mol in the registry
	lua_pushlightuserdata(L, &mol);
	lua_setfield(L, LUA_REGISTRYINDEX, "mol");

	// Push objects that should not be loaded
	lua_newtable(L);
	uint32_t i = 0;
	for (i = 0; m_persistent_globals[i]; i++)
		m_add_object_to_not_unpersist(L, m_persistent_globals[i], i + 1);

	++i;
	m_add_iterator_function_to_not_unpersist(L, "pairs", i++);
	m_add_iterator_function_to_not_unpersist(L, "ipairs", i++);

	pluto_unpersist(L, fr);
	lua_remove(L, -2); // remove the globals table

	// Delete the entry in the registry
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "mol");

	assert(lua_gettop(L) == 1);

	return 1;
}

/*
 ==========================================================
 PRIVATE METHODS
 ==========================================================
 */
