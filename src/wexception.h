/*
 * Copyright (C) 2002, 2003 by the Widelands Development Team
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

#ifndef WEXCEPTION_H
#define WEXCEPTION_H

#include <string>
#include <exception>

#include <stdint.h>

#ifdef __GNUC__
#define PRINTF_FORMAT(b, c) __attribute__ ((__format__ (__printf__, b, c)))
#else
#define PRINTF_FORMAT(b, c)
#endif

/** class wexception
 *
 * Stupid, simple exception class. It has the nice bonus that you can give it
 * sprintf()-style format strings
 */
class _wexception : public std::exception {
	std::string m_what;

public:
	explicit _wexception
		(const char * file, uint32_t line, const char * fmt, ...)
		throw () PRINTF_FORMAT(4, 5);
	virtual ~_wexception() throw ();

	/**
    * The target of the returned pointer remains valid during the lifetime of
	 * the _wexception object.
	 */
	virtual const char * what() const throw ();
};

#define wexception(...) _wexception(__FILE__, __LINE__, __VA_ARGS__)


#endif
