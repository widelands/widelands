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

#ifndef WL_BASE_FORMAT_COMMON_NODES_H
#define WL_BASE_FORMAT_COMMON_NODES_H

#include "base/format/abstract_node.h"

namespace format_impl {

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

struct CharNode : FormatNode {
	CharNode() : FormatNode(kNone, 0, 0) {
	}

	inline char* append(char* out, const ArgType t, const Argument arg, bool) const override {
		if (t != ArgType::kChar) {
			throw wexception("Wrong argument type: expected char, found %s", to_string(t).c_str());
		}

		// control characters
		if (arg.char_val >= 0 && arg.char_val < 0x20) {
			*out = '^';
			*(out + 1) = '@' + arg.char_val;
			return out + 2;
		} else if (arg.char_val == 0x7f) {
			*out = '^';
			*(out + 1) = '?';
			return out + 2;
		}

		*out = arg.char_val;
		return out + 1;
	}

	static const CharNode node_;
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_COMMON_NODES_H
