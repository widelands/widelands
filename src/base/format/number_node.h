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

		if (min_width_ == 0 || (flags_ & kLeftAlign) != 0) {
			// The easy case: Just start writing.
			size_t written = 0;
			if (arg < 0) {
				if (localize) {
					for (const char* c = kLocalizedMinusSign; *c; ++c, ++out, ++written) {
						*out = *c;
					}
				} else {
					*out = '-';
					++out;
					++written;
				}
			} else if ((flags_ & kNumberSign) != 0) {
				if (localize && arg == 0) {
					for (const char* c = kDigitWidthSpace; *c; ++c, ++out, ++written) {
						*out = *c;
					}
				} else {
					*out = '+';
					++out;
					++written;
				}
			}
			if (print0x_) {
				*out = '0';
				*(out + 1) = 'x';
				out += 2;
				written += 2;
			}

			if (arg == 0) {
				*out = '0';
				++out;
				++written;
			} else {
				size_t nr_digits = 0;
				for (Number i = (arg < 0 ? -arg : arg); i; ++nr_digits, i /= base_) {
				}

				Number i = (arg < 0 ? -arg : arg);
				for (size_t d = nr_digits; d; --d, i /= base_) {
					static Number digit;
					digit = (i % base_);
					if (hexadecimal_ && digit > 9) {
						*(out + d - 1) = (uppercase_ ? 'A' : 'a') + digit - 10;
					} else {
						*(out + d - 1) = '0' + digit;
					}
				}

				out += nr_digits;
				written += nr_digits;
			}

			if (written < min_width_) {
				written = min_width_ - written;
				for (; written; ++out, --written) {
					*out = ' ';
				}
			}

			return out;
		}

		// The more complex case: We want a right-aligned string with a given minimum width,
		// padded with leading whitespace or zeroes.
		size_t nr_digits;
		if (arg == 0) {
			nr_digits = 1;
		} else {
			nr_digits = 0;
			for (Number i = (arg < 0 ? -arg : arg); i; ++nr_digits, i /= base_) {
			}
		}

		size_t required_width = nr_digits;
		if (print0x_) {
			required_width += 2;
		}
		if (arg < 0 || (flags_ & kNumberSign) != 0) {
			required_width += localize ? (arg == 0) ? kDigitWidthSpaceLength :
			                             (arg < 0)  ? kLocalizedMinusSignLength :
                                                   1 :
                                      1;
		}
		if ((flags_ & kPadWith0) == 0 && required_width < min_width_) {
			required_width = min_width_ - required_width;
			for (; required_width; ++out, --required_width) {
				*out = ' ';
			}
		}

		if (arg < 0) {
			if (localize) {
				for (const char* c = kLocalizedMinusSign; *c; ++c, ++out) {
					*out = *c;
				}
			} else {
				*out = '-';
				++out;
			}
		} else if ((flags_ & kNumberSign) != 0) {
			if (localize && arg == 0) {
				for (const char* c = kDigitWidthSpace; *c; ++c, ++out) {
					*out = *c;
				}
			} else {
				*out = '+';
				++out;
			}
		}
		if (print0x_) {
			*out = '0';
			*(out + 1) = 'x';
			out += 2;
		}

		if ((flags_ & kPadWith0) != 0 && required_width < min_width_) {
			required_width = min_width_ - required_width;
			for (; required_width; ++out, --required_width) {
				*out = '0';
			}
		}

		if (arg == 0) {
			*out = '0';
			++out;
		} else {
			Number i = (arg < 0 ? -arg : arg);
			for (size_t d = nr_digits; d; --d, i /= base_) {
				static Number digit;
				digit = (i % base_);
				if (hexadecimal_ && digit > 9) {
					*(out + d - 1) = (uppercase_ ? 'A' : 'a') + digit - 10;
				} else {
					*(out + d - 1) = '0' + digit;
				}
			}

			out += nr_digits;
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
