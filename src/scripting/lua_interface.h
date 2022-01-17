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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_SCRIPTING_LUA_INTERFACE_H
#define WL_SCRIPTING_LUA_INTERFACE_H

#include <memory>

#include "scripting/lua.h"
#include "scripting/lua_errors.h"

class LuaTable;

// Provides an interface to call and execute Lua Code.
class LuaInterface {
public:
	LuaInterface();
	virtual ~LuaInterface();

	// Interpret the given string, will throw 'LuaError' on any error.
	void interpret_string(const std::string&);

	// Runs 'script' and returns the table it returned.
	virtual std::unique_ptr<LuaTable> run_script(const std::string& script);

protected:
	lua_State* lua_state_;
};

#endif  // end of include guard: WL_SCRIPTING_LUA_INTERFACE_H
