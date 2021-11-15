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

#ifndef WL_BASE_FORMAT_IMPL_H
#define WL_BASE_FORMAT_IMPL_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"

namespace format_impl {

/**
   General syntax of format specifiers:
      %N%
   OR
      [ N$ ] [ flags ] [ width ] [ . precision ] fmt

   flags: Any combination of
      -    Left align
      +    Show number sign for non-negative numbers
      0    pad with zeros instead of whitespace

      Note that - and 0 can not be combined.

   width: minimal number of characters

   precision:
      For floating-point, max number of digits after the period.
      For strings, max number of characters before padding.

   fmt:
      %                   percent sign                No arguments allowed
      c                   character                   No arguments allowed
      s                   string                      Can not have + or 0
      b                   boolean                     Similar to string
      i,d,li,ld,lli,lld   int
      u,lu,llu            unsigned int
      x                   hexadecimal int (lowercase)
      X                   hexadecimal int (uppercase)
      p                   unsigned hexadecimal int (lowercase)
      P                   unsigned hexadecimal int (uppercase)
      f                   float
*/

enum Flags : uint8_t {
	kNone = 0,
	kLeftAlign = 1,
	kNumberSign = 2,
	kPadWith0 = 4,
};

struct CharNode;
struct StringNode;
struct BooleanNode;
struct FloatNode;

struct AbstractNode {
	virtual ~AbstractNode() {
	}

	union Argument {
		char char_val;
		const char* string_val;
		double float_val;
		bool boolean_val;
		int64_t signed_val;
		uint64_t unsigned_val;
	};
	enum class ArgType {
		kNone,
		kChar,
		kString,
		kBoolean,
		kFloat,
		kSigned,
		kUnsigned,
		kPointer,
		kNullptr
	};

	virtual char* append(char* out, ArgType, Argument, bool localize) const = 0;
};

using ArgsPair = std::pair<AbstractNode::ArgType, AbstractNode::Argument>;
using ArgsVector = std::vector<ArgsPair>;
std::string to_string(AbstractNode::ArgType);

struct LiteralNode : AbstractNode {
	LiteralNode(const std::string& str) : content_(str), len_(content_.size()) {
	}

	inline char* append(char* out, const ArgType t, Argument, bool) const override {
		if (t != ArgType::kNone) {
			throw wexception("Attempt to call a literal node with a value");
		}
		const char* it = content_.c_str();
		for (size_t l = len_; l; --l, ++out, ++it) {
			*out = *it;
		}
		return out;
	}

private:
	const std::string content_;
	const size_t len_;
};

struct PercentNode : AbstractNode {
	inline char* append(char* out, const ArgType t, Argument, bool) const override {
		if (t != ArgType::kNone) {
			throw wexception("Attempt to call a percent node with a value");
		}
		*out = '%';
		return out + 1;
	}
};

constexpr unsigned kInfinitePrecision = std::numeric_limits<unsigned>::max();
constexpr unsigned kDefaultFloatPrecision = 6;

struct FormatNode : AbstractNode {
protected:
	FormatNode(const uint8_t f, const size_t w, const int32_t p)
	   : flags_(f), min_width_(w), precision_(p) {
	}

	const uint8_t flags_;       ///< Bitset of flags
	const size_t min_width_;    ///< Minimum number of characters to write; may be 0
	const unsigned precision_;  ///< See above; may be kInfinitePrecision
};

struct CharNode : FormatNode {
	CharNode() : FormatNode(kNone, 0, 0) {
	}

	inline char* append(char* out, const ArgType t, const Argument arg, bool) const override {
		if (t != ArgType::kChar) {
			throw wexception("Wrong argument type: excepted char, found %s", to_string(t).c_str());
		}
		*out = arg.char_val;
		return out + 1;
	}

	static const CharNode node_;
};

struct StringNode : FormatNode {
	StringNode(const uint8_t f, const size_t w, const int32_t p) : FormatNode(f, w, p) {
		if (flags_ & kNumberSign) {
			throw wexception("%%s can not have '+' specifier");
		}
		if (flags_ & kPadWith0) {
			throw wexception("%%s can not have '0' specifier");
		}
	}

	char* append(char* out, const ArgType t, const Argument arg_u, bool) const override {
		if (t != ArgType::kString) {
			throw wexception("Wrong argument type: excepted string, found %s", to_string(t).c_str());
		}
		const char* arg = arg_u.string_val;

		if (min_width_ == 0 || flags_ & kLeftAlign) {
			// The easy case: Just start writing.
			size_t written = 0;
			for (const char* a = arg; *a && written < precision_; ++a, ++out, ++written) {
				*out = *a;
			}
			if (written < min_width_) {
				written = min_width_ - written;
				for (; written; ++out, --written) {
					*out = ' ';
				}
			}
			return out;
		}

		// The more complex case: We want a right-aligned string with a given minimum width.
		const size_t arg_len = std::min<size_t>(precision_, strlen(arg));
		if (arg_len < min_width_) {
			for (size_t i = min_width_ - arg_len; i; ++out, --i) {
				*out = ' ';
			}
		}
		for (size_t l = arg_len; l; --l, ++out, ++arg) {
			*out = *arg;
		}
		return out;
	}

	static const StringNode node_;
};

struct BooleanNode : StringNode {
	BooleanNode(const uint8_t f, const size_t w, const int32_t p) : StringNode(f, w, p) {
	}

	char* append(char* out, const ArgType t, Argument arg, const bool localize) const override {
		bool val;
		switch (t) {
		case ArgType::kBoolean:
			val = arg.boolean_val;
			break;
		case ArgType::kSigned:
			val = arg.signed_val != 0;
			break;
		case ArgType::kUnsigned:
		case ArgType::kPointer:
			val = arg.unsigned_val != 0;
			break;
		case ArgType::kNullptr:
			val = false;
			break;
		case ArgType::kChar:
			val = arg.char_val != 0;
			break;
		default:
			throw wexception("Wrong argument type: excepted bool, found %s", to_string(t).c_str());
		}
		arg.string_val = localize ? val ? _("true") : _("false") : val ? "true" : "false";
		return StringNode::append(out, ArgType::kString, arg, localize);
	}

	static const BooleanNode node_;
};

template <typename Number> struct NumberNodeT : FormatNode {
	NumberNodeT(const uint8_t f, const size_t w, const bool hex, const bool uc)
	   : FormatNode(f, w, 0), base_(hex ? 16 : 10), hexadecimal_(hex), uppercase_(uc) {
		if ((flags_ & kLeftAlign) && (flags_ & kPadWith0)) {
			throw wexception("'-' and '0' can not be combined");
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
			arg = arg_u.unsigned_val;
			break;
		case ArgType::kFloat:
			arg = arg_u.float_val;
			break;
		case ArgType::kNullptr:
			arg_u.string_val = "nullptr";
			return StringNode::node_.append(out, AbstractNode::ArgType::kString, arg_u, localize);
		default:
			throw wexception("Wrong argument type: excepted %s, found %s",
			                 (std::is_signed<Number>::value ? "int" : "unsigned"),
			                 to_string(t).c_str());
		}

		if (min_width_ == 0 || flags_ & kLeftAlign) {
			// The easy case: Just start writing.
			size_t written = 0;
			if (arg < 0) {
				*out = '-';
				++out;
				++written;
			} else if (flags_ & kNumberSign) {
				*out = '+';
				++out;
				++written;
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
		if (arg < 0 || flags_ & kNumberSign) {
			++required_width;
		}
		if (required_width < min_width_) {
			required_width = min_width_ - required_width;
			for (; required_width; ++out, --required_width) {
				*out = (flags_ & kPadWith0 ? '0' : ' ');
			}
		}

		if (arg < 0) {
			*out = '-';
			++out;
		} else if (flags_ & kNumberSign) {
			*out = '+';
			++out;
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
	const bool hexadecimal_, uppercase_;
};
using IntNode = NumberNodeT<int64_t>;
using UintNode = NumberNodeT<uint64_t>;
template <typename NumberT>
const NumberNodeT<NumberT> NumberNodeT<NumberT>::node_(kNone, 0, false, false);

struct FloatNode : FormatNode {
	FloatNode(const uint8_t f, const size_t w, const int32_t p) : FormatNode(f, w, p) {
	}

	char* append(char* out, const ArgType t, Argument arg_u, const bool localize) const override {
		double arg;
		switch (t) {
		case ArgType::kFloat:
			arg = arg_u.float_val;
			break;
		case ArgType::kSigned:
			arg = arg_u.signed_val;
			break;
		case ArgType::kUnsigned:
		case ArgType::kPointer:
			arg = arg_u.unsigned_val;
			break;
		case ArgType::kNullptr:
			arg_u.string_val = "nullptr";
			return StringNode::node_.append(out, AbstractNode::ArgType::kString, arg_u, localize);
		default:
			throw wexception(
			   "Wrong argument type: excepted float/double, found %s", to_string(t).c_str());
		}

		const int64_t as_int = static_cast<int64_t>(arg < 0 ? -arg : arg);

		if (min_width_ == 0 || flags_ & kLeftAlign) {
			// The easy case: Just start writing.
			size_t written = 0;
			if (arg < 0) {
				*out = '-';
				++out;
				++written;
			} else if (flags_ & kNumberSign) {
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
		if (arg < 0 || flags_ & kNumberSign) {
			++required_width;
		}

		// Start writing
		if (required_width < min_width_) {
			required_width = min_width_ - required_width;
			for (; required_width; ++out, --required_width) {
				*out = (flags_ & kPadWith0 ? '0' : ' ');
			}
		}

		if (arg < 0) {
			*out = '-';
			++out;
		} else if (flags_ & kNumberSign) {
			*out = '+';
			++out;
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

struct WildcardNode : FormatNode {
	WildcardNode() : FormatNode(kNone, 0, 0) {
	}

	char* append(char* out, const ArgType t, Argument arg_u, const bool localize) const override {
		switch (t) {
		case AbstractNode::ArgType::kChar:
			return CharNode::node_.append(out, t, arg_u, localize);
		case AbstractNode::ArgType::kString:
			return StringNode::node_.append(out, t, arg_u, localize);
		case AbstractNode::ArgType::kBoolean:
			return BooleanNode::node_.append(out, t, arg_u, localize);
		case AbstractNode::ArgType::kFloat:
			return FloatNode::node_.append(out, t, arg_u, localize);
		case AbstractNode::ArgType::kSigned:
			return IntNode::node_.append(out, t, arg_u, localize);
		case AbstractNode::ArgType::kUnsigned:
			return UintNode::node_.append(out, t, arg_u, localize);
		case AbstractNode::ArgType::kPointer:
			return UintNode::node_.append(out, AbstractNode::ArgType::kUnsigned, arg_u, localize);
		case AbstractNode::ArgType::kNullptr:
			arg_u.string_val = "nullptr";
			return StringNode::node_.append(out, AbstractNode::ArgType::kString, arg_u, localize);
		default:
			throw wexception("No argument for wildcard found");
		}
	}
};

class Tree {
	std::vector<std::pair<std::unique_ptr<AbstractNode>, unsigned /* format index */>> nodes_;
	std::vector<AbstractNode*> format_nodes_by_index_;
	unsigned format_nodes_count_;

	static std::map<std::string, std::unique_ptr<Tree>> cache_;

	static constexpr int64_t kBufferSize = 1024 * 1024;  // arbitrary limit
	static char buffer_[kBufferSize];

public:
	static const Tree& get(const std::string& format_string) {
		const auto it = cache_.find(format_string);
		if (it != cache_.end()) {
			return *it->second;
		}

		Tree* t = new Tree(format_string.c_str());
		cache_.emplace(format_string, std::unique_ptr<Tree>(t));
		return *t;
	}

	template <typename... Args> std::string format(const bool localize, Args... args) const {
		char* out(buffer_);
		bool hit_last_arg = false;

		for (const auto& pair : nodes_) {
			if (pair.second == 0) {
				out = pair.first->append(
				   out, AbstractNode::ArgType::kNone, AbstractNode::Argument(), localize);
			} else {
				hit_last_arg |= format_impl(&out, pair.second, pair.second - 1, localize, args...);
			}
		}

		if (!hit_last_arg) {
			throw wexception("Too many arguments provided (expected only %u)", format_nodes_count_);
		}

		if (static_cast<int64_t>(out - buffer_) >= kBufferSize) {
			throw wexception("Buffer overflow: found %" PRId64 " characters, limit is %" PRId64,
			                 static_cast<int64_t>(out - buffer_), kBufferSize);
		}
		*out = '\0';
		return buffer_;
	}

	std::string format(const bool localize, const ArgsVector& args) const {
		if (args.size() != format_nodes_count_) {
			throw wexception("Wrong number of arguments: expected %u, found %u", format_nodes_count_,
			                 static_cast<unsigned>(args.size()));
		}

		char* out(buffer_);
		for (const auto& pair : nodes_) {
			if (pair.second == 0) {
				out = pair.first->append(
				   out, AbstractNode::ArgType::kNone, AbstractNode::Argument(), localize);
			} else {
				out = format_nodes_by_index_[pair.second - 1]->append(
				   out, args[pair.second - 1].first, args[pair.second - 1].second, localize);
			}
		}

		if (static_cast<int64_t>(out - buffer_) >= kBufferSize) {
			throw wexception("Buffer overflow: found %" PRId64 " characters, limit is %" PRId64,
			                 static_cast<int64_t>(out - buffer_), kBufferSize);
		}
		*out = '\0';
		return buffer_;
	}

private:
	static AbstractNode::Argument arg_;
	inline void format_do_impl_run(char** out,
	                               unsigned orig_index,
	                               bool localize,
	                               AbstractNode::ArgType t) const {
		*out = format_nodes_by_index_[orig_index - 1]->append(*out, t, arg_, localize);
	}

	inline void format_do_impl(char** out, unsigned orig_index, bool localize, bool t) const {
		arg_.boolean_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kBoolean);
	}
	inline void format_do_impl(char** out, unsigned orig_index, bool localize, char t) const {
		arg_.char_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kChar);
	}

	inline void format_do_impl(char** out, unsigned orig_index, bool localize, const char* t) const {
		arg_.string_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kString);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, const std::string& t) const {
		arg_.string_val = t.c_str();
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kString);
	}

	inline void format_do_impl(char** out, unsigned orig_index, bool localize, float t) const {
		arg_.float_val = static_cast<double>(t);
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kFloat);
	}
	inline void format_do_impl(char** out, unsigned orig_index, bool localize, double t) const {
		arg_.float_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kFloat);
	}

	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, signed long long int t) const {
		arg_.signed_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kSigned);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, signed long int t) const {
		arg_.signed_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kSigned);
	}
	inline void format_do_impl(char** out, unsigned orig_index, bool localize, signed int t) const {
		arg_.signed_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kSigned);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, signed short int t) const {
		arg_.signed_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kSigned);
	}
	inline void format_do_impl(char** out, unsigned orig_index, bool localize, int8_t t) const {
		arg_.signed_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kSigned);
	}

	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, unsigned long long int t) const {
		arg_.unsigned_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kUnsigned);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, unsigned long int t) const {
		arg_.unsigned_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kUnsigned);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, unsigned int t) const {
		arg_.unsigned_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kUnsigned);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, unsigned short t) const {
		arg_.unsigned_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kUnsigned);
	}
	inline void format_do_impl(char** out, unsigned orig_index, bool localize, uint8_t t) const {
		arg_.unsigned_val = t;
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kUnsigned);
	}

	inline void format_do_impl(char** out, unsigned orig_index, bool localize, const void* t) const {
		arg_.unsigned_val = reinterpret_cast<uintptr_t>(t);
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kPointer);
	}
	inline void
	format_do_impl(char** out, unsigned orig_index, bool localize, const std::nullptr_t) const {
		format_do_impl_run(out, orig_index, localize, AbstractNode::ArgType::kNullptr);
	}

	template <typename T>
	inline bool format_impl(
	   char** out, unsigned orig_index, unsigned arg_index, const bool localize, T t) const {
		if (arg_index != 0) {
			throw wexception("Too few arguments provided: %u required but only %u passed",
			                 format_nodes_count_, orig_index - arg_index);
		}
		format_do_impl(out, orig_index, localize, t);
		return true;
	}
	template <typename T, typename... Args>
	inline bool format_impl(char** out,
	                        unsigned orig_index,
	                        unsigned arg_index,
	                        const bool localize,
	                        T t,
	                        Args... args) const {
		if (arg_index == 0) {
			format_do_impl(out, orig_index, localize, t);
			return false;
		}
		// By the way, this is not a recursive call – the compiler generates a separate
		// function for every individual template specification, and they're all inlined.
		return format_impl(out, orig_index, arg_index - 1, localize, args...);
	}

	explicit Tree(const char* format_string) : format_nodes_count_(0) {
		for (; *format_string != '\0';) {
			// read the next character and determine the kind of node we're looking at
			if (*format_string != '%') {
				// plain text, read until the next % sign
				const char* start = format_string;
				size_t len = 0;
				for (; *format_string != '\0' && *format_string != '%'; ++format_string, ++len) {
				}
				nodes_.push_back(
				   {std::unique_ptr<AbstractNode>(new LiteralNode(std::string(start, len))), 0});
				continue;
			}
			// A format node. Let's see what it says.
			++format_string;
			if (*format_string == '\0') {
				throw wexception("expected format sequence after '%%', found \\0");
			}
			if (*format_string == '%') {
				nodes_.push_back({std::unique_ptr<AbstractNode>(new PercentNode), 0});
				++format_string;
				continue;
			}

			static unsigned format_index;
			static unsigned precision;
			static size_t min_width;
			static uint8_t flags;
			static bool min_w_found;
			precision = kInfinitePrecision;
			min_width = 0;
			flags = kNone;
			min_w_found = false;

			if (*format_string >= '1' && *format_string <= '9') {
				// Read a number. This is either a format index or the min width.
				format_index = 0;
				do {
					format_index *= 10;
					format_index += (*format_string - '0');
					++format_string;
				} while (*format_string >= '0' && *format_string <= '9');
				if (*format_string == '\0') {
					throw wexception("expected format sequence after '%%%u', found \\0", format_index);
				}

				if (*format_string == '%') {
					// A node of the form '%N%' is complete.
					AbstractNode* node = new WildcardNode;
					nodes_.push_back({std::unique_ptr<AbstractNode>(node), format_index});
					if (format_index > format_nodes_count_) {
						format_nodes_count_ = format_index;
						format_nodes_by_index_.resize(format_nodes_count_, nullptr);
					}
					if (format_nodes_by_index_[format_index - 1] != nullptr) {
						throw wexception("duplicate use of index %u", format_index);
					}
					format_nodes_by_index_[format_index - 1] = node;
					++format_string;
					continue;
				} else if (*format_string == '$') {
					// $ character was recognized. Continue below.
					++format_string;
				} else {
					// This was not a format index but the min_width.
					// Correct the assumptions and jump forward.
					min_width = format_index;
					format_index = format_nodes_count_ + 1;
					min_w_found = true;
				}
			} else {
				format_index = format_nodes_count_ + 1;
			}

			if (!min_w_found) {
				// Index was discovered. Now we get the flags.
				for (;; ++format_string) {
					if (*format_string == '+') {
						flags |= Flags::kNumberSign;
					} else if (*format_string == '-') {
						flags |= Flags::kLeftAlign;
					} else if (*format_string == '0') {
						flags |= Flags::kPadWith0;
					} else {
						break;
					}
				}

				// Now check if a width has been specified.
				while (*format_string >= '0' && *format_string <= '9') {
					min_width *= 10;
					min_width += (*format_string - '0');
					++format_string;
				}
			}

			// Now we check if a precision has been specified.
			if (*format_string == '\0') {
				throw wexception("unterminated format sequence at end of string");
			}
			if (*format_string == '.') {
				++format_string;
				if (*format_string < '0' || *format_string > '9') {
					throw wexception(
					   "expected numerical argument after '.', found '%c'", *format_string);
				}
				precision = 0;
				do {
					precision *= 10;
					precision += (*format_string - '0');
					++format_string;
				} while (*format_string >= '0' && *format_string <= '9');
			}

			// Finally, we reach the type specifiers.
			AbstractNode* node = nullptr;
			switch (*format_string) {
			case 's':
				node = new StringNode(flags, min_width, precision);
				break;
			case 'b':
				node = new BooleanNode(flags, min_width, precision);
				break;
			case 'f':
				node =
				   new FloatNode(flags, min_width,
				                 precision == kInfinitePrecision ? kDefaultFloatPrecision : precision);
				break;

			case 'c':
				if (flags != kNone || min_width != 0 || precision != kInfinitePrecision) {
					throw wexception("%%c can not have additional specifiers");
				}
				node = new CharNode;
				break;

			case 'l':
			case 'i':
			case 'd':
			case 'u': {
				if (precision != kInfinitePrecision) {
					throw wexception("integers can not have precision");
				}
				for (; *format_string == 'l'; ++format_string) {
				}
				if (*format_string == 'd' || *format_string == 'i') {
					node = new IntNode(flags, min_width, false, false);
				} else if (*format_string == 'u') {
					node = new UintNode(flags, min_width, false, false);
				} else {
					throw wexception("invalid format type character '%c' after '%%l'", *format_string);
				}
				break;
			}

			case 'x':
			case 'X': {
				if (precision != kInfinitePrecision) {
					throw wexception("integers can not have precision");
				}
				node = new IntNode(flags, min_width, true, *format_string == 'X');
				break;
			}

			case 'p':
			case 'P': {
				if (precision != kInfinitePrecision) {
					throw wexception("pointers can not have precision");
				}
				node = new UintNode(flags, min_width, true, *format_string == 'P');
				break;
			}

			default:
				throw wexception("unrecognized format type character '%c'", *format_string);
			}
			++format_string;

			nodes_.push_back({std::unique_ptr<AbstractNode>(node), format_index});
			if (format_index > format_nodes_count_) {
				format_nodes_count_ = format_index;
				format_nodes_by_index_.resize(format_nodes_count_, nullptr);
			}
			if (format_nodes_by_index_[format_index - 1] != nullptr) {
				throw wexception("duplicate use of index %u", format_index);
			}
			format_nodes_by_index_[format_index - 1] = node;

		}  // end of the big for loop

		assert(format_nodes_by_index_.size() == format_nodes_count_);
		for (unsigned i = format_nodes_count_; i; --i) {
			if (format_nodes_by_index_[format_nodes_count_ - i] == nullptr) {
				throw wexception("index %u is unused", format_nodes_count_ - i + 1);
			}
		}
	}
};

template <typename... Args>
std::string format(const bool localize, const std::string& format_string, Args... args) {
	try {
		// The textdomain uses a mutex internally, so it guarantees thread-safety.
		i18n::Textdomain textdomain("widelands");
		return format_impl::Tree::get(format_string).format(localize, args...);
	} catch (const std::exception& e) {
		log_err("bformat error: A string contains invalid printf placeholders");
		log_err("Error: %s", e.what());
		log_err("String: %s", format_string.c_str());
		throw;
	}
}

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_IMPL_H
