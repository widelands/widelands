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

#ifndef SCRIPTING_H
#define SCRIPTING_H

#include <map>
#include <stdexcept>
#include <stdint.h>
#include <string>

#include <lua.hpp>

#include <lua.hpp>

#include "logic/widelands_filewrite.h"
#include "logic/widelands_fileread.h"

namespace Widelands {
	struct Map_Map_Object_Loader;
	struct Map_Map_Object_Saver;
	struct Editor_Game_Base;
	struct Game;
}

struct LuaError : public std::runtime_error {
	LuaError(std::string const & reason) : std::runtime_error(reason) {}
};
struct LuaValueError : public LuaError {
	LuaValueError(std::string const & wanted) :
		LuaError("Variable not of expected type: " + wanted)
	{}
};
class LuaScriptNotExistingError : public LuaError {
	public:
		LuaScriptNotExistingError(std::string name) :
			LuaError("The script '" + name + "' was not found!") {}
};

/**
 * Easy handling of LuaCoroutines
 */
class LuaCoroutine {
	public:
		enum {
			DONE = 0,
			YIELDED = LUA_YIELD,
		};

		virtual int get_status(void) = 0;
		virtual int resume(uint32_t* = 0) = 0;
};

/**
 * This is the thin class that is used to execute code
 */
typedef std::map<std::string, std::string> ScriptContainer;
class LuaInterface {
	public:
		virtual void interpret_string(std::string) = 0;
		virtual void interpret_file(std::string) = 0;
		virtual std::string const & get_last_error() const = 0;

		virtual void register_script(std::string, std::string, std::string) = 0;
		virtual ScriptContainer& get_scripts_for(std::string) = 0;

		virtual void run_script(std::string, std::string) = 0;

		virtual LuaCoroutine* read_coroutine
			(Widelands::FileRead &, Widelands::Map_Map_Object_Loader&,
			 uint32_t) = 0;
		virtual uint32_t write_coroutine
			(Widelands::FileWrite &, Widelands::Map_Map_Object_Saver&,
			 LuaCoroutine *) = 0;
};

LuaInterface* create_lua_interface(Widelands::Editor_Game_Base*);

#endif
