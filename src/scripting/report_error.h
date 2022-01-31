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

#ifndef WL_SCRIPTING_REPORT_ERROR_H
#define WL_SCRIPTING_REPORT_ERROR_H

#include "scripting/lua.h"

#ifdef __GNUC__
void report_error(lua_State*, const char*, ...)
   __attribute__((__format__(__printf__, 2, 3), noreturn));
#else
[[noreturn]] void report_error(lua_State*, const char*, ...);
#endif

#endif  // end of include guard: WL_SCRIPTING_REPORT_ERROR_H
