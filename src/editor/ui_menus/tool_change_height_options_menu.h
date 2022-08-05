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

#ifndef WL_EDITOR_UI_MENUS_TOOL_CHANGE_HEIGHT_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_CHANGE_HEIGHT_OPTIONS_MENU_H

#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/box.h"
#include "ui_basic/spinbox.h"

class EditorInteractive;
struct EditorIncreaseHeightTool;

struct EditorToolChangeHeightOptionsMenu : public EditorToolOptionsMenu {
	EditorToolChangeHeightOptionsMenu(EditorInteractive&,
	                                  EditorIncreaseHeightTool&,
	                                  UI::UniqueWindow::Registry&);

	void update_window() override;

private:
	EditorIncreaseHeightTool& increase_tool_;
	UI::Box box_;
	UI::SpinBox change_by_;
	UI::SpinBox set_to_;

	void update_change_by();
	void update_set_to();
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_CHANGE_HEIGHT_OPTIONS_MENU_H
