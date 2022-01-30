/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_INCREASE_RESOURCES_TOOL_H
#define WL_EDITOR_TOOLS_INCREASE_RESOURCES_TOOL_H

#include "editor/tools/decrease_resources_tool.h"
#include "editor/tools/set_resources_tool.h"
#include "logic/widelands_geometry.h"

/// Increases the resources of a node by a value.
struct EditorIncreaseResourcesTool : public EditorTool {
	EditorIncreaseResourcesTool(EditorDecreaseResourcesTool& the_decrease_tool,
	                            EditorSetResourcesTool& the_set_to_tool)
	   : EditorTool(the_decrease_tool, the_set_to_tool),
	     decrease_tool_(the_decrease_tool),
	     set_tool_(the_set_to_tool),
	     change_by_(1),
	     cur_res_(0) {
	}

	/***
	 * Increase the resources of the current field by one if there is not already
	 * another resource there.
	 */
	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorInteractive& parent,
	                         EditorActionArgs* args,
	                         Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_increase_resources.png");
	}

	Widelands::NodeCaps nodecaps_for_buildhelp(const Widelands::FCoords& fcoords,
	                                           const Widelands::EditorGameBase& egbase) override {
		return resource_tools_nodecaps(fcoords, egbase, cur_res_);
	}

	int32_t get_change_by() const {
		return change_by_;
	}
	void set_change_by(const int32_t n) {
		change_by_ = n;
	}
	Widelands::DescriptionIndex get_cur_res() const {
		return cur_res_;
	}
	void set_cur_res(Widelands::DescriptionIndex const res) {
		cur_res_ = res;
	}

	EditorDecreaseResourcesTool& decrease_tool() const {
		return decrease_tool_;
	}
	EditorSetResourcesTool& set_tool() const {
		return set_tool_;
	}

private:
	EditorDecreaseResourcesTool& decrease_tool_;
	EditorSetResourcesTool& set_tool_;
	int32_t change_by_;
	Widelands::DescriptionIndex cur_res_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_INCREASE_RESOURCES_TOOL_H
