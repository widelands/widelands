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

#ifndef WL_BASE_FORMAT_TREE_H
#define WL_BASE_FORMAT_TREE_H

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/format/abstract_node.h"
#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"

namespace format_impl {

/**
   General syntax of format specifiers:
      %N%
   OR
      % [ N$ ] [ flags ] [ width ] [ . precision ] fmt

   N: Argument index. The N-th format argument will be used for the placeholder with ID N.
      Argument indices are 1-based and there may be no gaps or duplicates.
      Unnumbered placeholders are enumerated from left to right.
      Numbered and unnumbered placeholders may not be mixed in a format string.

   flags: Any combination of
      -    Left align
      +    Show number sign for non-negative numbers
      0    pad with zeros instead of whitespace

      Note that '-' and '0' can not be combined.

   width: minimal number of characters

   precision:
      For floating-point, max number of digits after the period.
      For strings, max number of characters before padding.
      Not allowed for other types.

   fmt:
      %                   percent sign                No arguments allowed
      c                   character                   No arguments allowed
      s                   string                      Can not have '+' or '0'
      b                   boolean                     Similar to string
      i,d,li,ld,lli,lld   int
      u,lu,llu            unsigned int
      x                   signed hexadecimal int (lowercase)
      X                   signed hexadecimal int (uppercase)
      p                   unsigned hexadecimal int (lowercase)
      P                   unsigned hexadecimal int (uppercase)
      f                   float/double

   Some notes:
      - The number of optional 'l' characters in %i, %d, and %u placeholders is limited to 2; it
        has no semantic meaning though as their arguments are always cast to a 64-bit value.
      - The %c specifier escapes control characters (0x0 to 0x1F and 0x7F) using caret notation.
      - The %p and %P specifiers add the prefix "0x" to the output; %x and %X do not add a prefix.
      - It is not possible to print a floating-point number in scientific exponent notation.

   The argument that controls localized output has the following effects:
      - Translatable decimal separator for floating point values.
      - Translatable "true"/"false" constants for booleans.
      - Unicode minus sign for negative numbers.
      - Unicode digit-sized whitespace for integers with value zero if the '+' flag is set.

   Unsigned integral and pointer values may not be greater than (2^64)-1, so that they can be
   stored in a uint64_t. All other numeric values may not be larger than (2^63)-1 or less than
   1-(2^63), so that they can be cast to int64_t. This also applies to floats and doubles.
*/

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

	// Parser functions
	explicit Tree(const char* format_string);
	void parse_next_node(const char*&);
	void parse_literal_node(const char*&);
	void parse_format_index(const char*&);
	bool parse_index_or_minwidth(const char*&);
	void parse_flags_and_minwidth(const char*&);
	void parse_precision(const char*&);
	std::unique_ptr<AbstractNode> parse_type_spec(const char*&);
	void recognized_wildcard_node();
	void recognized_node(std::unique_ptr<AbstractNode>);
};

template <typename... Args>
std::string format(const bool localize, const std::string& format_string, Args... args) {
	try {
		if (localize) {
			// The textdomain uses a mutex internally, so it guarantees thread-safety.
			i18n::Textdomain textdomain("widelands");
			return format_impl::Tree::get(format_string).format(true, args...);
		}
		MutexLock m(MutexLock::ID::kI18N);
		return format_impl::Tree::get(format_string).format(false, args...);
	} catch (const std::exception& e) {
		log_err("format error: A string contains invalid printf placeholders");
		log_err("Error: %s", e.what());
		log_err("String: %s", format_string.c_str());
		throw;
	}
}

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_TREE_H
