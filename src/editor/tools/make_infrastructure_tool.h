/*
 * Copyright (C) 2002-2004, 2006-2008, 2012 by the Widelands Development Team
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

#ifndef WL_EDITOR_TOOLS_MAKE_INFRASTRUCTURE_TOOL_H
#define WL_EDITOR_TOOLS_MAKE_INFRASTRUCTURE_TOOL_H

#include "editor/tools/tool.h"
#include "ui_basic/unique_window.h"

namespace Widelands {
class EditorGameBase;
}  // namespace Widelands

/**
 * This places immovables on the map
 */
// TODO(unknown):  Implement undo for this tool
struct EditorMakeInfrastructureTool : public EditorTool {
	EditorMakeInfrastructureTool() : EditorTool(*this, *this, false), player_(0) {
	}

	void set_player(Widelands::PlayerNumber const n) {
		player_ = n;
	}
	Widelands::PlayerNumber get_player() const {
		return player_;
	}

	int32_t handle_click_impl(const Widelands::World& world,
	                          const Widelands::NodeAndTriangle<>& center,
	                          EditorInteractive& parent,
	                          EditorActionArgs* args,
	                          Widelands::Map* map) override;

	const char* get_sel_impl() const override {
		return "images/ui_basic/fsel.png";
	}  //  Standard sel icon, most complex tool of all

private:
	Widelands::PlayerNumber player_;
	UI::UniqueWindow::Registry registry_;
};

int32_t editor_make_infrastructure_tool_callback(const Widelands::TCoords<Widelands::FCoords>& c,
                                                 Widelands::EditorGameBase& egbase,
                                                 int32_t const player);

#endif  // end of include guard: WL_EDITOR_TOOLS_MAKE_INFRASTRUCTURE_TOOL_H
