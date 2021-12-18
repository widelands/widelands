/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#ifndef WL_BASE_FORMAT_FLOAT_NODE_H
#define WL_BASE_FORMAT_FLOAT_NODE_H

#include <cstring>

#include "base/format/abstract_node.h"

namespace format_impl {

struct FloatNode : FormatNode {
	FloatNode(const uint8_t f, const size_t w, const int32_t p) : FormatNode(f, w, p) {
	}

	char* append(char* out, const ArgType t, Argument arg_u, const bool localize) const override {
		double arg;
		switch (t) {
		case ArgType::kFloat:
			if (arg_u.float_val > kMaxInt || arg_u.float_val < -kMaxInt) {
				throw wexception("Floating point value too large: %f", arg_u.float_val);
			}
			arg = arg_u.float_val;
			break;
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
		case ArgType::kNullptr:
			return append_nullptr_node(out, localize);
		default:
			throw wexception(
			   "Wrong argument type: expected float/double, found %s", to_string(t).c_str());
		}

		double rounding = 0.5;
		for (unsigned p = precision_; p; --p) {
			rounding /= 10.;
		}
		int64_t as_int;
		if (arg < 0) {
			arg -= rounding;
			as_int = static_cast<int64_t>(-arg);
		} else {
			arg += rounding;
			as_int = static_cast<int64_t>(arg);
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
				*out = '+';
				++out;
				++written;
			}

			if (as_int == 0) {
				*out = '0';
				++out;
				++written;
			} else {
				size_t nr_digits = 0;
				for (int64_t i = as_int; i; ++nr_digits, i /= 10) {
				}

				int64_t i = as_int;
				for (size_t d = nr_digits; d; --d, i /= 10) {
					*(out + d - 1) = '0' + (i % 10);
				}

				out += nr_digits;
				written += nr_digits;
			}

			if (precision_ > 0) {
				if (localize) {
					for (const char* decimal_sep = pgettext("decimal_separator", "."); *decimal_sep;
					     ++decimal_sep, ++out, ++written) {
						*out = *decimal_sep;
					}
				} else {
					*out = '.';
					++out;
					++written;
				}

				// Now write the decimals
				double decimal_part = (arg < 0 ? -arg : arg) - as_int;
				bool any_decimal_written = 0;
				for (size_t remaining_max_decimals = precision_;
				     remaining_max_decimals > 0 && decimal_part > 0;
				     --remaining_max_decimals, ++out, ++written, any_decimal_written = true) {
					decimal_part *= 10;
					static int64_t digit;
					digit = static_cast<int64_t>(decimal_part);
					*out = '0' + digit;
					decimal_part -= digit;
				}
				if (!any_decimal_written) {
					*out = '0';
					++out;
					++written;
				}
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
		const char* decimal_sep =
		   precision_ > 0 ? localize ? pgettext("decimal_separator", ".") : "." : "";
		const size_t decimal_sep_len = strlen(decimal_sep);
		size_t nr_digits_before_decimal;
		size_t nr_digits_after_decimal = 0;
		if (as_int == 0) {
			nr_digits_before_decimal = 1;
		} else {
			nr_digits_before_decimal = 0;
			for (int64_t i = as_int; i; ++nr_digits_before_decimal, i /= 10) {
			}
		}
		{
			double decimal_part = (arg < 0 ? -arg : arg) - as_int;
			for (size_t remaining_max_decimals = precision_;
			     remaining_max_decimals > 0 && decimal_part > 0;
			     --remaining_max_decimals, ++nr_digits_after_decimal,
			            decimal_part = (decimal_part * 10 - static_cast<int64_t>(decimal_part))) {
			}
			if (nr_digits_after_decimal == 0 && precision_ > 0) {
				nr_digits_after_decimal = 1;
			}
		}

		size_t required_width = nr_digits_before_decimal + decimal_sep_len + nr_digits_after_decimal;
		if (arg < 0 || (flags_ & kNumberSign) != 0) {
			required_width += (localize && arg < 0 ? kLocalizedMinusSignLength : 1);
		}

		// Start writing
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
			*out = '+';
			++out;
		}

		if ((flags_ & kPadWith0) != 0 && required_width < min_width_) {
			required_width = min_width_ - required_width;
			for (; required_width; ++out, --required_width) {
				*out = '0';
			}
		}

		if (as_int == 0) {
			*out = '0';
			++out;
		} else {
			int64_t i = as_int;
			for (size_t d = nr_digits_before_decimal; d; --d, i /= 10) {
				*(out + d - 1) = '0' + (i % 10);
			}
			out += nr_digits_before_decimal;
		}

		if (precision_ > 0) {
			for (size_t l = decimal_sep_len; l; --l, ++out, ++decimal_sep) {
				*out = *decimal_sep;
			}

			// Now the decimals
			double decimal_part = (arg < 0 ? -arg : arg) - as_int;
			bool any_decimal_written = 0;
			for (size_t remaining_max_decimals = precision_;
			     remaining_max_decimals > 0 && decimal_part > 0;
			     --remaining_max_decimals, ++out, any_decimal_written = true) {
				decimal_part *= 10;
				static int64_t digit;
				digit = static_cast<int64_t>(decimal_part);
				*out = '0' + digit;
				decimal_part -= digit;
			}
			if (!any_decimal_written) {
				*out = '0';
				++out;
			}
		}

		return out;
	}

	static const FloatNode node_;
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_FLOAT_NODE_H
