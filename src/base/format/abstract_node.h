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

#ifndef WL_BASE_FORMAT_ABSTRACT_NODE_H
#define WL_BASE_FORMAT_ABSTRACT_NODE_H

#include <cstring>

namespace format_impl {

enum Flags : uint8_t {
	kNone = 0,
	kLeftAlign = 1,
	kNumberSign = 2,
	kPadWith0 = 4,
};

// These are multi-character UTF-8 strings
static const char kLocalizedMinusSign[] = "−";  // minus sign
static const char kDigitWidthSpace[] = " ";     // digit-sized whitespace
static const size_t kLocalizedMinusSignLength = strlen(kLocalizedMinusSign);
static const size_t kDigitWidthSpaceLength = strlen(kDigitWidthSpace);

constexpr int64_t kMaxInt = std::numeric_limits<int64_t>::max();
constexpr unsigned kInfinitePrecision = std::numeric_limits<unsigned>::max();
constexpr unsigned kDefaultFloatPrecision = 6;

// Don't use localized + sign for 0.0 if at least the next two digits would also be 0.
constexpr double kDynamicPrecisionExtra = 0.005;

struct AbstractNode {
	virtual ~AbstractNode() {
	}

	union Argument {
		signed char char_val;
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

struct FormatNode : AbstractNode {
protected:
	FormatNode(const uint8_t f, const size_t w, const int32_t p)
	   : flags_(f), min_width_(w), precision_(p) {
	}

	const uint8_t flags_;       ///< Bitset of flags
	const size_t min_width_;    ///< Minimum number of characters to write; may be 0
	const unsigned precision_;  ///< See above; may be kInfinitePrecision
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_ABSTRACT_NODE_H
