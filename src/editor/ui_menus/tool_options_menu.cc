/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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
                                             char const* const title)
   : UI::UniqueWindow(&parent, "tool_options_menu", &registry, width, height, title),
     current_pointer_(parent.tools()->current_pointer) {
	if (get_usedefaultpos())
		center_to_parent();
}

void EditorToolOptionsMenu::select_correct_tool() {
	dynamic_cast<EditorInteractive&>(*get_parent())
	   .select_tool(*current_pointer_, EditorTool::First);
}
