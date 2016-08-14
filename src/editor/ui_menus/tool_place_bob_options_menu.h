/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_TOOL_PLACE_BOB_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_PLACE_BOB_OPTIONS_MENU_H

#include <vector>

#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/tabpanel.h"

struct EditorPlaceBobTool;
namespace UI {
struct Checkbox;
}

struct EditorToolPlaceBobOptionsMenu : public EditorToolOptionsMenu {
	EditorToolPlaceBobOptionsMenu(EditorInteractive&,
	                              EditorPlaceBobTool&,
	                              UI::UniqueWindow::Registry&);

private:
	void clicked(int32_t, bool);

	UI::TabPanel tabpanel_;
	std::vector<UI::Checkbox*> checkboxes_;
	EditorPlaceBobTool& pit_;
	bool click_recursion_protect_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_PLACE_BOB_OPTIONS_MENU_H
