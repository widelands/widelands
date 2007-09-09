/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#include "helper.h"

#include "error.h"
#include "wexception.h"

#include <cstdarg>
#include <iostream>
#include <sstream>
#include <vector>

//TODO: move wexception code into it's own file

using std::cout;

/**
 * split a string by whitespace
 */
std::vector<std::string> split_string
(const std::string & s, const char * const separators)
{
	std::vector<std::string> result;
	for
		(std::string::size_type pos = 0, endpos;
		 (pos = s.find_first_not_of(separators, pos)) != std::string::npos;
		 pos = endpos)
	{
		endpos = s.find_first_of(separators, pos);
		result.push_back(s.substr(pos, endpos - pos));
	}
	return result;
}

/**
 * remove spaces at the beginning or the end of a string
 */
void remove_spaces(std::string & s) {
	while (s[0] == ' ' or s[0] == '\t' or s[0] == '\n') s.erase(0, 1);

	while
		(s[s.size() - 1] == ' '  or
		 s[s.size() - 1] == '\t' or
		 s[s.size() - 1] == '\n')
		s.erase(s.size() - 1, 1);
}

/**
 * print a formatted log messages to cout.
 */
void log(const char *fmt, ...)
{
	char buffer[2048];
	va_list va;

	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	//TODO: use iostreams instead of vprintf because other parts of Widelands use iostreams
	cout << buffer;
	cout.flush();
}



/*
==============================================================================

class _wexception implementation

==============================================================================
*/
#undef wexception
_wexception::_wexception(const char* file, uint line, const char *fmt, ...) throw()
{
	va_list va;
	char buffer[256];
	va_start(va, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, va);
	va_end(va);
	std::ostringstream ost;
	ost << file << ':' << line << ' ' << buffer;
	m_what = ost.str();
}

_wexception::~_wexception() throw()
{
}

const char *_wexception::what() const throw() {return m_what.c_str();}

void myassert(int line, const char* file, const char* condt) throw(_wexception)
{
	critical_error("%s:%i: assertion \"%s\" failed!\n", file, line, condt);
	throw _wexception(__FILE__, __LINE__, "Assertion %s:%i (%s) has been ignored", file, line, condt);
}
