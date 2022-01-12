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

#ifndef WL_EDITOR_UI_MENUS_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H

#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/box.h"
#include "ui_basic/spinbox.h"

class EditorInteractive;
struct EditorNoiseHeightTool;

struct EditorToolNoiseHeightOptionsMenu : public EditorToolOptionsMenu {
	EditorToolNoiseHeightOptionsMenu(EditorInteractive&,
	                                 EditorNoiseHeightTool&,
	                                 UI::UniqueWindow::Registry&);

private:
	void update_interval(int32_t lower, int32_t upper);
	void update_upper();
	void update_lower();
	void update_set_to();

	EditorNoiseHeightTool& noise_tool_;
	UI::Box box_;
	UI::SpinBox lower_, upper_;
	UI::SpinBox set_to_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H
