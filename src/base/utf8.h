/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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

#ifndef WL_BASE_UTF8_H
#define WL_BASE_UTF8_H

#include <cassert>
#include <string>

struct Utf8 {
	/**
	 * Return @c true iff the given character is an extended byte of
	 * a multi-byte UTF8 character. This function returns false for
	 * the first byte of a multi-byte UTF8 character.
	 */
	static bool is_utf8_extended(char ch) {
		return (ch & 0xc0) == 0x80;
	}

	/**
	 * Decode the unicode character starting at \p pos and return it. Upon returning,
	 * \p pos will point to the beginning of the next unicode character.
	 * Return 0 on decoding errors.
	 */
	static uint16_t utf8_to_unicode(const std::string& in, std::string::size_type& pos) {
		assert(pos < in.size());
		if (in[pos] & 0x80) {
			if (is_utf8_extended(in[pos])) {
				pos++;
				return 0;
			}

			uint8_t ctrl = in[pos];
			uint16_t value = in[pos++];
			if (!is_utf8_extended(in[pos]))
				return 0;
			value = (value << 6) | (in[pos++] & 0x3f);
			if ((ctrl & 0xe0) == 0xc0)
				return value & 0x07ff;
			if (!is_utf8_extended(in[pos]))
				return 0;
			value = (value << 6) | (in[pos++] & 0x3f);
			if ((ctrl & 0xf0) == 0xe0)
				return value & 0xffff;
			while (is_utf8_extended(in[pos]))
				++pos;
			return 0;
		} else {
			return in[pos++];
		}
	}
};

#endif  // end of include guard: WL_BASE_UTF8_H
