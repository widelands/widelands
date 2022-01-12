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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
WException::WException(char const* const file, uint32_t const line, char const* const fmt, ...) {
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

char const* WException::what() const noexcept {
	return what_.c_str();
}

/*
 * class warning implementation
 */
WLWarning::WLWarning(char const* const et, char const* const em, ...) : title_(et) {
	char buffer[512];
	{
		va_list va;
		va_start(va, em);
		vsnprintf(buffer, sizeof(buffer), em, va);
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
