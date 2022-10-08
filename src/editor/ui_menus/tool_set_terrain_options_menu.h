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

#ifndef WL_EDITOR_UI_MENUS_TOOL_SET_TERRAIN_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_TOOL_SET_TERRAIN_OPTIONS_MENU_H

#include <memory>

#include "editor/ui_menus/categorized_item_selection_menu.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "logic/map_objects/world/terrain_description.h"

class EditorInteractive;
struct EditorSetTerrainTool;

struct EditorToolSetTerrainOptionsMenu : public EditorToolOptionsMenu {
	EditorToolSetTerrainOptionsMenu(EditorInteractive&,
	                                EditorSetTerrainTool&,
	                                UI::UniqueWindow::Registry&);
	~EditorToolSetTerrainOptionsMenu() override = default;

	void update_window() override;

private:
	std::unique_ptr<
	   CategorizedItemSelectionMenu<Widelands::TerrainDescription, EditorSetTerrainTool>>
	   multi_select_menu_;
	std::vector<std::unique_ptr<const Image>> offscreen_images_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_TOOL_SET_TERRAIN_OPTIONS_MENU_H
