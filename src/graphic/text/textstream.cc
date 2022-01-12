/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#include "base/string.h"
#include "graphic/text/rt_errors_impl.h"

namespace RT {

struct EndOfTextImpl : public EndOfText {
	EndOfTextImpl(size_t pos, const std::string& text)
	   : EndOfText(format("Unexpected End of Text, starting at %1%. Text is: '%2%'", pos, text)) {
	}
};

void TextStream::consume(size_t cnt) {
	while (cnt) {
		if (text_[pos_] == '\n') {
			++line_;
			col_ = 0;
		} else {
			++col_;
		}
		++pos_;
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
	while (pos_ < end_ && isspace(text_[pos_])) {
		consume(1);
	}
}
void TextStream::rskip_ws() {
	while (pos_ < end_ && isspace(text_[end_ - 1])) {
		--end_;
	}
}

/*
 * Return the next few characters without advancing the stream
 */
std::string TextStream::peek(size_t n, size_t at) const {
	return text_.substr(at > text_.size() ? pos_ : at, n);
}

/*
 * Throw a synatx error if not the thing shows up, we expected to.
 * Also advances the stream.
 */
void TextStream::expect(std::string n, bool skip_whitespace) {
	if (skip_whitespace) {
		skip_ws();
	}

	if (peek(n.size()) != n) {
		throw SyntaxErrorImpl(line_, col_, format("'%s'", n), peek(n.size()), peek(100));
	}
	consume(n.size());
}

/*
 * Parse forward till the next char is any of of the given chars.
 * Return the substring we went over
 */
std::string TextStream::till_any(std::string chars) {
	// Sticking with a double loop because chars will likely be short
	std::string rv;

	size_t j = pos_;
	size_t started_at = pos_;
	bool found = false;
	while (j < end_) {
		for (char& ch : chars) {
			if (ch == text_[j]) {
				found = true;
				break;
			}
		}
		if (found) {
			break;
		}

		// Get rid of control characters
		// https://en.cppreference.com/w/cpp/language/escape
		switch (text_[j]) {
		case '\a':
		case '\b':
		case '\f':
		case '\v':
			++j;
			break;
		default:
			break;
		}

		rv += text_[j];
		++j;
	}
	if (!found) {
		throw EndOfTextImpl(started_at, peek(100, started_at));
	}
	consume(j - started_at);

	// Undo the extra \ that were inserted in Parser::parse to prevent crashes.
	replace_all(rv, "\\\\", "\\");

	return rv;
}

/*
 * Parse till any of the chars is found or the end of the string has been hit.
 */
std::string TextStream::till_any_or_end(const std::string& chars) {
	std::string rv;
	try {
		rv = till_any(chars);
	} catch (EndOfTextImpl&) {
		rv = text_.substr(pos_, end_ - pos_);
		consume(end_ + 1 - pos_);
	}
	return rv;
}

/*
 * Return the next (potentially quoted) string
 */
std::string TextStream::parse_string() {
	std::string delim = peek(1);
	if (delim == "'" || delim == "\"") {
		consume(1);
		std::string rv = till_any(delim);
		consume(1);
		return rv;
	} else {
		return till_any(" \t>");
	}
}

/*
 * Return the text that is yet to be parsed
 */
std::string TextStream::remaining_text() {
	return text_.substr(pos_, end_ - pos_);
}
}  // namespace RT
