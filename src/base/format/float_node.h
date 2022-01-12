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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_BASE_FORMAT_FLOAT_NODE_H
#define WL_BASE_FORMAT_FLOAT_NODE_H

#include <cstring>

#include "base/format/abstract_node.h"
#include "base/format/write_number.h"

namespace format_impl {

// We assume that it is a single displayed character, but we allow it to be
// stored as a multi-byte UTF-8 sequence.
inline char* write_decimal_separator(char* out, bool localize) {
	if (localize) {
		for (const char* dec_sep = pgettext("decimal_separator", "."); *dec_sep; ++dec_sep) {
			*out = *dec_sep;
			++out;
		}
	} else {
		*out = '.';
		++out;
	}
	return out;
}

struct FloatNode : FormatNode {
	FloatNode(const uint8_t f, const size_t w, const int32_t p, bool dyn_prec)
	   : FormatNode(f, w, p), dynamic_precision_(dyn_prec) {
		rounding_ = 0.5;
		precision_multiplier_ = 1.;
		for (unsigned i = precision_; i > 0; --i) {
			rounding_ /= 10.;
			precision_multiplier_ *= 10.;
		}
		check_int_extra_precision_ = kDynamicPrecisionExtra / precision_multiplier_;
	}

	char* append(char* out, const ArgType t, Argument arg_u, const bool localize) const override {
		int64_t int_part;
		int64_t fractional = 0;
		bool is_int = true;
		bool is_negative = false;
		bool is_zero = false;
		size_t current_precision = precision_;

		switch (t) {
		case ArgType::kFloat:
			double rounded;
			double frac;

			is_negative = (arg_u.float_val < 0);
			if (is_negative) {
				rounded = -arg_u.float_val;
			} else {
				rounded = arg_u.float_val;
			}
			is_zero = (rounded < check_int_extra_precision_) && ((flags_ & kNumberSign) != 0);
			is_negative = is_negative && !is_zero;
			rounded += rounding_;

			if (rounded > kMaxInt) {
				throw wexception("Floating point value too large: %f", arg_u.float_val);
			}
			int_part = static_cast<int64_t>(rounded);

			frac = (rounded - int_part) * precision_multiplier_;
			fractional = static_cast<int64_t>(frac);
			is_int = (fractional == 0);
			if (dynamic_precision_) {
				if (is_int) {
					if (frac < check_int_extra_precision_) {
						current_precision = 0;
					}
				} else if
				     // Undo rounding and check that next decimal digits would also be 0.
				     (abs(frac - fractional - 0.5) < kDynamicPrecisionExtra) {
					int64_t f = fractional;
					while ((f % 10 == 0) && (current_precision > 0)) {
						f /= 10;
						--current_precision;
					}
				}
			}
			break;
		case ArgType::kSigned:
			int_part = arg_u.signed_val;
			is_negative = (int_part < 0);
			is_zero = (int_part == 0);
			if (dynamic_precision_) {
				current_precision = 0;
			}
			break;
		case ArgType::kUnsigned:
		case ArgType::kPointer:
			if (arg_u.unsigned_val > static_cast<uint64_t>(kMaxInt)) {
				throw wexception("Unsigned integral value too large: %" PRIu64, arg_u.unsigned_val);
			}
			int_part = arg_u.unsigned_val;
			is_zero = (int_part == 0);
			if (dynamic_precision_) {
				current_precision = 0;
			}
			break;
		case ArgType::kNullptr:
			return append_nullptr_node(out, localize);
		default:
			throw wexception(
			   "Wrong argument type: expected float/double, found %s", to_string(t).c_str());
		}

		if (min_width_ == 0 || (flags_ & kLeftAlign) != 0) {
			// The easy case: Just start writing.
			size_t written = 0;

			// Write the sign
			if (is_negative) {
				out = write_minus_sign(out, localize);
				++written;
			} else if ((flags_ & kNumberSign) != 0) {
				out = write_forced_plus_sign(out, localize, is_zero);
				++written;
			}

			// Write the integer part
			written = write_digits(out, int_part);
			out += written;

			if (current_precision > 0) {
				out = write_decimal_separator(out, localize);
				++written;

				// Write the decimals
				write_digits_w(out, fractional, current_precision);
				out += current_precision;
				written += current_precision;
			}

			for (size_t padding = min_width_ - written; padding > 0; --padding) {
				*out = ' ';
				++out;
			}

			return out;
		}

		// The more complex case: We want a right-aligned string with a given minimum width,
		// padded with leading whitespace or zeroes. So we need the width first:
		size_t nr_digits_before_decimal = number_of_digits(int_part);
		size_t required_width = nr_digits_before_decimal +
		                        current_precision > 0 ? current_precision + 1 : 0;
		if (is_negative || (flags_ & kNumberSign) != 0) {
			++required_width;
		}

		// Pad with spaces as needed
		if ((flags_ & kPadWith0) == 0) {
			for (size_t padding = min_width_ - required_width; padding > 0; --padding) {
				*out = ' ';
				++out;
			}
		}

		if (is_negative) {
			out = write_minus_sign(out, localize);
		} else if ((flags_ & kNumberSign) != 0) {
			out = write_forced_plus_sign(out, localize, is_zero);
		}

		// Pad with zeroes as needed
		if ((flags_ & kPadWith0) != 0) {
			for (size_t padding = min_width_ - required_width; padding > 0; --padding) {
				*out = '0';
				++out;
			}
		}

		// Write the integer part
		write_digits_w(out, int_part, nr_digits_before_decimal);
		out += nr_digits_before_decimal;

		if (current_precision > 0) {
			out = write_decimal_separator(out, localize);

			// Write the decimals
			write_digits_w(out, fractional, current_precision);
			out += current_precision;
		}

		return out;
	}

	static const FloatNode node_;

private:
	bool dynamic_precision_;
	double rounding_;
	double precision_multiplier_;
	double check_int_extra_precision_;
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_FLOAT_NODE_H
