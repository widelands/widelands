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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "helper.h"

#include <cstdarg>
#include <memory>
#include <string>

#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>

using namespace std;

/// Split a string by separators.
/// \note This ignores empty elements, so do not use this for example to split
/// a string with newline characters into lines, because it would ignore empty
/// lines.
vector<string> split_string(const string& s, const char* const separators) {
	vector<string> result;
	for (string::size_type pos = 0, endpos;
	     (pos = s.find_first_not_of(separators, pos)) != string::npos;
	     pos = endpos) {
		endpos = s.find_first_of(separators, pos);
		result.push_back(s.substr(pos, endpos - pos));
	}
	return result;
}

static boost::mt19937 random_generator;
string random_string(const string& chars, int nlen) {
	boost::uniform_int<> index_dist(0, chars.size() - 1);
	std::unique_ptr<char[]> buffer(new char[nlen]);
	for (int i = 0; i < nlen; ++i) {
		buffer[i] = chars[index_dist(random_generator)];
	}
	return string(buffer.get(), nlen);
}

char* next_word(char* & p, bool& reached_end, char const terminator) {
	assert(terminator);
	char* const result = p;
	for (; *p != terminator; ++p)
		if (*p == '\0') {
			reached_end = true;
			goto end;
		}
	reached_end = false;
	*p = '\0';  //  terminate the word
	++p;        //  move past the terminator
end:
	if (result < p)
		return result;
	throw wexception("expected word");
}
