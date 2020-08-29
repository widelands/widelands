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

#ifndef WL_EDITOR_TOOLS_SCENARIO_VISION_TOOL_H
#define WL_EDITOR_TOOLS_SCENARIO_VISION_TOOL_H

#include "editor/tools/tool.h"
#include "logic/see_unsee_node.h"

// Define which fields are explicitly revealed to the player
struct ScenarioVisionTool : public EditorTool {
	ScenarioVisionTool()
	   : EditorTool(*this, *this, false), player_(1), mode_(Widelands::SeeUnseeNode::kVisible) {
	}

	int32_t handle_click_impl(const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	EditorActionArgs format_args_impl(EditorInteractive& parent) override;

	const Image* get_sel_impl() const override {
		return g_image_cache->get("images/wui/editor/fsel_editor_vision.png");
	}

	uint8_t get_player() const {
		return player_;
	}
	void set_player(EditorInteractive&, uint8_t);
	Widelands::SeeUnseeNode get_mode() const {
		return mode_;
	}
	void set_mode(Widelands::SeeUnseeNode m) {
		mode_ = m;
	}

private:
	uint8_t player_;
	Widelands::SeeUnseeNode mode_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SCENARIO_VISION_TOOL_H
