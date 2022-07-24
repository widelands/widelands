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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "editor/tools/history.h"

#include "editor/editorinteractive.h"
#include "editor/tools/action_args.h"
#include "editor/tools/tool_action.h"

// === EditorActionArgs === //

constexpr size_t kMaximumUndoActions = 500;
constexpr size_t kTooManyUndoActionsDeleteBatch = 50;

EditorActionArgs::EditorActionArgs(EditorInteractive& base)
   : sel_radius(base.get_sel_radius()),
     change_by(0),
     current_resource(0),
     set_to(0),
     new_map_size(0, 0),
     interval(0, 0),
     refcount(0) {
}

EditorActionArgs::~EditorActionArgs() {
	while (!draw_actions.empty()) {
		delete draw_actions.back();
		draw_actions.pop_back();
	}
	new_bob_type.clear();
	old_bob_type.clear();
	new_immovable_types.clear();
	old_immovable_types.clear();
	original_resource.clear();
	original_heights.clear();
	original_terrain_type.clear();
	terrain_type.clear();
}

// === EditorHistory === //

uint32_t EditorHistory::undo_action() {
	if (undo_stack_.empty()) {
		return 0;
	}

	EditorToolAction uac = undo_stack_.front();
	undo_stack_.pop_front();
	redo_stack_.push_front(uac);

	undo_button_.set_enabled(!undo_stack_.empty());
	redo_button_.set_enabled(true);

	return uac.tool.handle_undo(
	   static_cast<EditorTool::ToolIndex>(uac.i), uac.center, uac.args, &(uac.map));
}

uint32_t EditorHistory::redo_action() {
	if (redo_stack_.empty()) {
		return 0;
	}

	EditorToolAction rac = redo_stack_.front();
	redo_stack_.pop_front();
	undo_stack_.push_front(rac);

	undo_button_.set_enabled(true);
	redo_button_.set_enabled(!redo_stack_.empty());

	return rac.tool.handle_click(
	   static_cast<EditorTool::ToolIndex>(rac.i), rac.center, rac.args, &(rac.map));
}

uint32_t EditorHistory::do_action(EditorTool& tool,
                                  EditorTool::ToolIndex ind,
                                  Widelands::Map& map,
                                  const Widelands::NodeAndTriangle<Widelands::Coords>& center,
                                  bool draw) {
	EditorToolAction ac(
	   tool, static_cast<uint32_t>(ind), map, center, parent_, tool.format_args(ind));
	if (draw && tool.is_undoable()) {
		if (undo_stack_.empty() ||
		    undo_stack_.front().tool.get_sel_impl() != draw_tool_.get_sel_impl()) {
			EditorToolAction da(draw_tool_, EditorTool::First, map, center, parent_,
			                    draw_tool_.format_args(EditorTool::First));

			if (!undo_stack_.empty()) {
				draw_tool_.add_action(undo_stack_.front(), *da.args);
				undo_stack_.pop_front();
			}

			redo_stack_.clear();
			undo_stack_.push_front(da);
			undo_button_.set_enabled(true);
			redo_button_.set_enabled(false);
		}
		dynamic_cast<EditorDrawTool*>(&(undo_stack_.front().tool))
		   ->add_action(ac, *undo_stack_.front().args);
	} else if (tool.is_undoable()) {
		redo_stack_.clear();
		undo_stack_.push_front(ac);
		undo_button_.set_enabled(true);
		redo_button_.set_enabled(false);
		if (undo_stack_.size() > kMaximumUndoActions) {
			for (size_t i = 0; i < kTooManyUndoActionsDeleteBatch; ++i) {
				undo_stack_.pop_back();
			}
		}
	}
	return tool.handle_click(ind, center, ac.args, &map);
}
