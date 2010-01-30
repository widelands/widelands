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

#include <string>
#include <stdexcept>

#include "log.h"

#include "lua_debug.h"
#include "lua_game.h"
#include "lua_map.h"
#include "coroutine_impl.h"
#include "c_utils.h"

#include "scripting.h"

// TODO: document this whole scripting stuff: changes in pluto, pickling,
// TODO:   unpickling, Luna & Luna changes, mos & mol pickling
// TODO: make pickling independet of OS by using widelands Stream*
// TODO: remove this include
#include "pluto/pluto.h"

// TODO: get rid of LuaCmd. Only LuaFunction should be kept alife

/*
============================================
       Lua Interface
============================================
*/
class LuaInterface_Impl : public LuaInterface {
	std::string m_last_error;
	std::map<std::string, ScriptContainer> m_scripts;
	lua_State * m_L;

	/*
	 * Private functions
	 */
	private:
		int m_check_for_errors(int);

	public:
		LuaInterface_Impl(Widelands::Editor_Game_Base *);
		virtual ~LuaInterface_Impl();

		virtual void interpret_string(std::string);
		virtual void interpret_file(std::string);
		virtual std::string const & get_last_error() const {return m_last_error;}

		virtual void register_script(std::string, std::string, std::string);
		virtual ScriptContainer & get_scripts_for(std::string ns) {
			return m_scripts[ns];
		}

		virtual void run_script(std::string, std::string);

		virtual LuaCoroutine * read_coroutine
			(Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);
		virtual uint32_t write_coroutine
			(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &,
			 LuaCoroutine *);
		virtual void read_global_env
			(Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);
		virtual uint32_t write_global_env
			(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &);
};

/*************************
 * Private functions
 *************************/
int LuaInterface_Impl::m_check_for_errors(int rv) {
	if (rv)
		throw LuaError(luaL_checkstring(m_L, -1));
	return rv;
}

/*************************
 * Public functions
 *************************/
LuaInterface_Impl::LuaInterface_Impl
	(Widelands::Editor_Game_Base * const egbase) : m_last_error("") {
	m_L = lua_open();

	// Open the lua libraries
#ifdef DEBUG
	static const luaL_Reg lualibs[] = {
		{"", luaopen_base},
		{LUA_LOADLIBNAME, luaopen_package},
		{LUA_TABLIBNAME, luaopen_table},
		{LUA_IOLIBNAME, luaopen_io},
		{LUA_OSLIBNAME, luaopen_os},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		{LUA_DBLIBNAME, luaopen_debug},
		{0,               0}
	};
#else
	static const luaL_Reg lualibs[] = {
		{"", luaopen_base},
		{LUA_TABLIBNAME, luaopen_table},
		{LUA_STRLIBNAME, luaopen_string},
		{LUA_MATHLIBNAME, luaopen_math},
		{0,               0}
	};
#endif
	const luaL_Reg * lib = lualibs;
	for (; lib->func; lib++) {
		lua_pushcfunction(m_L, lib->func);
		lua_pushstring(m_L, lib->name);
		lua_call(m_L, 1, 0);
	}

	// Now our own
	luaopen_wldebug(m_L);
	luaopen_wlmap(m_L);
	luaopen_wlgame(m_L);

	// Push the game onto the stack
	lua_pushstring(m_L, "egbase");
	lua_pushlightuserdata(m_L, static_cast<void *>(egbase));
	lua_settable(m_L, LUA_REGISTRYINDEX);
	// TODO: this should only be pushed if this is really a game!
	lua_pushstring(m_L, "game");
	lua_pushlightuserdata(m_L, static_cast<void *>(egbase));
	lua_settable(m_L, LUA_REGISTRYINDEX);

	// TODO: remove this again
	luaopen_pluto(m_L);
	lua_pop(m_L, 1);
	// End of removing
}

LuaInterface_Impl::~LuaInterface_Impl() {
	lua_close(m_L);
}

LuaCoroutine * LuaInterface_Impl::read_coroutine
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	LuaCoroutine_Impl * rv = new LuaCoroutine_Impl(0);

	rv->read(m_L, fr, mol, size);

	return rv;
}

uint32_t LuaInterface_Impl::write_coroutine
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos,
	 LuaCoroutine * cr)
{
	// we do not want to make the write function public by adding
	// it to the interface of LuaCoroutine. Therefore, we make a cast
	// to the Implementation function here.
	return dynamic_cast<LuaCoroutine_Impl *>(cr)->write(m_L, fw, mos);
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

// TODO: most of this function is also duplicate
void LuaInterface_Impl::read_global_env
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	int n = lua_gettop(m_L);
	log("\n\nReading Environment: %i\n", n);

	// Save the mol in the registry
	lua_pushlightuserdata(m_L, &mol);
	lua_setfield(m_L, LUA_REGISTRYINDEX, "mol");

	// Push all the stuff that should not be regenerated
	lua_newtable(m_L);
	static const char * globals[] = {
		"_VERSION", "assert", "collectgarbage", "coroutine", "debug",
		"dofile", "error", "gcinfo", "getfenv", "getmetatable", "io", "ipairs",
		"load", "loadfile", "loadstring", "math", "module", "newproxy", "next",
		"os", "package", "pairs", "pcall", "pluto", "print", "rawequal",
		"rawget", "rawset", "require", "select", "setfenv", "setmetatable",
		"table", "tonumber", "tostring", "type", "unpack", "wl", "xpcall",
		"string", 0
	};
	uint32_t ncounter = 1;
	for (uint32_t i = 0; globals[i]; i++)
	{
		log("Do not unpersist: %s\n", globals[i]);
		lua_pushint32(m_L, ncounter); // stack: table int
		lua_getglobal(m_L, globals[i]); // stack: table int value
		log("  got value: %i\n", ncounter);
		lua_settable(m_L, -3); //  table[int] = object
		ncounter++;
	}
	lua_getglobal(m_L, "coroutine");
	lua_getfield(m_L, -1, "create");
	lua_pushint32(m_L, ncounter++); // stack: newtable coroutine yield integer
	lua_settable(m_L, -4); //  newtable[yield] = integer
	lua_pop(m_L, 1); // pop coroutine

	log("Pushed all globals\n");

	DataReader_L rd(size, fr);

	pluto_unpersist(m_L, &read_func_L, &rd);
	log("Lua unpersisting done!");

	// luaL_checktype(m_L, -1, LUA_TTABLE);
	lua_pop(m_L, 2); // pop the thread & the table

	log("Done with unpickling!\n");

	// Delete the entry in the registry
	lua_pushnil(m_L);
	lua_setfield(m_L, LUA_REGISTRYINDEX, "mol");

	log("Unpickled %i bytes. Stack size: %i\n", size, lua_gettop(m_L));
}


struct DataWriter_L {
	uint32_t written;
	Widelands::FileWrite & fw;

	DataWriter_L(Widelands::FileWrite & gfw) : written(0), fw(gfw) {}
};

int write_func_L(lua_State *, const void * p, size_t data, void * ud) {
	DataWriter_L * dw = static_cast<DataWriter_L *>(ud);

	dw->fw.Data(p, data, Widelands::FileWrite::Pos::Null());
	dw->written += data;

	return data;
}

// TODO: this function + LuaCoroutine write should be factored out
// into a class that cares for persistence
uint32_t LuaInterface_Impl::write_global_env
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos)
{
	// Save a reference to the object saver
	lua_pushlightuserdata(m_L, &mos);
	lua_setfield(m_L, LUA_REGISTRYINDEX, "mos");

	// Push all globals that should not be persisted. If any of those
	// do not exist, do not push the value.
	lua_newtable(m_L);
	static const char * globals[] = {
		"_VERSION", "assert", "collectgarbage", "coroutine", "debug",
		"dofile", "error", "gcinfo", "getfenv", "getmetatable", "io", "ipairs",
		"load", "loadfile", "loadstring", "math", "module", "newproxy", "next",
		"os", "package", "pairs", "pcall", "pluto", "print", "rawequal",
		"rawget", "rawset", "require", "select", "setfenv", "setmetatable",
		"table", "tonumber", "tostring", "type", "unpack", "wl", "xpcall",
		"string", 0
	};
	uint32_t ncounter = 1;
	for (uint32_t i = 0; globals[i]; i++)
	{
		log("Persisting: %s\n", globals[i]);
		lua_getglobal(m_L, globals[i]); // stack: table value
		if (lua_isnil(m_L, -1)) {
			lua_pop(m_L, 1);
		} else {
			log("  got value: %i\n", ncounter);
			lua_pushint32(m_L, ncounter++); // stack: table value int
			lua_settable(m_L, -3); //  table[symbol] = integer
		}
	}
	lua_getglobal(m_L, "coroutine");
	lua_pushint32(m_L, ncounter++);
	lua_getfield(m_L, -2, "create");// stack: table coroutine integer create
	lua_settable(m_L, -4); //  newtable[integer] = create
	lua_pop(m_L, 1); // pop coroutine
	log("Pushed all globals\n");

	// Now, we just push our globals dict
	lua_pushvalue(m_L, LUA_GLOBALSINDEX);

	log("after pushing object: %i\n", lua_gettop(m_L));

	// fw.Unsigned32(0xff);
	DataWriter_L dw(fw);
	pluto_persist(m_L, &write_func_L, &dw);
	lua_pop(m_L, 2); // pop the two tables

	log("After pluto_persist!\n");
	log("Pickled %i bytes. Stack size: %i\n", dw.written, lua_gettop(m_L));

	// Delete the entry in the registry
	lua_pushnil(m_L);
	lua_setfield(m_L, LUA_REGISTRYINDEX, "mos");

	return dw.written;
}


void LuaInterface_Impl::register_script
	(std::string ns, std::string name, std::string content)
{
	m_scripts[ns][name] = content;
}

void LuaInterface_Impl::interpret_string(std::string cmd) {
	int rv = luaL_dostring(m_L, cmd.c_str());
	m_check_for_errors(rv);
}

void LuaInterface_Impl::interpret_file(std::string filename) {
	log("In LuaInterface::interpret_file:\n");
	log(" <%s>\n", filename.c_str());

	int rv = luaL_dofile(m_L, filename.c_str());
	m_check_for_errors(rv);
}

void LuaInterface_Impl::run_script(std::string ns, std::string name) {
	if
		((m_scripts.find(ns) == m_scripts.end()) ||
		 (m_scripts[ns].find(name) == m_scripts[ns].end()))
		throw LuaScriptNotExistingError(name);

	return interpret_string(m_scripts[ns][name]);
}

/*
============================================
       Global functions
============================================
*/
/*
 * Factory Function
 */
LuaInterface * create_lua_interface(Widelands::Editor_Game_Base * g) {
	return new LuaInterface_Impl(g);
}

