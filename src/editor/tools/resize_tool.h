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

#ifndef WL_EDITOR_TOOLS_RESIZE_TOOL_H
#define WL_EDITOR_TOOLS_RESIZE_TOOL_H

#include "editor/tools/tool.h"

///  Resize the map
struct EditorResizeTool : public EditorTool {
	EditorResizeTool(int16_t width, int16_t height)
	   : EditorTool(*this, *this), width_(width), height_(height) {
	}

	/**
	 * Change the map size
	 */
	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& eia,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	int32_t handle_undo_impl(const Widelands::NodeAndTriangle<>& center,
	                         EditorInteractive& eia,
	                         EditorActionArgs* args,
	                         Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_resize.png");
	}

	bool has_size_one() const override {
		return true;
	}

	void set_width(uint32_t w) {
		width_ = w;
	}

	uint32_t get_width() {
		return width_;
	}

	void set_height(uint32_t h) {
		height_ = h;
	}

	uint32_t get_height() {
		return height_;
	}

private:
	uint32_t width_;
	uint32_t height_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_RESIZE_TOOL_H
