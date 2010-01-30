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

#include "pluto/pluto.h"
#include "c_utils.h"

#include "persistence.h"

/*
 * ========================================================================
 *                            PRIVATE CLASSES
 * ========================================================================
 */

struct DataWriter {
	uint32_t written;
	Widelands::FileWrite & fw;

	DataWriter(Widelands::FileWrite & gfw) : written(0), fw(gfw) {}
};

int write_func(lua_State *, const void * p, size_t data, void * ud) {
	DataWriter * dw = static_cast<DataWriter *>(ud);

	dw->fw.Data(p, data, Widelands::FileWrite::Pos::Null());
	dw->written += data;

	return data;
}

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
		log("Pushing %s:%s!\n", table.c_str(), name.c_str());

		lua_getglobal(L, table.c_str()); // table object table
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			return false;
		}
		lua_getfield(L, -1, name.c_str()); // table object table function
		if (lua_isnil(L, -1)) {
			lua_pop(L, 2);
			return false;
		}
		lua_pushint32(L, nidx); // table object table function int
		lua_settable(L, 1); //  newtable[function] = int
		lua_pop(L, 1); // pop tabltable
		return true;
	} else {
		log("Persisting: %s\n", name.c_str());
		lua_getglobal(L, name.c_str()); // stack: table object value
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			return false;
		} else {
			log("  got value: %i\n", nidx);
			lua_pushint32(L, nidx); // stack: table object value int
			lua_settable(L, 1); //  table[symbol] = integer
			return true;
		}
	}
}


struct DataReader_L {
	uint32_t size;
	Widelands::FileRead & fr;

	DataReader_L(uint32_t gsize, Widelands::FileRead & gfr) :
		size(gsize), fr(gfr) {}
};

const char * read_func_L(lua_State *, void * ud, size_t * sz) {
	DataReader_L * dr = static_cast<DataReader_L *>(ud);

	*sz = dr->size;
	return static_cast<const char *>(dr->fr.Data(dr->size));
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

		lua_settable(L, -3); //  table[int] = function
		lua_pop(L, 1); // pop table
	} else {
		log("Do not unpersist: %s\n", name.c_str());
		lua_pushint32(L, idx); // stack: table int
		lua_getglobal(L, name.c_str()); // stack: table int value
		log("  got value: %i\n", idx);
		lua_settable(L, -3); //  table[int] = object
	}
	return true;
}

/*
 * ========================================================================
 *                            PUBLIC INTERFACE
 * ========================================================================
 */

/**
 * Does all the persisting work. Returns the number of bytes
 * written
 */
uint32_t persist_object
	(lua_State * L, const char ** globals,
	 Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos)
{
	assert(lua_gettop(L) == 2); // table object

	// Save a reference to the object saver
	lua_pushlightuserdata(L, &mos);
	lua_setfield(L, LUA_REGISTRYINDEX, "mos");

	// Push objects that should not be touched while persisting into the empty
	// table at stack position 1
	for (uint32_t i = 0, idx = 1; globals[i]; i++)
		if (m_add_object_to_not_persist(L, globals[i], idx))
			++idx;

	log("before calling pluto_persist: %i\n", lua_gettop(L));

	DataWriter dw(fw);
	pluto_persist(L, &write_func, &dw);
	lua_pop(L, 2); // pop the object and the table

	log("After pluto_persist!\n");
	log("Pickled %i bytes. Stack size: %i\n", dw.written, lua_gettop(L));

	// Delete the entry in the registry
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "mos");

	return dw.written;
}

/**
 * Does all the unpersisting work. Returns the number of bytes
 * written
 */
uint32_t unpersist_object
	(lua_State * L, const char ** globals,
	 Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	// Save the mol in the registry
	lua_pushlightuserdata(L, &mol);
	lua_setfield(L, LUA_REGISTRYINDEX, "mol");

	// Push objects that should not be loaded
	lua_newtable(L);
	for (uint32_t i = 0; globals[i]; i++)
		m_add_object_to_not_unpersist(L, globals[i], i + 1);

	log("Pushed all globals\n");

	DataReader_L rd(size, fr);

	pluto_unpersist(L, &read_func_L, &rd);
	log("Lua unpersisting done!");

	lua_remove(L, -2); // remove the globals table

	// Delete the entry in the registry
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "mol");

	assert(lua_gettop(L) == 1);

	log("Done with unpickling!\n");


	log("Unpickled %i bytes. Stack size: %i\n", size, lua_gettop(L));

	return 1;
}

/*
 ==========================================================
 PRIVATE METHODS
 ==========================================================
 */
