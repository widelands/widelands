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

#include "editor/tools/editor_history.h"

#include <string>

#include "editor/editorinteractive.h"
#include "editor/tools/editor_action_args.h"
#include "editor/tools/editor_tool_action.h"

// === Editor_Action_Args === //

Editor_Action_Args::Editor_Action_Args(Editor_Interactive & base):
	sel_radius(base.get_sel_radius()),
	change_by(0),
	cur_res(0),
	set_to(0),
	m_interval(0, 0),
	refcount(0)
{}

Editor_Action_Args::~Editor_Action_Args()
{
	while (not draw_actions.empty()) {
		delete draw_actions.back();
		draw_actions.pop_back();
	}
	nbob_type.clear();
	obob_type.clear();
	nimmov_types.clear();
	oimmov_types.clear();
	orgRes.clear();
	orgResT.clear();
	origHights.clear();
	origTerrainType.clear();
	terrainType.clear();
}

// === Editor_History === //

uint32_t Editor_History::undo_action()
{
	if (undo_stack.empty())
		return 0;

	Editor_Tool_Action uac = undo_stack.front();
	undo_stack.pop_front();
	redo_stack.push_front(uac);

	m_undo_button.set_enabled(!undo_stack.empty());
	m_redo_button.set_enabled(true);

	return
		uac.tool.handle_undo
		(static_cast<Editor_Tool::Tool_Index>(uac.i),
		 uac.map, uac.center, uac.parent, *uac.args);
}

uint32_t Editor_History::redo_action()
{
	if (redo_stack.empty())
		return 0;

	Editor_Tool_Action rac = redo_stack.front();
	redo_stack.pop_front();
	undo_stack.push_front(rac);

	m_undo_button.set_enabled(true);
	m_redo_button.set_enabled(!redo_stack.empty());

	return
		rac.tool.handle_click
		(static_cast<Editor_Tool::Tool_Index>(rac.i),
		 rac.map, rac.center, rac.parent, *rac.args);
}

uint32_t Editor_History::do_action
	(Editor_Tool & tool,
	Editor_Tool::Tool_Index ind,
	Widelands::Map & map,
	const Widelands::Node_and_Triangle< Widelands::Coords > center,
	Editor_Interactive & parent,
	bool draw)
{
	Editor_Tool_Action ac
		(tool, static_cast<uint32_t>(ind),
		 map, center, parent, tool.format_args(ind, parent));
	if (draw && tool.is_unduable()) {
		if
			(undo_stack.empty() or
			 undo_stack.front().tool.get_sel_impl() != std::string(m_draw_tool.get_sel_impl()))
		{
			Editor_Tool_Action da
				(m_draw_tool, Editor_Tool::First,
				 map, center, parent,
				 m_draw_tool.format_args(Editor_Tool::First, parent));

			if (not undo_stack.empty()) {
				m_draw_tool.add_action(undo_stack.front(), *da.args);
				undo_stack.pop_front();
			}

			redo_stack.clear();
			undo_stack.push_front(da);
			m_undo_button.set_enabled(true);
			m_redo_button.set_enabled(false);
		}
		dynamic_cast<Editor_Draw_Tool *>
			(&(undo_stack.front().tool))->add_action(ac, *undo_stack.front().args);
	} else if (tool.is_unduable()) {
		redo_stack.clear();
		undo_stack.push_front(ac);
		m_undo_button.set_enabled(true);
		m_redo_button.set_enabled(false);
	}
	return tool.handle_click(ind, map, center, parent, *ac.args);
}


void Editor_History::reset()
{
	undo_stack.clear();
	redo_stack.clear();
	m_undo_button.set_enabled(false);
	m_redo_button.set_enabled(false);
}

