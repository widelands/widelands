/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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

#ifndef WL_BASE_MACROS_H
#define WL_BASE_MACROS_H

#include <cinttypes>

// Make sure that Visual C++ does not bark at __attribute__.
#ifdef _MSC_VER
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

/* Macros for disabling GCC warnings and errors
 * From http://dbp-consulting.com/tutorials/SuppressingGCCWarnings.html and
 * slightly modified to remove support entirely for GCC < 4.6 because we'll
 * use in the middle of functions.
 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#define GCC_DIAG_DO_PRAGMA(x) _Pragma(#x)
#define GCC_DIAG_PRAGMA(x) GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define GCC_DIAG_OFF(x)                                                                            \
	GCC_DIAG_PRAGMA(push)                                                                           \
	GCC_DIAG_PRAGMA(ignored x)
#define GCC_DIAG_ON(x) GCC_DIAG_PRAGMA(pop)
#else
#define GCC_DIAG_OFF(x)
#define GCC_DIAG_ON(x)
#endif

/* Macros for disabling Clang warnings and errors
 * From https://svn.boost.org/trac/boost/wiki/Guidelines/WarningsGuidelines and
 * slightly modified.
 */
#ifdef __clang__
#define CLANG_DIAG_DO_PRAGMA(x) _Pragma(#x)
// _Pragma is unary operator  #pragma ("")
#define CLANG_DIAG_PRAGMA(x) CLANG_DIAG_DO_PRAGMA(clang diagnostic x)
#define CLANG_DIAG_OFF(x)                                                                          \
	CLANG_DIAG_PRAGMA(push)                                                                         \
	CLANG_DIAG_PRAGMA(ignored x)
// For example: #pragma clang diagnostic ignored "-Wno-unused-variable"
#define CLANG_DIAG_ON(x) CLANG_DIAG_PRAGMA(pop)
// For example: #pragma clang diagnostic warning "-Wno-unused-variable"
#else  // Ensure these macros do nothing for other compilers.
#define CLANG_DIAG_OFF(x)
#define CLANG_DIAG_ON(x)
#define CLANG_DIAG_PRAGMA(x)
#endif

#define DIAG_OFF(x) GCC_DIAG_OFF(x) CLANG_DIAG_OFF(x)
#define DIAG_ON(x) GCC_DIAG_ON(x) CLANG_DIAG_ON(x)

// For switch statements: Tell gcc7 and clang that a fallthrough is intended
// https://developers.redhat.com/blog/2017/03/10/wimplicit-fallthrough-in-gcc-7/
#ifdef __clang__
#define FALLS_THROUGH /* Falls through */ [[clang::fallthrough]]
#elif __GNUC__ >= 7
#define FALLS_THROUGH /* Falls through */ [[gnu::fallthrough]]
#else
#define FALLS_THROUGH /* Falls through */
#endif

// Compilers can't handle polymorphy with printf %p, expecting void* only.
// Surround ONLY printf statements that contain %p with these macros.
#ifdef __clang__
#if __has_warning("-Wformat-pedantic")  // Older Clang versions don't have -Wformat-pedantic
#define FORMAT_WARNINGS_OFF CLANG_DIAG_OFF("-Wformat") CLANG_DIAG_OFF("-Wformat-pedantic")
#define FORMAT_WARNINGS_ON CLANG_DIAG_ON("-Wformat") CLANG_DIAG_ON("-Wformat-pedantic")
#else
#define FORMAT_WARNINGS_OFF CLANG_DIAG_OFF("-Wformat")
#define FORMAT_WARNINGS_ON CLANG_DIAG_ON("-Wformat")
#endif
#else
#define FORMAT_WARNINGS_OFF GCC_DIAG_OFF("-Wformat")
#define FORMAT_WARNINGS_ON GCC_DIAG_ON("-Wformat")
#endif

// Couldn't we use `check_cxx_compiler_flag` in the cmake file instead?
// https://cmake.org/cmake/help/latest/module/CheckCXXCompilerFlag.html

// Older clang versions don't have "-Wreserved-identifier" either
#define CLANG_DIAG_RESERVED_IDENTIFIER_OFF
#define CLANG_DIAG_RESERVED_IDENTIFIER_ON
#ifdef __clang__
#if __has_warning("-Wreserved-identifier")
#undef CLANG_DIAG_RESERVED_IDENTIFIER_OFF
#undef CLANG_DIAG_RESERVED_IDENTIFIER_ON
#define CLANG_DIAG_RESERVED_IDENTIFIER_OFF CLANG_DIAG_OFF("-Wreserved-identifier")
#define CLANG_DIAG_RESERVED_IDENTIFIER_ON CLANG_DIAG_ON("-Wreserved-identifier")
#endif
#endif

// disallow copying or assigning a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)                                                         \
	TypeName(const TypeName&) = delete;                                                             \
	void operator=(const TypeName&) = delete

// Wrapper macro around a dynamic_cast.
// Keep in mind that dynamic_casts are performance-hungry, so use this sparingly please.
#define upcast(type, identifier, source) type* const identifier = dynamic_cast<type*>(source)

// consistency check for printf arguments
#ifdef __GNUC__
#ifdef _WIN32
#define PRINTF_FORMAT(b, c) __attribute__((__format__(gnu_printf, b, c)))
#else
#define PRINTF_FORMAT(b, c) __attribute__((__format__(__printf__, b, c)))
#endif
#else
#define PRINTF_FORMAT(b, c)
#endif

#ifdef _WIN32
#ifdef _WIN64
#define PRIuS PRIu64
#else
#define PRIuS PRIu32
#endif
#else
#if SIZE_MAX == UINT64_MAX
#define PRIuS "lu"
#else
#define PRIuS "u"
#endif
#endif

#endif  // end of include guard: WL_BASE_MACROS_H
