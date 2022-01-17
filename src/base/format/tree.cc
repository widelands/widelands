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

#include "base/format/tree.h"

#include <memory>

#include "base/format/wildcard_node.h"

namespace format_impl {

// Definitions of globals

std::map<std::string, std::unique_ptr<Tree>> Tree::cache_;
AbstractNode::Argument Tree::arg_;
char Tree::buffer_[Tree::kBufferSize];

const CharNode CharNode::node_;
const StringNode StringNode::node_(kNone, 0, kInfinitePrecision);
const BooleanNode BooleanNode::node_(kNone, 0, kInfinitePrecision);
const FloatNode FloatNode::node_(kNone, 0, kDefaultFloatPrecision);
const UintNode pointer_node_(kNone, 0, true, true, false);

std::string to_string(const AbstractNode::ArgType t) {
	switch (t) {
	case AbstractNode::ArgType::kNone:
		return "none";
	case AbstractNode::ArgType::kChar:
		return "char";
	case AbstractNode::ArgType::kString:
		return "string";
	case AbstractNode::ArgType::kBoolean:
		return "bool";
	case AbstractNode::ArgType::kFloat:
		return "float";
	case AbstractNode::ArgType::kSigned:
		return "int";
	case AbstractNode::ArgType::kUnsigned:
		return "unsigned";
	case AbstractNode::ArgType::kPointer:
		return "pointer";
	case AbstractNode::ArgType::kNullptr:
		return "nullptr";
	}
	NEVER_HERE();
}

// Parser implementation

static unsigned format_index;
static unsigned precision;
static size_t min_width;
static uint8_t flags;
static bool min_w_found;
static bool has_positional_node;
static bool has_unpositional_node;

static inline void add_flag_if_not_present(const Flags f, const char d) {
	if ((flags & f) != 0) {
		throw wexception("Repeated flag '%c'", d);
	}
	flags |= f;
}

inline void Tree::parse_literal_node(const char*& format_string) {
	const char* start = format_string;
	size_t len = 0;
	for (; *format_string != '\0' && *format_string != '%'; ++format_string, ++len) {
	}
	nodes_.emplace_back(std::unique_ptr<AbstractNode>(new LiteralNode(std::string(start, len))), 0);
}

inline void Tree::recognized_node(std::unique_ptr<AbstractNode> node) {
	if (format_index > format_nodes_count_) {
		format_nodes_count_ = format_index;
		format_nodes_by_index_.resize(format_nodes_count_, nullptr);
	}
	if (format_nodes_by_index_[format_index - 1] != nullptr) {
		throw wexception("duplicate use of index %u", format_index);
	}
	format_nodes_by_index_[format_index - 1] = node.get();
	nodes_.emplace_back(std::move(node), format_index);
}

inline void Tree::recognized_wildcard_node() {
	if (has_unpositional_node) {
		throw wexception("Cannot mix positional and unpositional placeholders");
	}
	has_positional_node = true;
	recognized_node(std::unique_ptr<AbstractNode>(new WildcardNode));
}

inline void Tree::parse_format_index(const char*& format_string) {
	format_index = 0;
	do {
		format_index *= 10;
		format_index += (*format_string - '0');
		++format_string;
	} while (*format_string >= '0' && *format_string <= '9');
}

inline bool Tree::parse_index_or_minwidth(const char*& format_string) {
	// Read a number. This is either a format index or the min width.
	parse_format_index(format_string);

	switch (*format_string) {
	case '\0':
		throw wexception("expected format sequence after '%%%u', found \\0", format_index);
	case '%':
		// A node of the form '%N%' is complete.
		recognized_wildcard_node();
		++format_string;
		return true;

	case '$':
		// $ character was recognized. Continue below.
		if (has_unpositional_node) {
			throw wexception("Cannot mix positional and unpositional placeholders");
		}
		has_positional_node = true;
		++format_string;
		break;

	default:
		// This was not a format index but the min_width.
		// Correct the assumptions and jump forward.
		if (has_positional_node) {
			throw wexception("Cannot mix positional and unpositional placeholders");
		}
		has_unpositional_node = true;
		min_width = format_index;
		format_index = format_nodes_count_ + 1;
		min_w_found = true;
		break;
	}
	return false;
}

inline void Tree::parse_flags_and_minwidth(const char*& format_string) {
	// Index was discovered. Now we get the flags.
	for (;; ++format_string) {
		if (*format_string == '+') {
			add_flag_if_not_present(Flags::kNumberSign, *format_string);
		} else if (*format_string == '-') {
			add_flag_if_not_present(Flags::kLeftAlign, *format_string);
		} else if (*format_string == '0') {
			add_flag_if_not_present(Flags::kPadWith0, *format_string);
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

inline void Tree::parse_precision(const char*& format_string) {
	++format_string;
	if (*format_string < '0' || *format_string > '9') {
		throw wexception("expected numerical argument after '.', found '%c'", *format_string);
	}
	precision = 0;
	do {
		precision *= 10;
		precision += (*format_string - '0');
		++format_string;
	} while (*format_string >= '0' && *format_string <= '9');
}

inline std::unique_ptr<AbstractNode> Tree::parse_type_spec(const char*& format_string) {
	switch (*format_string) {
	case 's':
		return std::unique_ptr<AbstractNode>(new StringNode(flags, min_width, precision));
	case 'b':
		return std::unique_ptr<AbstractNode>(new BooleanNode(flags, min_width, precision));
	case 'f':
		return std::unique_ptr<AbstractNode>(new FloatNode(
		   flags, min_width, precision == kInfinitePrecision ? kDefaultFloatPrecision : precision));

	case 'c':
		if (flags != kNone || min_width != 0 || precision != kInfinitePrecision) {
			throw wexception("%%c can not have additional specifiers");
		}
		return std::unique_ptr<AbstractNode>(new CharNode);

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
			return std::unique_ptr<AbstractNode>(new IntNode(flags, min_width, false, false, false));
		} else if (*format_string == 'u') {
			return std::unique_ptr<AbstractNode>(new UintNode(flags, min_width, false, false, false));
		} else {
			throw wexception("invalid format type character '%c' after '%%l'", *format_string);
		}
	}

	case 'x':
	case 'X': {
		if (precision != kInfinitePrecision) {
			throw wexception("integers can not have precision");
		}
		return std::unique_ptr<AbstractNode>(
		   new IntNode(flags, min_width, true, false, *format_string == 'X'));
	}

	case 'p':
	case 'P': {
		if (precision != kInfinitePrecision) {
			throw wexception("pointers can not have precision");
		}
		return std::unique_ptr<AbstractNode>(
		   new UintNode(flags, min_width, true, true, *format_string == 'P'));
	}

	default:
		throw wexception("unrecognized format type character '%c'", *format_string);
	}
}

void Tree::parse_next_node(const char*& format_string) {
	// Read the next character and determine the kind of node we're looking at.

	if (*format_string != '%') {
		// Plain text, read until the next % sign.
		parse_literal_node(format_string);
		return;
	}

	// A format node. Let's see what it says.
	++format_string;
	if (*format_string == '\0') {
		throw wexception("expected format sequence after '%%', found \\0");
	}
	if (*format_string == '%') {
		nodes_.emplace_back(std::unique_ptr<AbstractNode>(new PercentNode), 0);
		++format_string;
		return;
	}

	precision = kInfinitePrecision;
	min_width = 0;
	flags = kNone;
	min_w_found = false;

	if (*format_string >= '1' && *format_string <= '9') {
		if (parse_index_or_minwidth(format_string)) {
			return;
		}
	} else {
		if (has_positional_node) {
			throw wexception("Cannot mix positional and unpositional placeholders");
		}
		has_unpositional_node = true;
		format_index = format_nodes_count_ + 1;
	}

	if (!min_w_found) {
		parse_flags_and_minwidth(format_string);
	}

	// Now we check if a precision has been specified.
	if (*format_string == '\0') {
		throw wexception("unterminated format sequence at end of string");
	}
	if (*format_string == '.') {
		parse_precision(format_string);
	}

	// Finally, we reach the type specifiers.
	recognized_node(parse_type_spec(format_string));
	++format_string;
}

Tree::Tree(const char* format_string) : format_nodes_count_(0) {
	has_positional_node = false;
	has_unpositional_node = false;

	try {
		for (; *format_string != '\0';) {
			parse_next_node(format_string);
		}

		assert(format_nodes_by_index_.size() == format_nodes_count_);
		for (unsigned i = format_nodes_count_; i; --i) {
			if (format_nodes_by_index_[format_nodes_count_ - i] == nullptr) {
				throw wexception("index %u is unused", format_nodes_count_ - i + 1);
			}
		}
	} catch (...) {
		nodes_.clear();  // Prevent memory leaks
		throw;
	}
}

}  // namespace format_impl
