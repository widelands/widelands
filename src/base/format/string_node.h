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

#ifndef WL_BASE_FORMAT_STRING_NODE_H
#define WL_BASE_FORMAT_STRING_NODE_H

#include <cstring>
#include <memory>

#include "base/format/abstract_node.h"

namespace format_impl {

struct StringNode : FormatNode {
	StringNode(const uint8_t f, const size_t w, const int32_t p) : FormatNode(f, w, p) {
		if ((flags_ & kNumberSign) != 0) {
			throw wexception("%%s can not have '+' specifier");
		}
		if ((flags_ & kPadWith0) != 0) {
			throw wexception("%%s can not have '0' specifier");
		}
	}

	char* append(char* out, const ArgType t, const Argument arg_u, bool) const override {
		if (t != ArgType::kString) {
			throw wexception("Wrong argument type: expected string, found %s", to_string(t).c_str());
		}
		const char* arg = arg_u.string_val;
		std::unique_ptr<char[]> temp_buffer;

		// Replace control characters.
		size_t nr_characters = 0;
		size_t nr_ctrl_chars = 0;
		for (const char* c = arg; *c != 0; ++c, ++nr_characters) {
			const signed char ch = *c;
			if (ch == 0x7f || (ch >= 0 && ch < 0x20)) {
				++nr_ctrl_chars;
			}
		}
		if (nr_ctrl_chars > 0) {
			temp_buffer.reset(new char[nr_characters + nr_ctrl_chars + 1]);
			char* write = temp_buffer.get();
			for (const char* c = arg; *c != 0; ++c, ++write) {
				const signed char ch = *c;
				if (ch == 0x7f) {
					*write = '^';
					++write;
					*write = '?';
				} else if (ch >= 0 && ch < 0x20) {
					*write = '^';
					++write;
					*write = '@' + ch;
				} else {
					*write = ch;
				}
			}
			*write = 0;
			nr_characters += nr_ctrl_chars;
			arg = temp_buffer.get();
		}

		if (min_width_ == 0 || (flags_ & kLeftAlign) != 0) {
			// The easy case: Just start writing.
			size_t written = 0;
			for (const char* a = arg; *a != 0 && written < precision_; ++a, ++out, ++written) {
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
		const size_t arg_len = std::min<size_t>(precision_, nr_characters);
		if (arg_len < min_width_) {
			for (size_t i = min_width_ - arg_len; i != 0; ++out, --i) {
				*out = ' ';
			}
		}
		for (size_t l = arg_len; l != 0; --l, ++out, ++arg) {
			*out = *arg;
		}
		return out;
	}

	static const StringNode node_;
};

inline char* append_nullptr_node(char* out, const bool localize) {
	AbstractNode::Argument a;
	a.string_val = "nullptr";
	return StringNode::node_.append(out, AbstractNode::ArgType::kString, a, localize);
}

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_STRING_NODE_H
