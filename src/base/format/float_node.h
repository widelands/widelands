/*
 * Copyright (C) 2021-2023 by the Widelands Development Team
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
		for (const char* dec_sep = pgettext("decimal_separator", "."); *dec_sep != 0; ++dec_sep) {
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
		check_zero_extra_precision_ = kDynamicPrecisionExtra / precision_multiplier_;
	}

	char* append(char* out, const ArgType t, Argument arg_u, const bool localize) const override {
		int64_t int_part;
		int64_t fractional = 0;
		bool is_negative = false;
		bool is_zero = false;
		size_t current_precision = precision_;

		switch (t) {
		case ArgType::kFloat: {
			double rounded;
			double frac;

			if (arg_u.float_val < 0) {
				is_negative = true;
				rounded = -arg_u.float_val;
			} else {
				rounded = arg_u.float_val;
			}

			if ((flags_ & kNumberSign) != 0) {
				is_zero = (rounded < check_zero_extra_precision_);
				is_negative = is_negative && !is_zero;
			}

			rounded += rounding_;
			if (rounded > kMaxInt) {
				throw wexception("Floating point value too large: %f", arg_u.float_val);
			}
			int_part = static_cast<int64_t>(rounded);

			frac = (rounded - int_part) * precision_multiplier_;
			fractional = static_cast<int64_t>(frac);
			if (dynamic_precision_) {
				while ((fractional % 10 == 0) && (current_precision > 1)) {
					fractional /= 10;
					--current_precision;
				}
			}
		} break;
		case ArgType::kSigned:
			int_part = arg_u.signed_val;
			if (int_part < 0) {
				is_negative = true;
				int_part = -int_part;
			}
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

		size_t nr_digits_before_decimal = number_of_digits(int_part);
		size_t required_width =
		   nr_digits_before_decimal + (current_precision > 0 ? current_precision + 1 : 0);
		if (is_negative || (flags_ & kNumberSign) != 0) {
			++required_width;
		}
		size_t padding = 0;
		if (min_width_ > required_width) {
			padding = min_width_ - required_width;
		}

		// Right aligned, padding with spaces
		if ((flags_ & (kPadWith0 | kLeftAlign)) == 0) {
			out = pad_with_space(out, padding, localize);
		}

		if (is_negative) {
			out = write_minus_sign(out, localize);
		} else if ((flags_ & kNumberSign) != 0) {
			out = write_forced_plus_sign(out, localize, is_zero);
		}

		// Pad with zeroes as needed
		if ((flags_ & kPadWith0) != 0) {
			out = pad_with_char(out, padding, '0');
		}

		// Write the integer part
		out = write_digits(out, int_part, nr_digits_before_decimal);

		if (current_precision > 0) {
			out = write_decimal_separator(out, localize);

			// Write the decimals
			out = write_digits(out, fractional, current_precision);
		}

		if ((flags_ & kLeftAlign) != 0) {
			out = pad_with_space(out, padding, localize);
		}

		return out;
	}

	static const FloatNode node_;

private:
	bool dynamic_precision_;
	double rounding_;
	double precision_multiplier_;
	double check_zero_extra_precision_;
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_FLOAT_NODE_H
