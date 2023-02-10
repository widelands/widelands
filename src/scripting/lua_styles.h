/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LUA_STYLES_H
#define WL_SCRIPTING_LUA_STYLES_H

#include "scripting/luna.h"

namespace LuaStyles {

/**
 * Base class for all classes in 'styles'.
 */
class LuaStylesModuleClass : public LunaClass {
public:
	const char* get_modulename() override {
		return "styles";
	}
};

void luaopen_styles(lua_State*);
}  // namespace LuaStyles

#endif  // end of include guard: WL_SCRIPTING_LUA_STYLES_H
