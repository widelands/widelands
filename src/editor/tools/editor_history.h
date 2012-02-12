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

#ifndef EDITOR_HISTORY_H
#define EDITOR_HISTORY_H

#include <deque>

#include "editor_tool.h"
#include <complex>

//struct Editor_Action_Args;
struct Editor_Interactive;
namespace UI { struct Button; }

/**
 * The all actions done with an editor tool are saved on a stack to
 * provide undo / redo functionality.
 * Do all tool action you want to make "undoable" using this class.
 */
struct Editor_History {

	Editor_History(UI::Button & undo, UI::Button & redo): m_undo_button(undo), m_redo_button(redo) {};

	uint32_t do_action(Editor_Tool & tool, Editor_Tool::Tool_Index ind, Widelands::Map & map,
	                   Widelands::Node_and_Triangle<> const center, Editor_Interactive & parent);
	uint32_t undo_action();
	uint32_t redo_action();

	/// Must be called after every change of map, world, or ... to avoid undo errors
	void reset();

	/// Put an action on the undo stack. It is possible to push an action without doing it.
	void push_action(Editor_Tool & tool, Editor_Tool::Tool_Index ind,  Widelands::Map & map,
	                 Widelands::Node_and_Triangle<> const center, Editor_Interactive & parent);

private:

	UI::Button & m_undo_button;
	UI::Button & m_redo_button;

	/// Class to save an action done by an editor tool
	struct Editor_Tool_Action {
		Editor_Tool & tool;

		Editor_Tool::Tool_Index i;
		Widelands::Map & map;
		Widelands::Node_and_Triangle<> center;
		Editor_Interactive & parent;

		Editor_Action_Args * args;

		Editor_Tool_Action(Editor_Tool & t, Editor_Tool::Tool_Index ind,
		                   Widelands::Map & m, Widelands::Node_and_Triangle<> c,
		                   Editor_Interactive & p, Editor_Action_Args nargs) :
			tool(t), i(ind), map(m), center(c), parent(p)
		{ args = new Editor_Action_Args(parent); *args = nargs; args->refcount++; }

		~Editor_Tool_Action() {
			if (args->refcount <= 1) delete args;
			else args->refcount--;
		}

		Editor_Tool_Action(const Editor_Tool_Action & b) :
			tool(b.tool), i(b.i), map(b.map), center(b.center), parent(b.parent), args(b.args)
		{ args->refcount++; }

	};

	std::deque<Editor_Tool_Action> undo_stack;
	std::deque<Editor_Tool_Action> redo_stack;

};



#endif

