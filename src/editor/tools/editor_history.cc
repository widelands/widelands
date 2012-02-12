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

#include "editor_history.h"
#include "editor/editorinteractive.h"

Editor_Action_Args::Editor_Action_Args(Editor_Interactive & base):
	sel_radius(base.get_sel_radius()),
	m_interval(0, 0),
	refcount(0)
{}

uint32_t Editor_History::undo_action() {
	if (undo_stack.empty())
		return 0;

	Editor_Tool_Action uac = undo_stack.front();
	undo_stack.pop_front();
	redo_stack.push_front(uac);

	m_undo_button.set_enabled(!undo_stack.empty());
	m_redo_button.set_enabled(true);

	return uac.tool.handle_undo(uac.i, uac.map, uac.center, uac.parent, *uac.args);
}

uint32_t Editor_History::redo_action() {
	if (redo_stack.empty())
		return 0;

	Editor_Tool_Action rac = redo_stack.front();
	redo_stack.pop_front();
	undo_stack.push_front(rac);

	m_undo_button.set_enabled(true);
	m_redo_button.set_enabled(!redo_stack.empty());

	return rac.tool.handle_click(rac.i, rac.map, rac.center, rac.parent, *rac.args);
}

uint32_t Editor_History::do_action(Editor_Tool & tool,
                                   Editor_Tool::Tool_Index ind,
                                   Widelands::Map & map,
                                   const Widelands::Node_and_Triangle< Widelands::Coords > center,
                                   Editor_Interactive & parent) {
	Editor_Tool_Action ac(tool, ind, map, center, parent, tool.format_args(ind, parent));
	if (tool.is_unduable()) {
		redo_stack.clear();
		undo_stack.push_front(ac);
		m_undo_button.set_enabled(true);
		m_redo_button.set_enabled(false);
	}
	return tool.handle_click(ind, map, center, parent, *ac.args);
}


void Editor_History::reset() {
	undo_stack.clear();
	redo_stack.clear();
	m_undo_button.set_enabled(false);
	m_redo_button.set_enabled(false);
}

void Editor_History::push_action(
    Editor_Tool & tool, Editor_Tool::Tool_Index ind, Widelands::Map & map,
    const Widelands::Node_and_Triangle< Widelands::Coords > center, Editor_Interactive & parent) {
	undo_stack.push_front(Editor_Tool_Action(tool, ind, map, center, parent, tool.format_args(ind, parent)));
	redo_stack.clear();
	m_undo_button.set_enabled(true);
	m_redo_button.set_enabled(false);
}

