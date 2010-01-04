/*
 * Copyright (C) 2006-2009 by the Widelands Development Team
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

#ifndef S_LUASCRIPTING_H
#define S_LUASCRIPTING_H

#include <string>

extern "C" {
#include <lua.h>
}

#include "scripting.h"

namespace Widelands {
	struct Game;
}

class LuaInterface {
	public:
		LuaInterface(Widelands::Game* const);
		~LuaInterface();

		int interpret_string(std::string);
		const std::string& get_last_error() const { return m_last_error; }

	private:
		lua_State* m_luastate;
		std::string m_last_error;
};


#endif /* end of include guard: S_LUA_H */

