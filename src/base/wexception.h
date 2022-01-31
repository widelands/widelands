/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_BASE_WEXCEPTION_H
#define WL_BASE_WEXCEPTION_H

#include <exception>
#include <string>

#include "base/macros.h"

#ifndef PRINTF_FORMAT
#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__((__format__(__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif
#endif

/** Stupid, simple exception class.
 *
 * It has the nice bonus that you can give it sprintf()-style format strings.
 */
class WException : public std::exception {
public:
	explicit WException(char const* const file, uint32_t const line, char const* const fmt, ...)
	   PRINTF_FORMAT(4, 5);

	/**
	 * The target of the returned pointer remains valid during the lifetime of
	 * the WException object.
	 */
	const char* what() const noexcept override;

protected:
	WException() {
	}
	std::string what_;
};

extern bool g_fail_on_lua_error;

#define wexception(...) WException(__FILE__, __LINE__, __VA_ARGS__)

// Throws a wexception for unreachable code.
#define NEVER_HERE() throw WException(__FILE__, __LINE__, "Unreachable code was reached.")

#endif  // end of include guard: WL_BASE_WEXCEPTION_H
