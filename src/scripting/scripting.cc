/*
 * Copyright (C) 2006-2010, 2013 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "scripting/scripting.h"

#include <stdexcept>
#include <string>

#ifdef _MSC_VER
#include <ctype.h> // for tolower
#endif

#include "io/filesystem/layered_filesystem.h"
#include "log.h"
#include "scripting/c_utils.h"
#include "scripting/factory.h"
#include "scripting/lua_bases.h"
#include "scripting/lua_editor.h"
#include "scripting/lua_game.h"
#include "scripting/lua_globals.h"
#include "scripting/lua_map.h"
#include "scripting/lua_root.h"
#include "scripting/lua_ui.h"
#include "scripting/persistence.h"

namespace {

// Calls 'method_to_call' with argument 'name'. This expects that this will
// return a table with registered functions in it. If 'register_globally' is
// true, this will also do name = <table> globally.
void open_lua_library
	(lua_State* L, const std::string& name, lua_CFunction method_to_call, bool register_globally) {
	lua_pushcfunction(L, method_to_call);  // S: function
	lua_pushstring(L, name); // S: function name
	lua_call(L, 1, 1); // S: module_table

	if (register_globally) {
		lua_setglobal(L, name.c_str()); // S:
	} else {
		lua_pop(L, 1); // S:
	}
}

// Checks the return value of a function all for nonzero state and throws the
// string that the function hopefully pushed as an Error. Returns 'rv' if there
// is no error.
int check_return_value_for_errors(lua_State* L, int rv) {
	if (rv) {
		const std::string err = luaL_checkstring(L, -1);
		lua_pop(L, 1);
		throw LuaError(err);
	}
	return rv;
}

// Returns true if 's' is too short for having an extension.
bool too_short_for_extension(const std::string & s) {
	return s.size() < 4;
}

// Returns true if 's' ends with .lua.
bool is_lua_file(const std::string& s) {
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

// Setup the basic Widelands functions and pushes egbase into the Lua registry
// so that it is available for all the other Lua functions.
void setup_for_editor_and_game(lua_State* L, Widelands::Editor_Game_Base * g) {
	LuaBases::luaopen_wlbases(L);
	LuaMap::luaopen_wlmap(L);
	LuaUi::luaopen_wlui(L);

	// Push the editor game base
	lua_pushlightuserdata(L, static_cast<void *>(g));
	lua_setfield(L, LUA_REGISTRYINDEX, "egbase");
}

// Push a reference to this coroutine into the registry so that it will not get
// garbage collected. Returns the index of the reference in the registry.
uint32_t reference_coroutine(lua_State* L) {
	assert(L != nullptr);
	lua_pushthread(L);
	return luaL_ref(L, LUA_REGISTRYINDEX);
}

// Unreference the coroutine with the given index in the registry again. I might then get garbage
// collected.
void unreference_coroutine(lua_State* L, uint32_t idx) {
	luaL_unref(L, LUA_REGISTRYINDEX, idx);
}

}  // namespace

/*
============================================
       Lua Table
============================================
*/
LuaTable::LuaTable(lua_State * L) : m_L(L) {}

LuaTable::~LuaTable() {
	lua_pop(m_L, 1);
}

std::string LuaTable::get_string(std::string s) {
	lua_getfield(m_L, -1, s.c_str());
	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 1);
		throw LuaTableKeyError(s);
	}
	if (not lua_isstring(m_L, -1)) {
		lua_pop(m_L, 1);
		throw LuaError(s + "is not a string value.");
	}
	std::string rv = lua_tostring(m_L, -1);
	lua_pop(m_L, 1);

	return rv;
}

LuaCoroutine * LuaTable::get_coroutine(std::string s) {
	lua_getfield(m_L, -1, s.c_str());

	if (lua_isnil(m_L, -1)) {
			lua_pop(m_L, 1);
			throw LuaTableKeyError(s);
	}
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
		throw LuaError(s + "is not a function value.");
	}
	LuaCoroutine * cr = new LuaCoroutine(luaL_checkthread(m_L, -1));
	lua_pop(m_L, 1); // Remove coroutine from stack
	return cr;
}

/*
============================================
       Lua Interface
============================================
*/
LuaInterface::LuaInterface() {
	m_L = luaL_newstate();

	// Open the Lua libraries
	open_lua_library(m_L, "", luaopen_base, false);
	open_lua_library(m_L, LUA_TABLIBNAME, luaopen_table, true);
	open_lua_library(m_L, LUA_STRLIBNAME, luaopen_string, true);
	open_lua_library(m_L, LUA_MATHLIBNAME, luaopen_math, true);
	open_lua_library(m_L, LUA_DBLIBNAME, luaopen_debug, true);
	open_lua_library(m_L, LUA_COLIBNAME, luaopen_coroutine, true);

#ifndef NDEBUG
	open_lua_library(m_L, LUA_LOADLIBNAME, luaopen_package, true);
	open_lua_library(m_L, LUA_IOLIBNAME, luaopen_io, true);
	open_lua_library(m_L, LUA_OSLIBNAME, luaopen_os, true);
#endif

	// Push the instance of this class into the registry
	// MSVC2008 requires that stored and retrieved types are
	// same, so use LuaInterface* on both sides.
	lua_pushlightuserdata
		(m_L, reinterpret_cast<void *>(dynamic_cast<LuaInterface *>(this)));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "lua_interface");

	// Now our own
	LuaGlobals::luaopen_globals(m_L);

	// Also push the "wl" table.
	lua_newtable(m_L);
	lua_setglobal(m_L, "wl");

	register_scripts(*g_fs, "aux", "scripting");
}

LuaInterface::~LuaInterface() {
	lua_close(m_L);
}

const ScriptContainer& LuaInterface::get_scripts_for(const std::string& ns) {
	return m_scripts[ns];
}

std::string LuaInterface::register_script
	(FileSystem & fs, const std::string& ns, const std::string& path)
{
		size_t length;
		void * input_data = fs.Load(path, length);

		std::string data(static_cast<char *>(input_data));
		std::string name = path.substr(0, path.size() - 4); // strips '.lua'

		// make sure the input_data is freed
		free(input_data);

		size_t pos = name.find_last_of("/\\");
		if (pos != std::string::npos)  name = name.substr(pos + 1);

		log("Registering script: (%s,%s)\n", ns.c_str(), name.c_str());
		m_scripts[ns][name] = data;

		return name;
}

void LuaInterface::register_scripts
	(FileSystem & fs, const std::string& ns, const std::string& subdir)
{
	filenameset_t scripting_files;

	// Theoretically, we should be able to use fs.FindFiles(*.lua) here,
	// but since FindFiles doesn't support globbing in Zips and most
	// saved maps/games are zip, we have to work around this issue.
	fs.FindFiles(subdir, "*", &scripting_files);

	for
		(filenameset_t::iterator i = scripting_files.begin();
		 i != scripting_files.end(); ++i)
	{
		if (too_short_for_extension(*i) or not is_lua_file(*i))
			continue;

		register_script(fs, ns, *i);
	}
}

void LuaInterface::interpret_string(const std::string& cmd) {
	int rv = luaL_dostring(m_L, cmd.c_str());
	check_return_value_for_errors(m_L, rv);
}

std::unique_ptr<LuaTable> LuaInterface::run_script
	(FileSystem & fs, std::string path, std::string ns)
{
	bool delete_ns = false;
	if (not m_scripts.count(ns))
		delete_ns = true;

	const std::string name = register_script(fs, ns, path);

	std::unique_ptr<LuaTable> rv = run_script(ns, name);

	if (delete_ns)
		m_scripts.erase(ns);
	else
		m_scripts[ns].erase(name);

	return rv;
}

	std::unique_ptr<LuaTable> LuaInterface::run_script
	(std::string ns, std::string name)
{
	if
		((m_scripts.find(ns) == m_scripts.end()) ||
		 (m_scripts[ns].find(name) == m_scripts[ns].end()))
		throw LuaScriptNotExistingError(ns, name);

	const std::string & s = m_scripts[ns][name];

	check_return_value_for_errors
		(m_L, luaL_loadbuffer(m_L, s.c_str(), s.size(), (ns + ":" + name).c_str()) ||
		 lua_pcall(m_L, 0, 1, 0)
	);

	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 1); // No return value from script
		lua_newtable(m_L); // Push an empty table
	}
	if (not lua_istable(m_L, -1))
		throw LuaError("Script did not return a table!");
	return std::unique_ptr<LuaTable>(new LuaTable(m_L));
}

/*
 * Returns a given hook if one is defined, otherwise returns 0
 */
std::unique_ptr<LuaTable> LuaInterface::get_hook(std::string name) {
	lua_getglobal(m_L, "hooks");
	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 1);
		return std::unique_ptr<LuaTable>();
	}

	lua_getfield(m_L, -1, name.c_str());
	if (lua_isnil(m_L, -1)) {
		lua_pop(m_L, 2);
		return std::unique_ptr<LuaTable>();
	}
	lua_remove(m_L, -2);

	return std::unique_ptr<LuaTable>(new LuaTable(m_L));
}


/*
 * ===========================
 * LuaEditorInterface
 * ===========================
 */
LuaEditorInterface::LuaEditorInterface(Widelands::Editor_Game_Base* g)
	: m_factory(new EditorFactory())
{
	setup_for_editor_and_game(m_L, g);
	LuaRoot::luaopen_wlroot(m_L, true);
	LuaEditor::luaopen_wleditor(m_L);

	// Push the factory class into the registry
	lua_pushlightuserdata(m_L, reinterpret_cast<void*>(dynamic_cast<Factory*>(m_factory.get())));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "factory");
}

LuaEditorInterface::~LuaEditorInterface() {
}


/*
 * ===========================
 * LuaGameInterface
 * ===========================
 */

// Special handling of math.random.

// We inject this function to make sure that Lua uses our random number
// generator.  This guarantees that the game stays in sync over the network and
// in replays. Obviously, we only do this for LuaGameInterface, not for
// the others.

// The function was designed to simulate the standard math.random function and
// was therefore copied nearly verbatim from the Lua sources.
static int L_math_random(lua_State * L) {
	Widelands::Game & game = get_game(L);
	uint32_t t = game.logic_rand();

	lua_Number r = t / 4294967296.; // create a double in [0,1)

	switch (lua_gettop(L)) { /* check number of arguments */
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

LuaGameInterface::LuaGameInterface(Widelands::Game * g)
	: m_factory(new GameFactory())
{
	setup_for_editor_and_game(m_L, g);

	// Overwrite math.random
	lua_getglobal(m_L, "math");
	lua_pushcfunction(m_L, L_math_random);
	lua_setfield(m_L, -2, "random");
	lua_pop(m_L, 1); // pop "math"

	LuaRoot::luaopen_wlroot(m_L, false);
	LuaGame::luaopen_wlgame(m_L);

	// Push the game into the registry
	lua_pushlightuserdata(m_L, static_cast<void *>(g));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "game");

	// Push the factory class into the registry
	lua_pushlightuserdata
		(m_L, reinterpret_cast<void *>(dynamic_cast<Factory *>(m_factory.get())));
	lua_setfield(m_L, LUA_REGISTRYINDEX, "factory");
}

LuaGameInterface::~LuaGameInterface() {
}

LuaCoroutine * LuaGameInterface::read_coroutine
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	LuaCoroutine * rv = new LuaCoroutine(nullptr);
	rv->read(m_L, fr, mol, size);

	return rv;
}

uint32_t LuaGameInterface::write_coroutine
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos, LuaCoroutine * cr)
{
	return cr->write(m_L, fw, mos);
}


void LuaGameInterface::read_global_env
	(Widelands::FileRead & fr, Widelands::Map_Map_Object_Loader & mol,
	 uint32_t size)
{
	// Clean out the garbage before loading.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	assert(lua_gettop(m_L) == 0); // S:
	unpersist_object(m_L, fr, mol, size);
	assert(lua_gettop(m_L) == 1); // S: unpersisted_object
	luaL_checktype(m_L, -1, LUA_TTABLE);

	// Now, we have to merge all keys from the loaded table
	// into the global table
	lua_pushnil(m_L);  // S: table nil
	while (lua_next(m_L, 1) != 0) {
		// S: table key value
		lua_rawgeti(m_L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);  // S: table key value globals_table
		lua_pushvalue(m_L, -3); // S: table key value globals_table key
		lua_gettable(m_L, -2);  // S: table key value globals_table value_in_globals
		if (lua_compare(m_L, -1, -3, LUA_OPEQ)) {
			lua_pop(m_L, 3); // S: table key
			continue;
		} else {
			// Make this a global value
			lua_pop(m_L, 1);  // S: table key value globals_table
			lua_pushvalue(m_L, -3);  // S: table key value globals_table key
			lua_pushvalue(m_L, -3);  // S: table key value globals_table key value
			lua_settable(m_L, -3);  // S: table key value globals_table
			lua_pop(m_L, 2);  // S: table key
		}
	}

	lua_pop(m_L, 1); // pop the table returned by unpersist_object

	// Clean out the garbage before returning.
	lua_gc(m_L, LUA_GCCOLLECT, 0);
}

uint32_t LuaGameInterface::write_global_env
	(Widelands::FileWrite & fw, Widelands::Map_Map_Object_Saver & mos)
{
	// Clean out the garbage before writing.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	// Empty table + object to persist on the stack Stack
	lua_newtable(m_L);
	lua_rawgeti(m_L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

	uint32_t nwritten = persist_object(m_L, fw, mos);

	// Garbage collect once more, so we do not return unnecessary stuff.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	return nwritten;
}

/*
 * ===========================
 * LuaCoroutine
 * ===========================
 */
LuaCoroutine::LuaCoroutine(lua_State* ms)
   : m_L(ms), m_idx(LUA_REFNIL), m_ninput_args(0), m_nreturn_values(0) {
	if (m_L) {
		m_idx = reference_coroutine(m_L);
	}
}

LuaCoroutine::~LuaCoroutine() {
	unreference_coroutine(m_L, m_idx);
}

int LuaCoroutine::get_status() {
	return lua_status(m_L);
}

int LuaCoroutine::resume() {
	int rv = lua_resume(m_L, nullptr, m_ninput_args);
	m_ninput_args = 0;
	m_nreturn_values = lua_gettop(m_L);

	if (rv != 0 && rv != YIELDED) {
		const char* err = lua_tostring(m_L, -1);
		throw LuaError(err);
	}

	return rv;
}

void LuaCoroutine::push_arg(const Widelands::Player * plr) {
	to_lua<LuaGame::L_Player>(m_L, new LuaGame::L_Player(plr->player_number()));
	m_ninput_args++;
}

void LuaCoroutine::push_arg(const Widelands::Coords & coords) {
	to_lua<LuaMap::L_Field>(m_L, new LuaMap::L_Field(coords));
	++m_ninput_args;
}

void LuaCoroutine::push_arg(const Widelands::Building_Descr* building_descr) {
	assert(building_descr != nullptr);
	to_lua<LuaMap::L_BuildingDescription>(m_L, new LuaMap::L_BuildingDescription(building_descr));
	++m_ninput_args;
}

std::string LuaCoroutine::pop_string() {
	if (!m_nreturn_values) {
		return "";
	}
	if (!lua_isstring(m_L, -1)) {
		throw LuaError("pop_string(), but no string on the stack.");
	}
	const std::string return_value = lua_tostring(m_L, -1);
	lua_pop(m_L, 1);
	--m_nreturn_values;
	return return_value;
}

uint32_t LuaCoroutine::pop_uint32() {
	if (!m_nreturn_values) {
		return 0;
	}
	if (!lua_isnumber(m_L, -1)) {
		throw LuaError("pop_string(), but no integer on the stack.");
	}
	const uint32_t return_value = luaL_checkuint32(m_L, -1);
	lua_pop(m_L, 1);
	--m_nreturn_values;
	return return_value;
}

#define COROUTINE_DATA_PACKET_VERSION 2
uint32_t LuaCoroutine::write
	(lua_State * parent, Widelands::FileWrite & fw,
	 Widelands::Map_Map_Object_Saver & mos)
{
	// Clean out the garbage before we write this.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	fw.Unsigned8(COROUTINE_DATA_PACKET_VERSION);

	fw.Unsigned32(m_ninput_args);
	fw.Unsigned32(m_nreturn_values);

	// Empty table + object to persist on the stack Stack
	lua_newtable(parent);
	lua_pushthread(m_L);
	lua_xmove(m_L, parent, 1);

	const uint32_t nwritten = persist_object(parent, fw, mos);

	// Clean out the garbage again.
	lua_gc(m_L, LUA_GCCOLLECT, 0);

	return nwritten;
}


void LuaCoroutine::read
	(lua_State * parent, Widelands::FileRead & fr,
	 Widelands::Map_Map_Object_Loader & mol, uint32_t size)
{
	uint8_t version = fr.Unsigned8();

	if (version > COROUTINE_DATA_PACKET_VERSION)
		throw wexception("Unknown data packet version: %i\n", version);

	// The current numbers of arguments on the stack
	m_ninput_args = fr.Unsigned32();
	if (version > 1) {
		m_nreturn_values = fr.Unsigned32();
	}

	// Empty table + object to persist on the stack Stack
	unpersist_object(parent, fr, mol, size);

	m_L = luaL_checkthread(parent, -1);
	lua_pop(parent, 1);

	// Cache a lua reference to this object so that it does not
	// get garbage collected
	lua_pushthread(m_L);
	m_idx = luaL_ref(m_L, LUA_REGISTRYINDEX);

	// Clean out the garbage again.
	lua_gc(m_L, LUA_GCCOLLECT, 0);
}
