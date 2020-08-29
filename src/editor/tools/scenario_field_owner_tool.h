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

#ifndef WL_EDITOR_TOOLS_SCENARIO_FIELD_OWNER_TOOL_H
#define WL_EDITOR_TOOLS_SCENARIO_FIELD_OWNER_TOOL_H

#include "editor/tools/tool.h"

// Define which map regions initially belong to which player
struct ScenarioFieldOwnerTool : public EditorTool {
	ScenarioFieldOwnerTool() : EditorTool(*this, *this), new_owner_(0) {
	}

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
		return g_image_cache->get("images/wui/editor/fsel_editor_field_owner.png");
	}

	uint8_t get_new_owner() const {
		return new_owner_;
	}
	void set_new_owner(const uint8_t o) {
		new_owner_ = o;
	}

private:
	uint8_t new_owner_;
};

#endif  // end of include guard: WL_EDITOR_TOOLS_SCENARIO_FIELD_OWNER_TOOL_H
