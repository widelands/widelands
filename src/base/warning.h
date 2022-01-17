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

#ifndef WL_BASE_WARNING_H
#define WL_BASE_WARNING_H

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

/// Similar exception type as wexception, but without the debug output.
/// Intended for normal warnings like "map could not be found".
struct WLWarning : public std::exception {
	explicit WLWarning(char const* title, char const* message, ...) PRINTF_FORMAT(3, 4);

	/// The target of the returned pointer remains valid during the lifetime of
	/// the warning object.
	virtual const char* title() const;
	const char* what() const noexcept override;

protected:
	WLWarning() {
	}
	std::string what_;
	std::string title_;
};

#endif  // end of include guard: WL_BASE_WARNING_H
