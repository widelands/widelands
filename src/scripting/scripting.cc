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

#include "scripting.h"

#include "log.h"

#include "lua_debug.h"
#include "lua_game.h"
#include "lua_map.h"
#include "c_utils.h"

#include <string>
#include <stdexcept>

#include "log.h"
#include "lua_debug.h"
#include "lua_game.h"
#include "lua_map.h"
#include "c_utils.h"

#include "scripting.h"

// TODO: remove this include
extern "C" {
#include "pluto/pluto.h"
}

// TODO: *.lua globbing doesn't work with zip file system

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
}

LuaInterface_Impl::~LuaInterface_Impl() {
	lua_close(m_L);
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

