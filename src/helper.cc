/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include <cstdarg>
#include <iostream>
#include <string>
#include <vector>

#include "wexception.h"
#include "error.h"

using std::cout;

/*
===============
split_string

Split a string by whitespace.
===============
*/
void split_string(std::string in, std::vector<std::string>* plist, const char* separators)
{
	std::string::size_type pos, endpos;

	pos = 0;

	while(pos != std::string::npos) {
		pos = in.find_first_not_of(separators, pos);
		if (pos == std::string::npos)
			break;

		endpos = in.find_first_of(separators, pos);

		plist->push_back(in.substr(pos, endpos - pos));

		pos = endpos;
	}
}

/*
 * remove spaces at the beginning or the end of a string
 */
void remove_spaces(std::string* in) {
  while(((*in)[0])==' ' || ((*in)[0])=='\t' || ((*in)[0])=='\n') 
      in->erase(0, 1);

   while(((*in)[in->size()-1])==' ' || ((*in)[in->size()-1])=='\t' || ((*in)[in->size()-1])=='\n') 
      in->erase(in->size()-1, 1);
}

/*
 * Convert this string from an ASCII to an unicode 
 * string. This function should eventually no longer be 
 * needed (when everything is unicode)
 */
std::wstring widen_string(std::string in) {
   std::wstring retval;

   for(uint i=0; i<in.size(); i++) 
      retval.append(1, in[i]);
   return retval;
}

/*
 * Convert this string from an uncicode to an ASCII 
 * string. Wide characters are silently ignored. 
 * This function should eventually no longer be used
 */
std::string narrow_string(std::wstring in) {
   std::string retval;

   for(uint i=0; i<in.size(); i++) { 
      wchar_t str = in[i];
      if(str >= 255) continue;
      retval.append(1, in[i]);
   }
   return retval;
}

/*
===============
log

Print a formatted log messages to cout.
===============
*/
void log(const char *fmt, ...)
{
	char buf[2048];
	va_list va;
	
	va_start(va, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va);
	va_end(va);

	// use iostreams instead of vprintf because other parts of Widelands use iostreams
	cout << buf;
	cout.flush();
}



/*
==============================================================================

class wexception implementation

==============================================================================
*/

wexception::wexception(const char *fmt, ...) throw()
{
	va_list va;

	va_start(va, fmt);
	vsnprintf(m_string, sizeof(m_string), fmt, va);
	va_end(va);
}

wexception::~wexception() throw()
{
}

const char *wexception::what() const throw()
{
	return m_string;
}

void myassert(int line, const char* file, const char* condt) throw(wexception)
{
	critical_error("%s:%i: assertion \"%s\" failed!\n", file, line, condt);
	throw wexception("Assertion %s:%i (%s) has been ignored", file, line, condt);
}

