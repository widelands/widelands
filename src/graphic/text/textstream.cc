/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include "graphic/text/textstream.h"

#include <boost/format.hpp>

#include "graphic/text/rt_errors_impl.h"

using namespace std;
using namespace boost;

namespace RT {

struct EOT_Impl : public EOT {
	EOT_Impl(size_t pos, string text)
		: EOT((format("Unexpected End of Text, starting at %1%. Text is: '%2%'") % pos % text).str())
	{}
};

void TextStream::m_consume(size_t cnt) {
	while (cnt) {
		if (m_t[m_i] == '\n') {
			++m_lineno;
			m_col = 0;
		} else
			++m_col;
		++m_i;
		--cnt;
	}
}

/*
 * Skip over whitespace or the given string (which must
 * appear in the stream right at this point in time)
 *
 * r* means skip_ws starting from the back of the string
 */
void TextStream::skip_ws() {
	while (m_i < m_end and isspace(m_t[m_i]))
		m_consume(1);
}
void TextStream::rskip_ws() {
	while (m_i < m_end and isspace(m_t[m_end - 1]))
		--m_end;
}

/*
 * Return the next few characters without advancing the stream
 */
string TextStream::peek(size_t n, size_t at) const {
	return m_t.substr(at > m_t.size() ? m_i : at, n);
}

/*
 * Throw a synatx error if not the thing shows up, we expected to.
 * Also advances the stream.
 */
void TextStream::expect(string n, bool skip_whitespace) {
	if (skip_whitespace)
		skip_ws();

	if (peek(n.size()) != n)
		throw SyntaxError_Impl(m_lineno, m_col, (format("'%s'") % n).str(), peek(n.size()), peek(100));
	m_consume(n.size());
}

/*
 * Parse forward till the next char is any of of the given chars.
 * Return the substring we went over
 */
string TextStream::till_any(string chars) {
	// Boost should provide a function here, but I was unable to figure it out
	// Sticking with a double loop because chars will likely be short
	string rv;

	size_t j = m_i;
	size_t started_at = m_i;
	bool found = false;
	while (j < m_end) {
		for (size_t k = 0; k < chars.size(); ++k) {
			if (chars[k] == m_t[j]) {
				found = true;
				break;
			}
		}
		if (found) break;

		if (m_t[j] == '\\')
			++j;
		rv += m_t[j];
		++j;
	}
	if (!found)
		throw EOT_Impl(started_at, peek(100, started_at));
	m_consume(j - started_at);

	return rv;
}

/*
 * Parse till any of the chars is found or the end of the string has been hit.
 */
string TextStream::till_any_or_end(string chars) {
	string rv;
	try {
		rv = till_any(chars);
	} catch (EOT_Impl &) {
		rv = m_t.substr(m_i, m_end - m_i);
		m_consume(m_end + 1 - m_i);
	}
	return rv;
}

/*
 * Return the next (potentially quoted) string
 */
string TextStream::parse_string() {
	string delim = peek(1);
	if (delim == "'" or delim == "\"") {
		m_consume(1);
		string rv = till_any(delim);
		m_consume(1);
		return rv;
	} else
		return till_any(" \t>");
}

/*
 * Return the text that is yet to be parsed
 */
string TextStream::remaining_text() {
	return m_t.substr(m_i, m_end - m_i);
}

}

