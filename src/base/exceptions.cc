/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "base/warning.h"
#include "base/wexception.h"

#include <cstdarg>
#include <sstream>

bool g_fail_on_lua_error(false);

/*
 * class WException implementation
 */
#undef wexception
WException::WException(char const* const file, uint32_t const line, char const* const fmt, ...)
   : file_(file), line_(line) {
	char buffer[512];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	std::ostringstream ost;
	ost << '[' << file << ':' << line << "] " << buffer;
	what_ = ost.str();
}

/*
 * class warning implementation
 */
WLWarning::WLWarning(char const* const title, char const* const message, ...) : title_(title) {
	char buffer[512];
	{
		va_list va;
		va_start(va, message);
		vsnprintf(buffer, sizeof(buffer), message, va);
		va_end(va);
	}
	what_ = buffer;
}

char const* WLWarning::title() const {
	return title_.c_str();
}

char const* WLWarning::what() const noexcept {
	return what_.c_str();
}
