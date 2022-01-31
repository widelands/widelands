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

#ifndef WL_SCRIPTING_LUA_GLOBALS_H
#define WL_SCRIPTING_LUA_GLOBALS_H

#include "scripting/lua.h"

class FileRead;
class FileWrite;

namespace LuaGlobals {

void luaopen_globals(lua_State*);

void read_textdomain_stack(FileRead&, const lua_State*);
void write_textdomain_stack(FileWrite&, const lua_State*);

}  // namespace LuaGlobals

#endif  // end of include guard: WL_SCRIPTING_LUA_GLOBALS_H
