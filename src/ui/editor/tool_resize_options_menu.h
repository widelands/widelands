/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_UI_EDITOR_TOOL_RESIZE_OPTIONS_MENU_H
#define WL_UI_EDITOR_TOOL_RESIZE_OPTIONS_MENU_H

#include "ui/editor/map_size_box.h"
#include "ui/editor/tool_options_menu.h"
#include "ui/basic/box.h"
#include "ui/basic/multilinetextarea.h"

class EditorInteractive;
struct EditorResizeTool;

struct EditorToolResizeOptionsMenu : public EditorToolOptionsMenu {
	EditorToolResizeOptionsMenu(EditorInteractive&, EditorResizeTool&, UI::UniqueWindow::Registry&);

private:
	EditorInteractive& eia();
	void update_dimensions();

	EditorResizeTool& resize_tool_;
	UI::Box box_;
	MapSizeBox map_size_box_;
	UI::MultilineTextarea text_area_;
};

#endif  // end of include guard: WL_UI_EDITOR_TOOL_RESIZE_OPTIONS_MENU_H
