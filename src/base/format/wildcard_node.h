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

#ifndef WL_BASE_FORMAT_WILDCARD_NODE_H
#define WL_BASE_FORMAT_WILDCARD_NODE_H

#include "base/format/boolean_node.h"
#include "base/format/common_nodes.h"
#include "base/format/float_node.h"
#include "base/format/number_node.h"

namespace format_impl {

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
			return pointer_node_.append(out, AbstractNode::ArgType::kUnsigned, arg_u, localize);
		case AbstractNode::ArgType::kNullptr:
			return append_nullptr_node(out, localize);
		default:
			throw wexception("No argument for wildcard found");
		}
	}
};

}  // namespace format_impl

#endif  // end of include guard: WL_BASE_FORMAT_WILDCARD_NODE_H
