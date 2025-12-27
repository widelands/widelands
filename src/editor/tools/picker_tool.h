/*
 * Copyright (C) 2025 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_PICKER_TOOL_H
#define WL_EDITOR_TOOLS_PICKER_TOOL_H

#include <memory>

#include "editor/tools/tool.h"

struct EditorToolOptionsMenu;

/// A simple tool to pick a height, terrain, immovable, resource, or critter from a field.
struct EditorPickerTool : public EditorTool {
	explicit EditorPickerTool(EditorInteractive& parent) : EditorTool(parent, *this, *this, false) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	void set_linked_tool(EditorToolOptionsMenu* window) {
		linked_tool_window_ = window;
	}
	const EditorToolOptionsMenu* get_linked_tool() const {
		return linked_tool_window_;
	}

	[[nodiscard]] const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_pick.png");
	}

	[[nodiscard]] bool has_size_one() const override {
		return true;
	}

	[[nodiscard]] bool operates_on_triangles() const override;

private:
	EditorToolOptionsMenu* linked_tool_window_ {nullptr};
};

#endif  // end of include guard: WL_EDITOR_TOOLS_PICKER_TOOL_H
