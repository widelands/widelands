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

#ifndef TEXTSTREAM_H
#define TEXTSTREAM_H

#include <string>

#include <stdint.h>

namespace RT {

class TextStream {
public:
	TextStream(std::string text) : m_t(text), m_lineno(1), m_col(0), m_i(0), m_end(text.size()) {}

	size_t line() const {return m_lineno;}
	size_t col() const {return m_col;}
	size_t pos() const {return m_i;}

	std::string peek(size_t, size_t = -1) const;
	void expect(std::string, bool = true);

	std::string till_any(std::string);
	std::string till_any_or_end(std::string);
	std::string parse_string();

	void skip(size_t d) {m_i += d;}
	void skip_ws();
	void rskip_ws();

	std::string remaining_text();

private:
	void m_consume(size_t);
	std::string m_t;
	uint32_t m_lineno, m_col;
	size_t m_i, m_end;
};

}
#endif /* end of include guard: TEXTSTREAM_H */

