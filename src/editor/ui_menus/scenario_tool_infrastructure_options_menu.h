/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_SCENARIO_TOOL_INFRASTRUCTURE_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_SCENARIO_TOOL_INFRASTRUCTURE_OPTIONS_MENU_H

#include <memory>
#include <vector>

#include "editor/tools/scenario_infrastructure_tool.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "logic/map.h"
#include "notifications/notifications.h"
#include "ui_basic/box.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/icongrid.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/tabpanel.h"

class EditorInteractive;

/// Tool menu for placing buldings, flags and player immovables
struct ScenarioToolInfrastructureOptionsMenu : public EditorToolOptionsMenu {
	ScenarioToolInfrastructureOptionsMenu(EditorInteractive&,
	                                      ScenarioInfrastructureTool&,
	                                      UI::UniqueWindow::Registry&);
	~ScenarioToolInfrastructureOptionsMenu() {
	}

	void update_players();
	void update_text();

private:
	EditorInteractive& eia();
	ScenarioInfrastructureTool& tool_;

	void select_player();
	void toggle_selected(UI::IconGrid*, Widelands::MapObjectType, int32_t);

	std::unique_ptr<UI::Box> main_box_;
	std::unique_ptr<UI::Dropdown<Widelands::PlayerNumber>> players_;
	std::unique_ptr<UI::Checkbox> force_;
	std::unique_ptr<UI::Checkbox> construct_;
	std::unique_ptr<UI::TabPanel> item_categories_;
	std::vector<std::unique_ptr<UI::TabPanel>> item_tabs_;
	std::vector<std::unique_ptr<UI::IconGrid>> item_grids_;
	std::unique_ptr<UI::MultilineTextarea> selected_items_;

	std::unique_ptr<Notifications::Subscriber<Widelands::NoteEditorPlayerEdited>> subscriber_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_SCENARIO_TOOL_INFRASTRUCTURE_OPTIONS_MENU_H
