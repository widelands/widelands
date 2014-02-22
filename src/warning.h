/*
 * Copyright (C) 2002, 2003, 2009 by the Widelands Development Team
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

#ifndef WARNING_H
#define WARNING_H

#include <cstring>
#include <exception>
#include <string>

#include <stdint.h>

#include "port.h"

#ifndef PRINTF_FORMAT
#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__ ((__format__ (__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif
#endif

/// Similar exception type as wexception, but without the debug output.
/// Intended for normal warnings like "map could not be found".
struct warning : public std::exception {
	explicit warning (char const * title, char const * message, ...)
	 PRINTF_FORMAT(3, 4);
	virtual ~warning() throw ();

	/// The target of the returned pointer remains valid during the lifetime of
	/// the warning object.
	virtual const char * title() const;
	virtual const char * what() const throw () override;

protected:
	warning() {};
	std::string m_what;
	std::string m_title;
};

#endif
