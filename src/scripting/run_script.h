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

#ifndef WL_SCRIPTING_RUN_SCRIPT_H
#define WL_SCRIPTING_RUN_SCRIPT_H

#include <memory>

#include "io/filesystem/filesystem.h"
#include "scripting/lua.h"
#include "scripting/lua_table.h"

// Checks the return value of a function all for nonzero state and throws the
// string that the function hopefully pushed as an Error. Returns 'rv' if there
// is no error.
int check_return_value_for_errors(lua_State* L, int rv);

// Runs the 'script' searched in the given 'fs'.
std::unique_ptr<LuaTable> run_script(lua_State* L, const std::string& path, FileSystem* fs);

#endif  // end of include guard: WL_SCRIPTING_RUN_SCRIPT_H
