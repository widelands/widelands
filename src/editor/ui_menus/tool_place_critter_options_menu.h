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

#ifndef WL_EDITOR_UI_MENUS_TOOL_PLACE_CRITTER_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_PLACE_CRITTER_OPTIONS_MENU_H

#include <memory>

#include "editor/tools/place_critter_tool.h"
#include "editor/ui_menus/categorized_item_selection_menu.h"
#include "editor/ui_menus/tool_options_menu.h"

class EditorInteractive;
namespace Widelands {
class CritterDescr;
}

struct EditorToolPlaceCritterOptionsMenu : public EditorToolOptionsMenu {
	EditorToolPlaceCritterOptionsMenu(EditorInteractive&,
	                                  EditorPlaceCritterTool&,
	                                  UI::UniqueWindow::Registry&);
	~EditorToolPlaceCritterOptionsMenu() override = default;

private:
	std::unique_ptr<CategorizedItemSelectionMenu<Widelands::CritterDescr, EditorPlaceCritterTool>>
	   multi_select_menu_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_PLACE_CRITTER_OPTIONS_MENU_H
