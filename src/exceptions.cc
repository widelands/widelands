/*
 * Copyright (C) 2002, 2006-2009 by the Widelands Development Team
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

#include "warning.h"
#include "wexception.h"

#include <cstdarg>
#include <sstream>

/*
 * class _wexception implementation
 */
#undef wexception
_wexception::_wexception(const char* file, uint32_t line, const char *fmt, ...)
throw ()
{
	char buffer[512];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	std::ostringstream ost;
	ost << '[' << file << ':' << line << "] " << buffer;
	m_what = ost.str();
}

_wexception::~_wexception() throw () {}

const char *_wexception::what() const throw ()
{
	return m_what.c_str();
}


/*
 * class warning implementation
 */
warning::warning(const char *et, const char *em, ...) throw ()
{
	m_title = et;
	m_what = em;
}

warning::~warning() throw () {}

const char *warning::title() const throw ()
{
	return m_title.c_str();
}

const char *warning::what() const throw ()
{
	return m_what.c_str();
}
