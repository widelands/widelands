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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_SCRIPTING_LUA_PATH_H
#define WL_SCRIPTING_LUA_PATH_H

#include "scripting/luna.h"

namespace LuaPath {

/**
 * Base class for all classes in path.
 */
class LuaPathModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "path";
	}
};

void luaopen_path(lua_State*);
}  // namespace LuaPath

#endif  // end of include guard: WL_SCRIPTING_LUA_PATH_H
