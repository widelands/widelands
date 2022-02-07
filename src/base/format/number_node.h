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

#ifndef WL_BASE_FORMAT_NUMBER_NODE_H
#define WL_BASE_FORMAT_NUMBER_NODE_H

#include "base/format/abstract_node.h"
#include "base/format/write_number.h"

namespace format_impl {

template <typename Number> struct NumberNodeT : FormatNode {
	NumberNodeT(const uint8_t f, const size_t w, const bool hex, const bool p0x, const bool uc)
	   : FormatNode(f, w, 0),
	     base_(hex ? 16 : 10),
	     hexadecimal_(hex),
	     print0x_(p0x),
	     uppercase_(uc) {
		if ((flags_ & kLeftAlign) != 0 && (flags_ & kPadWith0) != 0) {
			throw wexception("'-' and '0' can not be combined");
		}
		if (print0x_ && !hexadecimal_) {
			throw wexception("0x can only be prefixed to hexadecimal numbers");
		}
	}

	char* append(char* out, const ArgType t, Argument arg_u, const bool localize) const override {
		Number arg;
		switch (t) {
		case ArgType::kSigned:
			arg = arg_u.signed_val;
			break;
		case ArgType::kUnsigned:
		case ArgType::kPointer:
			if (arg_u.unsigned_val > static_cast<uint64_t>(kMaxInt)) {
				throw wexception("Unsigned integral value too large: %" PRIu64, arg_u.unsigned_val);
			}
			arg = arg_u.unsigned_val;
			break;
		case ArgType::kFloat:
			if (arg_u.float_val > kMaxInt || arg_u.float_val < -kMaxInt) {
				throw wexception("Floating point value too large: %f", arg_u.float_val);
			}
			arg = arg_u.float_val;
			break;
		case ArgType::kNullptr:
			return append_nullptr_node(out, localize);
		default:
			throw wexception("Wrong argument type: expected %s, found %s",
			                 (std::is_signed<Number>::value ? "int" : "unsigned"),
			                 to_string(t).c_str());
		}

		bool is_negative = false;
		if (std::is_signed<Number>::value && (arg < 0)) {
			is_negative = true;
			arg = -arg;
		}
		size_t nr_digits = number_of_digits(arg, hexadecimal_);

		size_t required_width = nr_digits;
		if (print0x_) {
			required_width += 2;
		}
		if (is_negative || (flags_ & kNumberSign) != 0) {
			++required_width;
		}

		size_t padding = 0;
		if (min_width_ > required_width) {
			padding = min_width_ - required_width;
		}

		// Right aligned, padding with spaces
		if ((flags_ & (kPadWith0 | kLeftAlign)) == 0) {
			for (; padding > 0; --padding) {
				*out = ' ';
				++out;
			}
		}

		if (is_negative) {
			out = write_minus_sign(out, localize);
		} else if ((flags_ & kNumberSign) != 0) {
			out = write_forced_plus_sign(out, localize, arg == 0);
		}
		if (print0x_) {
			*out = '0';
			*(out + 1) = 'x';
			out += 2;
		}

		if (flags_ & kPadWith0) {
			for (; padding > 0; --padding) {
				*out = '0';
				++out;
			}
		}

		out = write_digits(out, arg, nr_digits, hexadecimal_, uppercase_);

		// No need to check for left aligned: Other cases already zeroed the padding.
		for (; padding > 0; --padding) {
			*out = ' ';
			++out;
		}

		return out;
	}

	static const NumberNodeT node_;

private:
	const Number base_;
	const bool hexadecimal_, print0x_, uppercase_;
};
using IntNode = NumberNodeT<int64_t>;
using UintNode = NumberNodeT<uint64_t>;
template <typename NumberT>
const NumberNodeT<NumberT> NumberNodeT<NumberT>::node_(kNone, 0, false, false, false);
extern const UintNode pointer_node_;

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_NUMBER_NODE_H
