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

#include "editor/ui_menus/tool_toolhistory_options_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/tools/tool.h"
#include "editor/tools/toolhistory_tool.h"

inline EditorInteractive& EditorToolhistoryMenu::eia() const {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
 */
EditorToolhistoryMenu::EditorToolhistoryMenu(EditorInteractive& parent,
                                             EditorHistoryTool& history_tool,
                                       UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "toolhistory_menu", &registry, 250, 30, _("Tool History")),
     history_tool_(history_tool) {

	initialization_complete();
}

