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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "editor/ui_menus/tool_options_menu.h"

EditorToolOptionsMenu::EditorToolOptionsMenu(EditorInteractive& parent,
                                             UI::UniqueWindow::Registry& registry,
                                             uint32_t const width,
                                             uint32_t const height,
                                             const std::string& title,
                                             EditorTool& tool)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "tool_options_menu", &registry, width, height, title),
     parent_(parent),
     current_tool_(tool) {
	select_correct_tool();
	clicked.connect([this] { select_correct_tool(); });
}

void EditorToolOptionsMenu::select_correct_tool() {
	parent_.select_tool(current_tool_, EditorTool::First);
}
