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

/**
 * remove spaces at the beginning or the end of a string
 */
void remove_spaces(string& s) {
	while (s[0] == ' ' or s[0] == '\t' or s[0] == '\n')
		s.erase(0, 1);

	while (*s.rbegin() == ' ' or * s.rbegin() == '\t' or * s.rbegin() == '\n')
		s.erase(s.size() - 1, 1);
}

/**
 * Tell whether a SDL_Keysym is printable.
 *
 * \param k SDL_Keysym to be checked for printability
 *
 * \return True if k is a printable character
 *
 * \todo This is _by_far_ not complete enough
 * \todo Should be based on k.unicode (already enabled by
 * WLApplication::init_hardware()) instead of k.sym. Doing so needs to take the
 * current locale into account; perhaps there already is a fitting gettext
 * function?
 */
bool is_printable(SDL_keysym k) {
	return (k.sym == SDLK_TAB) || ((k.sym >= SDLK_SPACE) && (k.sym <= SDLK_z)) ||
	       ((k.sym >= SDLK_WORLD_0) && (k.sym <= SDLK_WORLD_95)) ||
	       ((k.sym >= SDLK_KP0) && (k.sym <= SDLK_KP_EQUALS));
}

/**
 * Implemantation for NumberGlob.
 */
NumberGlob::NumberGlob(const string& pictmp) : templ_(pictmp), cur_(0) {
	int nchars = count(pictmp.begin(), pictmp.end(), '?');
	fmtstr_ = "%0" + boost::lexical_cast<string>(nchars) + "i";

	max_ = 1;
	for (int i = 0; i < nchars; ++i) {
		max_ *= 10;
		replstr_ += "?";
	}
	max_ -= 1;
}

bool NumberGlob::next(string* s) {
	if (cur_ > max_)
		return false;

	if (max_) {
		*s = boost::replace_last_copy(templ_, replstr_, (boost::format(fmtstr_) % cur_).str());
	} else {
		*s = templ_;
	}
	++cur_;
	return true;
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

char* next_word(char*& p, bool& reached_end, char const terminator) {
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

bool match(char*& candidate, const char* pattern) {
	for (char* p = candidate;; ++p, ++pattern)
		if (not * pattern) {
			candidate = p;
			return true;
		} else if (*p != *pattern)
			break;
	return false;
}

bool skip(char*& p, char const c) {
	char* t = p;
	while (*t == c)
		++t;
	if (p < t) {
		p = t;
		return true;
	} else
		return false;
}

void force_skip(char*& p, char const c) {
	char* t = p;
	while (*t == c)
		++t;
	if (p < t)
		p = t;
	else
		throw wexception("expected '%c' but found \"%s\"", c, p);
}

bool match_force_skip(char*& candidate, const char* pattern) {
	for (char* p = candidate;; ++p, ++pattern)
		if (not * pattern) {
			force_skip(p);
			candidate = p;
			return true;
		} else if (*p != *pattern)
			return false;

	return false;
}
