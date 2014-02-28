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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <map>
#include <string>

#include <stdint.h>

#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "scripting/eris/lua.hpp"
#include "wexception.h"

namespace Widelands {
	class Editor_Game_Base;
	class Game;
	class Map_Map_Object_Loader;
	struct Map_Map_Object_Saver;
	class Player;
}

class EditorFactory;
class GameFactory;

class LuaError : public _wexception {
public:
	LuaError(const std::string & reason) : wexception("%s", reason.c_str()) {}
};
class LuaValueError : public LuaError {
public:
	LuaValueError(const std::string & wanted) :
		LuaError("Variable not of expected type: " + wanted)
	{}
};
class LuaTableKeyError : public LuaError {
public:
	LuaTableKeyError(const std::string & wanted) :
		LuaError(wanted + " is not a field in this table.")
	{}
};
class LuaScriptNotExistingError : public LuaError {
public:
	LuaScriptNotExistingError(std::string ns, std::string name) :
		LuaError("The script '" + ns + ":" + name + "' was not found!") {}
};

class LuaCoroutine;

/*
 * Easy handling of return values from Wideland's Lua configurations
 * scripts: they return a Lua table with (string,value) pairs.
 */
class LuaTable {
public:
	LuaTable(lua_State* L);
	~LuaTable();

	std::string get_string(std::string);
	LuaCoroutine* get_coroutine(std::string);

private:
	lua_State * m_L;
};

// Provides an interface to call and execute Lua Code.
typedef std::map<std::string, std::string> ScriptContainer;
class LuaInterface {
public:
	LuaInterface();
	virtual ~LuaInterface();

	// Register all Lua files in the directory "subdir" in "fs" under the
	// namespace "ns".
	void register_scripts
		(FileSystem & fs, const std::string& ns, const std::string& subdir);

	// Register the Lua file "filename" in "fs" under the namespace "ns". Returns
	// the name the script was registered, usually $(basename filename).
	std::string register_script
		(FileSystem & fs, const std::string& ns, const std::string& filename);

	// Returns the scripts that have been registered for this namespace.
	const ScriptContainer& get_scripts_for(const std::string&);

	// Interpret the given string, will throw 'LuaError' on any error.
	void interpret_string(const std::string&);

	std::unique_ptr<LuaTable> run_script(std::string, std::string);
	std::unique_ptr<LuaTable> run_script
			(FileSystem &, std::string, std::string);

	std::unique_ptr<LuaTable> get_hook(std::string name);

protected:
	lua_State * m_L;

private:
	std::map<std::string, ScriptContainer> m_scripts;
};

class LuaEditorInterface : public LuaInterface {
public:
	LuaEditorInterface(Widelands::Editor_Game_Base * g);
	virtual ~LuaEditorInterface();

private:
	std::unique_ptr<EditorFactory> m_factory;
};

class LuaGameInterface : public LuaInterface {
public:
	LuaGameInterface(Widelands::Game * g);
	virtual ~LuaGameInterface();

	// Input/output for coroutines.
	LuaCoroutine * read_coroutine
		(Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);
	uint32_t write_coroutine
		(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &, LuaCoroutine *);

	// Input output for the global game state.
	void read_global_env
		(Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);
	uint32_t write_global_env
		(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &);

private:
	std::unique_ptr<GameFactory> m_factory;
};

// Easy handling of function objects and coroutines.
class LuaCoroutine {
public:
	// The state of the coroutine, which can either be yielded, i.e. it expects
	// to be resumed again or done which means that it will not do any more work
	// and can be deleted.
	enum {
		DONE = 0,
		YIELDED = LUA_YIELD,
	};

	LuaCoroutine(lua_State * L);
	virtual ~LuaCoroutine();

	// Returns either 'DONE' or 'YIELDED'.
	int get_status();

	// Resumes the coroutine and returns it's state after it did its execution.
	// If 'sleeptime' is not null, it will contain the time in milliseconds the
	// lua code requested for this coroutine to sleep before it should be
	// resumed again.
	int resume(uint32_t* sleeptime = nullptr);

	// Push the given arguments onto the Lua stack, so that a Coroutine can
	// receive them. This is for example used in the initialization scripts or
	// in hooks.
	void push_arg(const Widelands::Player *);
	void push_arg(const Widelands::Coords &);

private:
	friend class LuaGameInterface;

	// Input/Output for coroutines. Do not call directly, instead use
	// LuaGameInterface methods for this.
	uint32_t write(lua_State *, Widelands::FileWrite &, Widelands::Map_Map_Object_Saver &);
	void read(lua_State *, Widelands::FileRead &, Widelands::Map_Map_Object_Loader &, uint32_t);

	lua_State* m_L;
	uint32_t m_idx;
	uint32_t m_nargs;
};

#endif
