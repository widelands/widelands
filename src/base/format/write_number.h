/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#ifndef WL_BASE_FORMAT_WRITE_NUMBER_H
#define WL_BASE_FORMAT_WRITE_NUMBER_H

#include <cassert>

#include "base/format/abstract_node.h"

namespace format_impl {

// Always writes exactly `width` characters.
// Can be used for 0 padding right aligned (decimal part of float), but make sure that it fits!
inline char* write_digits(
   char* out, uint64_t arg, const size_t width, bool hexadecimal = false, bool uppercase = false) {
	uint64_t base = hexadecimal ? 16 : 10;

	uint64_t digit;
	for (size_t d = width; d > 0; --d) {
		digit = (arg % base);
		if (hexadecimal && digit > 9) {
			*(out + d - 1) = (uppercase ? 'A' : 'a') + digit - 10;
		} else {
			*(out + d - 1) = '0' + digit;
		}
		arg /= base;
	}
	assert(arg == 0);
	return out + width;
}

// Get the required width
inline size_t number_of_digits(uint64_t arg, bool hexadecimal = false) {
	uint64_t base = hexadecimal ? 16 : 10;

	size_t n = 1;
	for (uint64_t i = arg; i >= base; i /= base) {
		++n;
	}
	return n;
}

// Only counts as a single display character, even if it requires a UTF-8 multi-byte
// character sequence for storage.
// We return the next position, display width counter should be increased
// by 1 in caller.
inline char* write_minus_sign(char* out, bool localize) {
	if (localize) {
		for (const char* c = kLocalizedMinusSign; *c; ++c) {
			*out = *c;
			++out;
		}
	} else {
		*out = '-';
		++out;
	}
	return out;
}

// Only counts as a single display character, even if it requires a UTF-8 multi-byte
// character sequence for storage.
// We return the next position, display width counter should be increased
// by 1 in caller.
inline char* write_forced_plus_sign(char* out, bool localize, bool is_zero = false) {
	if (localize && is_zero) {
		for (const char* c = kDigitWidthSpace; *c; ++c) {
			*out = *c;
			++out;
		}
	} else {
		*out = '+';
		++out;
	}
	return out;
}

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_WRITE_NUMBER_H
