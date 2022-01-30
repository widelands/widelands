/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "scripting/persistence.h"

#include <memory>

#include "io/fileread.h"
#include "io/filewrite.h"
#include "scripting/eris.h"
#include "scripting/luna_impl.h"

/*
 * ========================================================================
 *                            PRIVATE CLASSES
 * ========================================================================
 */

namespace {

struct LuaReaderHelper {
	std::unique_ptr<char[]> data;
	size_t data_len;
};

int LuaWriter(lua_State* /* L */, const void* write_data, size_t len, void* userdata) {
	FileWrite* fw = static_cast<FileWrite*>(userdata);

	fw->data(write_data, len, FileWrite::Pos::null());
	return 0;
}

const char* LuaReader(lua_State* /* L */, void* userdata, size_t* bytes_read) {
	const LuaReaderHelper& helper = *static_cast<LuaReaderHelper*>(userdata);

	*bytes_read = helper.data_len;
	return helper.data.get();
}

}  // namespace

/**
 * Add one object to the table of objects that should not
 * be touched. Returns true if the object was non nil and
 * therefore stored, false otherwise.
 */
static bool add_object_to_not_persist(lua_State* L, std::string name, uint32_t nidx) {
	// Search for a dot. If one is found, we first have
	// to get the global module.
	std::string::size_type pos = name.find('.');

	if (pos != std::string::npos) {
		std::string table = name.substr(0, pos);
		name = name.substr(pos + 1);

		lua_getglobal(L, table.c_str());  // table object table
		assert(!lua_isnil(L, -1));

		lua_getfield(L, -1, name.c_str());  // table object table function
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			return false;
		}

		lua_pushint32(L, nidx);  // table object table function int
		lua_settable(L, 1);      //  newtable[function] = int
		lua_pop(L, 1);           // pop tabltable
	} else {
		lua_getglobal(L, name.c_str());  // stack: table object value
		if (lua_isnil(L, -1)) {
			lua_pop(L, 1);
			return false;
		}
		lua_pushint32(L, nidx);  // stack: table object value int
		lua_settable(L, 1);      //  table[symbol] = integer
	}
	return true;
}

// Special handling for the upvalues of pairs and ipairs which are iterator
// functions, but always the same and therefor need not be persisted (in fact
// they are c functions, so they can't be persisted all the same)
static void
add_iterator_function_to_not_persist(lua_State* L, const std::string& global, uint32_t idx) {
	lua_getglobal(L, global.c_str());
	lua_newtable(L);
	lua_call(L, 1, 1);  // pairs{}, stack now contains iterator function
	lua_pushuint32(L, idx);
	lua_settable(L, 1);  //  table[function] = integer
}

static bool add_object_to_not_unpersist(lua_State* L, std::string name, uint32_t idx) {
	// S: ... globals

	// Search for a dot. If one is found, we first have
	// to get the global module.
	const std::string::size_type pos = name.find('.');

	if (pos != std::string::npos) {
		const std::string table = name.substr(0, pos);
		name = name.substr(pos + 1);

		lua_getglobal(L, table.c_str());  // S: ... gtables table
		assert(!lua_isnil(L, -1));        // table must already exist!

		lua_pushint32(L, idx);  // S: ... gtables table idx

		lua_getfield(L, -2, name.c_str());  // S: ... gtables table idx function
		assert(!lua_isnil(L, -1));          // function must already exist

		lua_settable(L, -4);  //  gtables[int] = function, S: ... gtables table
		lua_pop(L, 1);        // S: ... gtables
	} else {
		lua_pushint32(L, idx);           // S: ... gtable int
		lua_getglobal(L, name.c_str());  // S: ... gtable int object
		lua_settable(L, -3);             // S: gtable[int] = object
	}
	return true;
}

static void
add_iterator_function_to_not_unpersist(lua_State* L, const std::string& global, uint32_t idx) {
	lua_pushuint32(L, idx);            // S: ... globals idx
	lua_getglobal(L, global.c_str());  // S: ... globals idx "pairs"
	lua_newtable(L);                   // S: ... globals idx "pairs" table
	lua_call(L, 1, 1);                 // calls, pairs {}: ... globals idx iterator_func
	lua_settable(L, -3);               //  globals[int] = function, S: ... globals
}

/*
 * ========================================================================
 *                            PUBLIC INTERFACE
 * ========================================================================
 */

// Those are the globals that will be regenerated (not by the persistence engine),
// e.g. C-functions or automatically populated fields. Changing the ordering here will
// break save game compatibility.
static const char* kPersistentGlobals[] = {"_VERSION",
                                           "assert",
                                           "collectgarbage",
                                           "coroutine",
                                           "debug",
                                           "dofile",
                                           "error",
                                           "gcinfo",
                                           "getfenv",
                                           "getmetatable",
                                           "io",
                                           "ipairs",
                                           "load",
                                           "loadfile",
                                           "loadstring",
                                           "math",
                                           "module",
                                           "newproxy",
                                           "next",
                                           "os",
                                           "package",
                                           "pairs",
                                           "pcall",
                                           "print",
                                           "rawequal",
                                           "rawget",
                                           "rawset",
                                           "rawlen",
                                           "require",
                                           "select",
                                           "setfenv",
                                           "setmetatable",
                                           "table",
                                           "tonumber",
                                           "tostring",
                                           "type",
                                           "unpack",
                                           "wl",
                                           "xpcall",
                                           "string",
                                           "_",
                                           "set_textdomain",
                                           "get_build_id",
                                           "coroutine.yield",
                                           "ngettext",
                                           "include",
                                           "path",
                                           "pgettext",
                                           "ticks",
                                           "push_textdomain",
                                           "pop_textdomain",
                                           "npgettext",
                                           nullptr};

/**
 * Does all the persisting work. Returns the number of bytes
 * written
 */
uint32_t persist_object(lua_State* L, FileWrite& fw, Widelands::MapObjectSaver& mos) {
	assert(lua_gettop(L) == 2);  // S: globals_table object

	// Save a reference to the object saver
	lua_pushlightuserdata(L, &mos);
	lua_setfield(L, LUA_REGISTRYINDEX, "mos");

	// Push objects that should not be touched while persisting into the empty
	// table at stack position 1
	uint32_t i = 1;

	// First, the restore function for __persist.
	lua_pushcfunction(L, &luna_unpersisting_closure);
	lua_pushuint32(L, i++);
	lua_settable(L, 1);

	// Now the iterators functions.
	add_iterator_function_to_not_persist(L, "pairs", i++);
	add_iterator_function_to_not_persist(L, "ipairs", i++);

	// And finally the globals.
	for (int j = 0; kPersistentGlobals[j]; ++j) {
		add_object_to_not_persist(L, kPersistentGlobals[j], i++);
	}

	// The next few lines make eris error messages much more useful, but make
	// eris much slower too. Only enable if you need more debug information.
	lua_pushboolean(L, true);
	eris_set_setting(L, "path", lua_gettop(L));
	lua_pop(L, 1);

	size_t cpos = fw.get_pos();
	eris_dump(L, &LuaWriter, &fw);
	uint32_t nwritten = fw.get_pos() - cpos;

	lua_pop(L, 2);  // pop the object and the table

	// Delete the entry in the registry
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "mos");

	return nwritten;
}

void unpersist_object(lua_State* L, FileRead& fr, Widelands::MapObjectLoader& mol, uint32_t size) {
	assert(lua_gettop(L) == 0);  // S:

	// Save the mol in the registry
	lua_pushlightuserdata(L, &mol);
	lua_setfield(L, LUA_REGISTRYINDEX, "mol");

	// Push objects that should not be loaded
	lua_newtable(L);  // S: table
	uint32_t i = 1;

	// Luna restore (for persistence).
	lua_pushuint32(L, i++);
	lua_pushcfunction(L, &luna_unpersisting_closure);
	lua_settable(L, 1);

	add_iterator_function_to_not_unpersist(L, "pairs", i++);
	add_iterator_function_to_not_unpersist(L, "ipairs", i++);

	for (int j = 0; kPersistentGlobals[j]; ++j) {
		add_object_to_not_unpersist(L, kPersistentGlobals[j], i++);
	}

	LuaReaderHelper helper;
	helper.data_len = size;
	helper.data.reset(new char[size]);
	fr.data(helper.data.get(), size);

	eris_undump(L, &LuaReader, &helper);

	lua_remove(L, -2);  // remove the globals table

	// Delete the entry in the registry
	lua_pushnil(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "mol");

	assert(lua_gettop(L) == 1);
}

/*
 ==========================================================
 PRIVATE METHODS
 ==========================================================
 */
