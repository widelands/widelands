/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_INFO_TOOL_H
#define WL_EDITOR_TOOLS_INFO_TOOL_H

#include "editor/tools/tool.h"

/// A simple tool to show information about the clicked node.
struct EditorInfoTool : public EditorTool {
	EditorInfoTool() : EditorTool(*this, *this, false) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	const Image* get_sel_impl() const override {
		return g_gr->images().get("images/wui/editor/fsel_editor_info.png");
	}

	bool has_size_one() const override {
		return true;
	}
};

#endif  // end of include guard: WL_EDITOR_TOOLS_INFO_TOOL_H
