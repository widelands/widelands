/*
 * Copyright (C) 2012 by the Widelands Development Team
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

#ifndef EDITOR_TOOL_ACTION_H
#define EDITOR_TOOL_ACTION_H

#include "editor/tools/editor_action_args.h"
#include "logic/widelands_geometry.h"

class Editor_Tool;
namespace Widelands {class map;}
struct Editor_Interactive;


/// Class to save an action done by an editor tool
// implementations in editor_history.cc
struct Editor_Tool_Action {
	Editor_Tool & tool;

	uint32_t i;
	Widelands::Map & map;
	Widelands::Node_and_Triangle<> center;
	Editor_Interactive & parent;

	Editor_Action_Args * args;

	Editor_Tool_Action
		(Editor_Tool & t, uint32_t ind,
		Widelands::Map & m, Widelands::Node_and_Triangle<> c,
		Editor_Interactive & p, Editor_Action_Args nargs)
			: tool(t), i(ind), map(m), center(c), parent(p)
	{
		args = new Editor_Action_Args(parent);
		*args = nargs;
		args->refcount++;
	}

	~Editor_Tool_Action() {
		if (args->refcount <= 1)
			delete args;
		else args->refcount--;
	}

	Editor_Tool_Action(const Editor_Tool_Action & b):
		tool(b.tool), i(b.i), map(b.map),
		center(b.center), parent(b.parent), args(b.args)
	{args->refcount++;}
};

#endif
