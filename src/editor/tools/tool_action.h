/*
 * Copyright (C) 2012-2022 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_TOOL_ACTION_H
#define WL_EDITOR_TOOLS_TOOL_ACTION_H

#include "editor/tools/action_args.h"
#include "logic/widelands_geometry.h"

class EditorTool;

/// Class to save an action done by an editor tool
// implementations in editor_history.cc
struct EditorToolAction {
	EditorTool& tool;

	uint32_t i;
	Widelands::Map& map;
	Widelands::NodeAndTriangle<> center;
	EditorInteractive& parent;

	EditorActionArgs* args;

	EditorToolAction(EditorTool& t,
	                 uint32_t ind,
	                 Widelands::Map& m,
	                 Widelands::NodeAndTriangle<> c,
	                 EditorInteractive& p,
	                 const EditorActionArgs& nargs)
	   : tool(t), i(ind), map(m), center(c), parent(p) {
		args = new EditorActionArgs(parent);
		*args = nargs;
		args->refcount++;
	}

	~EditorToolAction() {
		if (args->refcount <= 1)
			delete args;
		else
			args->refcount--;
	}

	EditorToolAction(const EditorToolAction& b)
	   : tool(b.tool), i(b.i), map(b.map), center(b.center), parent(b.parent), args(b.args) {
		args->refcount++;
	}

	EditorToolAction& operator=(const EditorToolAction& b) = delete;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_TOOL_ACTION_H
