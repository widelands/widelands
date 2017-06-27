/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_BASE_LOG_H
#define WL_BASE_LOG_H

#include <iostream>

#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__((__format__(__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif

// printf macros for size_t, in the style of inttypes.h
#ifdef _LP64
#define PRIS_PREFIX "z"
#else
#define PRIS_PREFIX
#endif

// Use these macros after a % in a printf format string
// to get correct 32/64 bit behavior, like this:
// size_t size = records.size();
// printf("%" PRIuS "\n", size);

#define PRIdS PRIS_PREFIX "d"
#define PRIxS PRIS_PREFIX "x"
// Win64 is finicky:
// https://stackoverflow.com/questions/44382862/how-to-printf-a-size-t-without-warning-in-mingw-w64-gcc-7-1
#ifdef _WIN64
#define PRIuS PRIu64
#else
#define PRIuS PRIS_PREFIX "u"
#endif
#define PRIXS PRIS_PREFIX "X"
#define PRIoS PRIS_PREFIX "o"

// Print a formatted log messages to stdout on most systems and 'stdout.txt' on windows.
void log(const char*, ...) PRINTF_FORMAT(1, 2);

extern bool g_verbose;

#endif  // end of include guard: WL_BASE_LOG_H
