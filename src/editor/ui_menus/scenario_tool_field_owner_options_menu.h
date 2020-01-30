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

#ifndef WL_EDITOR_UI_MENUS_SCENARIO_TOOL_FIELD_OWNER_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_SCENARIO_TOOL_FIELD_OWNER_OPTIONS_MENU_H

#include <memory>

#include "editor/tools/scenario_field_owner_tool.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "logic/map.h"
#include "ui_basic/dropdown.h"

class EditorInteractive;

/// Tool menu for setting the initial owner of fields
struct ScenarioToolFieldOwnerOptionsMenu : public EditorToolOptionsMenu {
	ScenarioToolFieldOwnerOptionsMenu(EditorInteractive&,
	                                  ScenarioFieldOwnerTool&,
	                                  UI::UniqueWindow::Registry&);
	~ScenarioToolFieldOwnerOptionsMenu() {
	}

private:
	EditorInteractive& eia();
	ScenarioFieldOwnerTool& tool_;

	void select();

	UI::Dropdown<Widelands::PlayerNumber> list_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_SCENARIO_TOOL_FIELD_OWNER_OPTIONS_MENU_H
