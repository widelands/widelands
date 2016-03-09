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

// === EditorActionArgs === //

EditorActionArgs::EditorActionArgs(EditorInteractive & base):
	sel_radius(base.get_sel_radius()),
	change_by(0),
	cur_res(0),
	set_to_resource(0),
	m_interval(0, 0),
	refcount(0)
{}

EditorActionArgs::~EditorActionArgs()
{
	while (!draw_actions.empty()) {
		delete draw_actions.back();
		draw_actions.pop_back();
	}
	nbob_type.clear();
	obob_type.clear();
	nimmov_types.clear();
	oimmov_types.clear();
	org_res.clear();
	org_res_t.clear();
	origHights.clear();
	origTerrainType.clear();
	terrainType.clear();
}

// === EditorHistory === //

uint32_t EditorHistory::undo_action(const Widelands::World& world) {
	if (undo_stack.empty())
		return 0;

	EditorToolAction uac = undo_stack.front();
	undo_stack.pop_front();
	redo_stack.push_front(uac);

	m_undo_button.set_enabled(!undo_stack.empty());
	m_redo_button.set_enabled(true);

	return uac.tool.handle_undo(static_cast<EditorTool::ToolIndex>(uac.i),
	                            world,
	                            uac.center,
	                            uac.parent,
	                            uac.args,
								&(uac.map));
}

uint32_t EditorHistory::redo_action(const Widelands::World& world) {
	if (redo_stack.empty())
		return 0;

	EditorToolAction rac = redo_stack.front();
	redo_stack.pop_front();
	undo_stack.push_front(rac);

	m_undo_button.set_enabled(true);
	m_redo_button.set_enabled(!redo_stack.empty());

	return rac.tool.handle_click(static_cast<EditorTool::ToolIndex>(rac.i),
	                             world,
	                             rac.center,
	                             rac.parent,
	                             rac.args,
								 &(rac.map));
}

uint32_t EditorHistory::do_action(EditorTool& tool,
											  EditorTool::ToolIndex ind,
                                   Widelands::Map& map,
                                   const Widelands::World& world,
                                   const Widelands::NodeAndTriangle<Widelands::Coords> center,
											  EditorInteractive& parent,
                                   bool draw) {
	EditorToolAction ac
		(tool, static_cast<uint32_t>(ind),
		 map, center, parent, tool.format_args(ind, parent));
	if (draw && tool.is_unduable()) {
		if
			(undo_stack.empty() ||
			 undo_stack.front().tool.get_sel_impl() != std::string(m_draw_tool.get_sel_impl()))
		{
			EditorToolAction da
				(m_draw_tool, EditorTool::First,
				 map, center, parent,
				 m_draw_tool.format_args(EditorTool::First, parent));

			if (!undo_stack.empty()) {
				m_draw_tool.add_action(undo_stack.front(), *da.args);
				undo_stack.pop_front();
			}

			redo_stack.clear();
			undo_stack.push_front(da);
			m_undo_button.set_enabled(true);
			m_redo_button.set_enabled(false);
		}
		dynamic_cast<EditorDrawTool *>
			(&(undo_stack.front().tool))->add_action(ac, *undo_stack.front().args);
	} else if (tool.is_unduable()) {
		redo_stack.clear();
		undo_stack.push_front(ac);
		m_undo_button.set_enabled(true);
		m_redo_button.set_enabled(false);
	}
	return tool.handle_click(ind, world, center, parent, ac.args, &map);
}
