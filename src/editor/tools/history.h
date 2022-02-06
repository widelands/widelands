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

#ifndef WL_EDITOR_TOOLS_HISTORY_H
#define WL_EDITOR_TOOLS_HISTORY_H

#include <deque>

#include "editor/tools/draw_tool.h"
#include "editor/tools/tool.h"
#include "ui_basic/button.h"

/**
 * The all actions done with an editor tool are saved on a stack to
 * provide undo / redo functionality.
 * Do all tool action you want to make "undoable" using this class.
 */
struct EditorHistory {
	EditorHistory(UI::Button& undo, UI::Button& redo) : undo_button_(undo), redo_button_(redo) {
	}

	uint32_t do_action(EditorTool& tool,
	                   EditorTool::ToolIndex ind,
	                   Widelands::Map& map,
	                   const Widelands::NodeAndTriangle<>& center,
	                   EditorInteractive& parent,
	                   bool draw = false);
	uint32_t undo_action();
	uint32_t redo_action();

private:
	UI::Button& undo_button_;
	UI::Button& redo_button_;

	EditorDrawTool draw_tool_;

	std::deque<EditorToolAction> undo_stack_;
	std::deque<EditorToolAction> redo_stack_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_HISTORY_H
