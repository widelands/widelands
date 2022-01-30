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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_TEXT_TEXTSTREAM_H
#define WL_GRAPHIC_TEXT_TEXTSTREAM_H

#include <string>

namespace RT {

class TextStream {
public:
	explicit TextStream(const std::string& text)
	   : text_(text), line_(1), col_(0), pos_(0), end_(text.size()) {
	}

	size_t line() const {
		return line_;
	}
	size_t col() const {
		return col_;
	}
	size_t pos() const {
		return pos_;
	}

	std::string peek(size_t, size_t = -1) const;
	void expect(std::string, bool = true);

	std::string till_any(std::string);
	std::string till_any_or_end(const std::string&);
	std::string parse_string();

	void skip(size_t d) {
		pos_ += d;
	}
	void skip_ws();
	void rskip_ws();

	std::string remaining_text();

private:
	void consume(size_t);
	std::string text_;
	uint32_t line_, col_;
	size_t pos_, end_;
};
}  // namespace RT
#endif  // end of include guard: WL_GRAPHIC_TEXT_TEXTSTREAM_H
