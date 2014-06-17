/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#ifndef LUA_ERRORS_H
#define LUA_ERRORS_H

#include <string>

#include "wexception.h"

class LuaError : public _wexception {
public:
	LuaError(const std::string& reason) : wexception("%s", reason.c_str()) {
	}
};

class LuaScriptNotExistingError : public LuaError {
public:
	LuaScriptNotExistingError(const std::string& name)
	   : LuaError("The script '" + name + "' was not found!") {
	}
};

#endif /* end of include guard: LUA_ERRORS_H */
