/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#include "logic/map_objects/backtrace.h"

#ifndef _WIN32
#ifndef __APPLE__
#if !defined(__linux__) || defined(__GLIBC__)

#include <execinfo.h>
#endif
#endif
#endif

std::string get_backtrace() {
	std::string result("Backtrace:\n");
#ifndef _WIN32
#ifndef __APPLE__
#if !defined(__linux__) || defined(__GLIBC__)
#define BACKTRACE_STACKSIZE 24

	void* stack[BACKTRACE_STACKSIZE];
	size_t size = backtrace(stack, BACKTRACE_STACKSIZE);
	char** const list = backtrace_symbols(stack, size);
	for (char* const* it = list; size; --size, ++it) {
		result += *it;
		result += '\n';
	}
	free(list);
#endif
#endif
#endif
	return result;
}
