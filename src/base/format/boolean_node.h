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

#ifndef WL_BASE_FORMAT_BOOLEAN_NODE_H
#define WL_BASE_FORMAT_BOOLEAN_NODE_H

#include "base/format/string_node.h"

namespace format_impl {

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
			throw wexception("Wrong argument type: expected bool, found %s", to_string(t).c_str());
		}
		arg.string_val = localize ? val ? _("true") : _("false") : val ? "true" : "false";
		return StringNode::append(out, ArgType::kString, arg, localize);
	}

	static const BooleanNode node_;
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_BOOLEAN_NODE_H
