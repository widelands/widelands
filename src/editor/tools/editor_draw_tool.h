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

#ifndef EDITOR_DRAW_TOOL_H
#define EDITOR_DRAW_TOOL_H

#include "editor/tools/editor_tool.h"
#include "editor/tools/editor_tool_action.h"

///  This is not a real editor tool. It serves to combine 'hold down mouse and move'
///  tool actions in one class.
struct Editor_Draw_Tool : public Editor_Tool {
	Editor_Draw_Tool() : Editor_Tool(*this, *this) {}

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	int32_t handle_undo_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	Editor_Action_Args format_args_impl(Editor_Interactive & parent) override;

	char const * get_sel_impl() const override {
		return "EDITOR_DRAW_TOOL";
	}

	void add_action(Editor_Tool_Action ac, Editor_Action_Args & args);

};

#endif

