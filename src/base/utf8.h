/*
 * Copyright (C) 2011-2017 by the Widelands Development Team
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
	 * Convert a unicode character into a multi-byte utf8 string.
	 */
	static std::string unicode_to_utf8(uint16_t unicode) {
		unsigned char buf[4];

		if (unicode < 0x80) {
			buf[0] = unicode;
			buf[1] = 0;
		} else if (unicode < 0x800) {
			buf[0] = ((unicode & 0x7c0) >> 6) | 0xc0;
			buf[1] = (unicode & 0x3f) | 0x80;
			buf[2] = 0;
		} else {
			buf[0] = ((unicode & 0xf000) >> 12) | 0xe0;
			buf[1] = ((unicode & 0xfc0) >> 6) | 0x80;
			buf[2] = (unicode & 0x3f) | 0x80;
			buf[3] = 0;
		}

		return reinterpret_cast<char*>(buf);
	}

	/**
	 * Decode the unicode character starting at \p pos and return it. Upon returning,
	 * \p pos will point to the beginning of the next unicode character.
	 * Return 0 on decoding errors.
	 */
	static uint16_t utf8_to_unicode(const std::string& in, std::string::size_type& pos) {
		assert(pos < in.size());
		if (in[pos] & 0xc0) {
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
