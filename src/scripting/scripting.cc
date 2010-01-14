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
#include "lua_map.h"

#include <string>
#include <stdexcept>

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

/*
============================================
       Lua Coroutine
============================================
*/
class LuaCoroutine_Impl : public LuaCoroutine {
	public:
		LuaCoroutine_Impl(lua_State * L) : m_L(L) {}
		virtual ~LuaCoroutine_Impl() {}

		virtual int get_status() {
			return lua_status(m_L);
		}
		virtual int resume() {
			return lua_resume(m_L, 0);
		}

	private:
		lua_State * m_L;
};

/*
============================================
       Lua State
============================================
*/
class LuaState_Impl : public LuaState {
	private:
		bool m_owning_state;

	// TODO: this shouldn't be public
	public:
		lua_State * m_L;

	public:
		LuaState_Impl(lua_State * l, bool owning_state);
		virtual ~LuaState_Impl();

		virtual void push_int32(int32_t);
		virtual int32_t pop_int32();
		virtual void push_uint32(uint32_t);
		virtual uint32_t pop_uint32();
		virtual void push_double(double);
		virtual double pop_double();
		virtual void push_string(std::string &);
		virtual std::string pop_string();
		virtual LuaCoroutine * pop_coroutine();
};

/*
 * Public functions
 */
LuaState_Impl::LuaState_Impl(lua_State * l, bool owning_state) :
	m_owning_state(owning_state),
	m_L(l)
{
}

LuaState_Impl::~LuaState_Impl()
{
	if (m_owning_state)
		lua_close(m_L);
}

/**
 * Pops one value from the lua state with error checking
 * returns this value to the caller.
 */
uint32_t LuaState_Impl::pop_uint32()
{
	lua_Integer d = lua_tointeger(m_L, -1);
	if ((d < 0) or (d == 0 && !lua_isnumber(m_L, -1)))
		throw LuaValueError("uint32");
	lua_pop(m_L, 1);
	return static_cast<uint32_t>(d);
}

int32_t LuaState_Impl::pop_int32()
{
	lua_Integer d = lua_tointeger(m_L, -1);
	if (d == 0 && !lua_isnumber(m_L, -1))
		throw LuaValueError("int32");
	lua_pop(m_L, 1);
	return static_cast<int32_t>(d);
}

double LuaState_Impl::pop_double()
{
	if (!lua_isnumber(m_L, -1))
		throw LuaValueError("double");
	lua_Number d = lua_tonumber(m_L, -1);
	lua_pop(m_L, 1);
	return static_cast<double>(d);
}

std::string LuaState_Impl::pop_string()
{
	const char * s = lua_tostring(m_L, -1);
	if (!s)
		throw LuaValueError("string");
	lua_pop(m_L, 1);
	return std::string(s);
}

LuaCoroutine * LuaState_Impl::pop_coroutine()
{
	lua_State * L = lua_tothread(m_L, -1);
	if (!L)
		throw LuaValueError("coroutine");
	lua_pop(m_L, 1);
	return new LuaCoroutine_Impl(L);
}

/**
 * Unconditionally push the value on the Stack
 */
void LuaState_Impl::push_int32(int32_t val)
{
	lua_pushinteger(m_L, val);
}

void LuaState_Impl::push_uint32(uint32_t val)
{
	lua_pushinteger(m_L, val);
}
void LuaState_Impl::push_double(double val)
{
	lua_pushnumber(m_L, val);
}
void LuaState_Impl::push_string(std::string & val)
{
	lua_pushstring(m_L, val.c_str());
}

/*
============================================
       Lua Interface
============================================
*/
class LuaInterface_Impl : public LuaInterface {
	LuaState_Impl * m_state;
	std::string m_last_error;

	/*
	 * Private functions
	 */
	private:
		int m_check_for_errors(int);

	public:
		LuaInterface_Impl(Widelands::Game *);
		virtual ~LuaInterface_Impl();

		virtual LuaState * interpret_string(std::string);
		virtual LuaState * interpret_file(std::string);
		virtual std::string const & get_last_error() const {return m_last_error;}
};

/*************************
 * Private functions
 *************************/
int LuaInterface_Impl::m_check_for_errors(int rv) {
	if (rv)
		throw LuaError(m_state->pop_string());
	return rv;
}

/*************************
 * Public functions
 *************************/
LuaInterface_Impl::LuaInterface_Impl
	(Widelands::Game * const game) : m_last_error("")
{
	lua_State * const L = lua_open();

	m_state = new LuaState_Impl(L, true);

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
		lua_pushcfunction(L, lib->func);
		lua_pushstring(L, lib->name);
		lua_call(L, 1, 0);
	}

	// Now our own
	luaopen_wldebug(L);
	luaopen_wlmap(L);

	// Push the game onto the stack
	lua_pushstring(L, "game");
	lua_pushlightuserdata(L, static_cast<void *>(game));
	lua_settable(L, LUA_REGISTRYINDEX);
}

LuaInterface_Impl::~LuaInterface_Impl() {
	delete m_state;
}

LuaState * LuaInterface_Impl::interpret_string(std::string cmd) {
	int rv = luaL_dostring(m_state->m_L, cmd.c_str());
	m_check_for_errors(rv);

	return m_state;
}

LuaState * LuaInterface_Impl::interpret_file(std::string filename) {
	log("In LuaInterface::interpret_file:\n");
	log(" <%s>\n", filename.c_str());

	int rv = luaL_dofile(m_state->m_L, filename.c_str());
	m_check_for_errors(rv);

	return m_state;
}

/*
============================================
       Global functions
============================================
*/
/*
 * Factory Function
 */
LuaInterface * create_lua_interface(Widelands::Game * g) {
	return new LuaInterface_Impl(g);
}

