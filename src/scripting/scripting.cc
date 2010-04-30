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
#include "io/filesystem/layered_filesystem.h"

#include "c_utils.h"
#include "coroutine_impl.h"
#include "lua_debug.h"
#include "lua_editor.h"
#include "lua_game.h"
#include "lua_globals.h"
#include "lua_map.h"
#include "lua_ui.h"
#include "persistence.h"

#include "scripting.h"

#ifdef _MSC_VER
#include <ctype.h> // for tolower
#endif

// TODO: add wl.editor to documentation
// TODO: Roads are not rendered in the Editor. Fixe mark_map, post and presplit
// TODO: remove wl.debug
// TODO: get_game should throw an error if not called in the game.

/*
============================================
       Lua Table
============================================
*/
class LuaTable_Impl : virtual public LuaTable {
	lua_State * m_L;

public:
		LuaTable_Impl(lua_State * L) : m_L(L) {}

		virtual ~LuaTable_Impl() {
			lua_pop(m_L, 1);
		}

		virtual std::string get_string(std::string s) {
			lua_getfield(m_L, -1, s.c_str());
			if (not lua_isstring(m_L, -1)) {
				lua_pop(m_L, 1);
				throw LuaError
					(s + " is not a field in the table returned by the last "
					 "script or not a string");
			}
			std::string rv = lua_tostring(m_L, -1);
			lua_pop(m_L, 1);

			return rv;
		}

	virtual LuaCoroutine * get_coroutine(std::string s) {
		lua_getfield(m_L, -1, s.c_str());

		if (lua_isfunction(m_L, -1)) {
			// Oh well, a function, not a coroutine. Let's turn it into one
			lua_State * t = lua_newthread(m_L);
			lua_pop(m_L, 1); // Immediately remove this thread again

			lua_xmove(m_L, t, 1); // Move function to coroutine
			lua_pushthread(t); // Now, move thread object back
			lua_xmove(t, m_L, 1);
		}

		if (not lua_isthread(m_L, -1)) {
			lua_pop(m_L, 1);
			throw LuaError
				(s + "is not a field in the table returned by the last script "
				 "or not a function");
		}
		LuaCoroutine * cr = new LuaCoroutine_Impl(luaL_checkthread(m_L, -1));
		lua_pop(m_L, 1); // Remove coroutine from stack
		return cr;
	}
};
/*
============================================
       Lua Interface
============================================
*/
struct LuaInterface_Impl : public virtual LuaInterface {
	std::string m_last_error;
	std::map<std::string, ScriptContainer> m_scripts;

protected:
	lua_State * m_L;

	/*
	 * Private functions
	 */
	private:
		int m_check_for_errors(int);
		std::string m_register_script
			(FileSystem & fs, std::string path, std::string ns);
		bool m_filename_to_short(const std::string &);
		bool m_is_lua_file(const std::string &);

	public:
		LuaInterface_Impl();
		virtual ~LuaInterface_Impl();

		virtual void interpret_string(std::string);
		virtual std::string const & get_last_error() const {return m_last_error;}

		virtual void register_scripts
			(FileSystem &, std::string, std::string = "scripting");
		virtual ScriptContainer & get_scripts_for(std::string ns) {
			return m_scripts[ns];
		}

		virtual boost::shared_ptr<LuaTable> run_script(std::string, std::string);
		virtual boost::shared_ptr<LuaTable> run_script
			(FileSystem &, std::string, std::string);
};


/*************************
 * Private functions
 *************************/
int LuaInterface_Impl::m_check_for_errors(int rv) {
	if (rv) {
		std::string err = luaL_checkstring(m_L, -1);
		lua_pop(m_L, 1);
		throw LuaError(err);
	}
	return rv;
}

std::string LuaInterface_Impl::m_register_script
	(FileSystem & fs, std::string path, std::string ns)
{
		size_t length;
		std::string data(static_cast<char *>(fs.Load(path, length)));
		std::string name = path.substr(0, path.size() - 4); // strips '.lua'

		for (size_t i = name.size() - 1; i; i--) {
			if (name[i] == '/' or name[i] == '\\') {
				name = name.substr(i + 1, name.size());
				break;
			}
		}

		log("Registering script: (%s,%s)\n", ns.c_str(), name.c_str());
		m_scripts[ns][name] = data;

		return name;
}

bool LuaInterface_Impl::m_filename_to_short(const std::string & s) {
	return s.size() < 4;
}
bool LuaInterface_Impl::m_is_lua_file(const std::string & s) {
	std::string ext = s.substr(s.size() - 4, s.size());
	// std::transform fails on older system, therefore we use an explicit loop
	for (uint32_t i = 0; i < ext.size(); i++) {
#ifndef _MSC_VER
		ext[i] = std::tolower(ext[i]);
#else
		ext[i] = tolower(ext[i]);
#endif
	}
	return (ext == ".lua");
}

/*************************
 * Public functions
 *************************/
LuaInterface_Impl::LuaInterface_Impl() : m_last_error("") {
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
		{LUA_DBLIBNAME, luaopen_debug}, // needed for testsuite
		{0,               0}
	};
#endif
	const luaL_Reg * lib = lualibs;
	for (; lib->func; lib++) {
		lua_pushcfunction(m_L, lib->func);
		lua_pushstring(m_L, lib->name);
		lua_call(m_L, 1, 0);
	}

	// Push the instance of this class into the registry
	lua_pushlightuserdata(m_L, reinterpret_cast<void*>(this));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "lua_interface");

	// Now our own
	luaopen_globals(m_L);

	register_scripts(*g_fs, "aux");
}

LuaInterface_Impl::~LuaInterface_Impl() {
	lua_close(m_L);
}

void LuaInterface_Impl::register_scripts
	(FileSystem & fs, std::string ns, std::string subdir)
{
	filenameset_t scripting_files;

	// Theoretically, we should be able to use fs.FindFiles(*.lua) here,
	// but since FindFiles doesn't support Globbing in Zips and most
	// saved maps/games are zip, we have to work around this issue.
	fs.FindFiles(subdir, "*", &scripting_files);

	for
		(filenameset_t::iterator i = scripting_files.begin();
		 i != scripting_files.end(); i++)
	{
		if (m_filename_to_short(*i) or not m_is_lua_file(*i))
			continue;

		m_register_script(fs, *i, ns);
	}
}

void LuaInterface_Impl::interpret_string(std::string cmd) {
	int rv = luaL_dostring(m_L, cmd.c_str());
	m_check_for_errors(rv);
}

boost::shared_ptr<LuaTable> LuaInterface_Impl::run_script
	(FileSystem & fs, std::string path, std::string ns)
{
	bool delete_ns = false;
	if (not m_scripts.count(ns))
		delete_ns = true;

	std::string name = m_register_script(fs, path, ns);

	boost::shared_ptr<LuaTable> rv = run_script(ns, name);

	if (delete_ns)
		m_scripts.erase(ns);
	else
		m_scripts[ns].erase(name);

	return rv;
}

boost::shared_ptr<LuaTable> LuaInterface_Impl::run_script
	(std::string ns, std::string name)
{
	if
		((m_scripts.find(ns) == m_scripts.end()) ||
		 (m_scripts[ns].find(name) == m_scripts[ns].end()))
		throw LuaScriptNotExistingError(ns, name);

	const std::string & s = m_scripts[ns][name];

	m_check_for_errors
		(luaL_loadbuffer(m_L, s.c_str(), s.size(), (ns + ":" + name).c_str()) ||
		 lua_pcall(m_L, 0, 1, 0)
	);

	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 1); // No return value from script
		lua_newtable(m_L); // Push an empty table
	}
	if (not lua_istable(m_L, -1))
		throw LuaError("Script did not return a table!");
	return boost::shared_ptr<LuaTable>(new LuaTable_Impl(m_L));
}

/*
 * ===========================
 * LuaEditorGameBaseInterface_Impl
 * ===========================
 */
struct LuaEditorGameBaseInterface_Impl : public LuaInterface_Impl
{
	LuaEditorGameBaseInterface_Impl(Widelands::Editor_Game_Base * g);
	virtual ~LuaEditorGameBaseInterface_Impl() {}
};

LuaEditorGameBaseInterface_Impl::LuaEditorGameBaseInterface_Impl
	(Widelands::Editor_Game_Base * g) :
LuaInterface()
{
	luaopen_wldebug(m_L);
	luaopen_wlmap(m_L);
	luaopen_wlgame(m_L);
	luaopen_wlui(m_L);

	// Push the editor game base
	lua_pushlightuserdata(m_L, static_cast<void *>(g));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "egbase");
}


/*
 * ===========================
 * LuaEditorInterface_Impl
 * ===========================
 */
struct LuaEditorInterface_Impl : public LuaEditorGameBaseInterface_Impl
{
	LuaEditorInterface_Impl(Widelands::Editor_Game_Base * g);
	virtual ~LuaEditorInterface_Impl() {}
};

LuaEditorInterface_Impl::LuaEditorInterface_Impl
	(Widelands::Editor_Game_Base * g) :
LuaEditorGameBaseInterface_Impl(g)
{
	luaopen_wleditor(m_L);
}


/*
 * ===========================
 * LuaGameInterface
 * ===========================
 */
struct LuaGameInterface_Impl : public LuaEditorGameBaseInterface_Impl,
	public virtual LuaGameInterface
{
	LuaGameInterface_Impl(Widelands::Game * g);
	virtual ~LuaGameInterface_Impl() {}

	virtual LuaCoroutine* read_coroutine
		(Widelands::FileRead &, Widelands::Map_Map_Object_Loader&,
		 uint32_t);
	virtual uint32_t write_coroutine
		(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver&,
		 LuaCoroutine *);

	virtual void read_global_env
		(Widelands::FileRead &, Widelands::Map_Map_Object_Loader&,
		 uint32_t);
	virtual uint32_t write_global_env
		(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver&);
};

/*
 * Special handling of math.random.
 *
 * We inject this function to make sure that lua uses our random number
 * generator.  This guarantees that the game stays in sync over the network and
 * in replays. Obviously, we only do this for LuaGameInterface, not for
 * the others.
 *
 * The function was designed to simulate the standard math.random function and
 * was therefore nearly verbatimly copied from the lua sources.
 */
static int L_math_random(lua_State * L) {
	Widelands::Game & game = get_game(L);
	uint32_t t = game.logic_rand();

	lua_Number r = t / 4294967296.; // create a double in [0,1)

	switch (lua_gettop(L)) {  /* check number of arguments */
		case 0:
		{  /* no arguments */
			lua_pushnumber(L, r);  /* Number between 0 and 1 */
			break;
		}
		case 1:
		{  /* only upper limit */
			int32_t u = luaL_checkint32(L, 1);
			luaL_argcheck(L, 1 <= u, 1, "interval is empty");
			lua_pushnumber(L, floor(r * u) + 1);  /* int between 1 and `u' */
			break;
		}
		case 2:
		{  /* lower and upper limits */
			int32_t l = luaL_checkint32(L, 1);
			int32_t u = luaL_checkint32(L, 2);
			luaL_argcheck(L, l <= u, 2, "interval is empty");
			/* int between `l' and `u' */
			lua_pushnumber(L, floor(r * (u - l + 1)) + l);
			break;
		}
		default: return luaL_error(L, "wrong number of arguments");
	}
	return 1;

}

LuaGameInterface_Impl::LuaGameInterface_Impl(Widelands::Game * g) :
	LuaEditorGameBaseInterface_Impl(g)
{
	// Overwrite math.random
	lua_getglobal(m_L, "math");
	lua_pushcfunction(m_L, L_math_random);
	lua_setfield(m_L, -2, "random");
	lua_pop(m_L, 1); // pop "math"

	// Push the game onto the stack
	lua_pushlightuserdata(m_L, static_cast<void *>(g));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "game");
}

LuaCoroutine * LuaGameInterface_Impl::read_coroutine
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	LuaCoroutine_Impl * rv = new LuaCoroutine_Impl(0);

	rv->read(m_L, fr, mol, size);

	return rv;
}

uint32_t LuaGameInterface_Impl::write_coroutine
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos,
	 LuaCoroutine * cr)
{
	// we do not want to make the write function public by adding
	// it to the interface of LuaCoroutine. Therefore, we make a cast
	// to the Implementation function here.
	return dynamic_cast<LuaCoroutine_Impl *>(cr)->write(m_L, fw, mos);
}


static const char * m_persistent_globals[] = {
	"_VERSION", "assert", "collectgarbage", "coroutine", "debug",
	"dofile", "error", "gcinfo", "getfenv", "getmetatable", "io", "ipairs",
	"load", "loadfile", "loadstring", "math", "module", "newproxy", "next",
	"os", "package", "pairs", "pcall", "print", "rawequal",
	"rawget", "rawset", "require", "select", "setfenv", "setmetatable",
	"table", "tonumber", "tostring", "type", "unpack", "wl", "xpcall",
	"string", "use", "_", "set_textdomain", "coroutine.yield", 0
};
void LuaGameInterface_Impl::read_global_env
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	// Empty table + object to persist on the stack Stack
	unpersist_object(m_L, m_persistent_globals, fr, mol, size);
	luaL_checktype(m_L, -1, LUA_TTABLE);

	// Now, we have to merge all keys from the loaded table
	// into the global table
	lua_pushnil(m_L);
	while (lua_next(m_L, -2) != 0) {
		// key value
		lua_pushvalue(m_L, -2); // key value key
		lua_gettable(m_L, LUA_GLOBALSINDEX); // key value global_value
		if (lua_equal(m_L, -1, -2)) {
			lua_pop(m_L, 2); // key
			continue;
		} else {
			// Make this a global value
			lua_pop(m_L, 1); // key value
			lua_pushvalue(m_L, -2); // key value key
			lua_pushvalue(m_L, -2); // key value key value
			lua_settable(m_L, LUA_GLOBALSINDEX); // key value
			lua_pop(m_L, 1); // key
		}
	}

	lua_pop(m_L, 1); // pop the table returned by unpersist_object
}

uint32_t LuaGameInterface_Impl::write_global_env
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos)
{
	// Empty table + object to persist on the stack Stack
	lua_newtable(m_L);
	lua_pushvalue(m_L, LUA_GLOBALSINDEX);

	return persist_object(m_L, m_persistent_globals, fw, mos);
}

/*
============================================
       Global functions
============================================
*/
/*
 * Factory Function, create lua interfaces for the following use cases:
 *
 * "game": load all libraries needed for the game to run properly
 */
LuaGameInterface* create_LuaGameInterface(Widelands::Game * g) {
	return new LuaGameInterface_Impl(g);
}
LuaInterface* create_LuaEditorInterface(Widelands::Editor_Game_Base * g) {
        return new LuaEditorInterface_Impl(g);
}
LuaInterface* create_LuaInterface() {
	return new LuaInterface_Impl();
}


