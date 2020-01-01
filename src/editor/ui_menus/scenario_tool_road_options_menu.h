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

#ifndef WL_EDITOR_UI_MENUS_SCENARIO_TOOL_ROAD_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_SCENARIO_TOOL_ROAD_OPTIONS_MENU_H

#include <memory>

#include "editor/tools/scenario_road_tool.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/radiobutton.h"

class EditorInteractive;

struct ScenarioToolRoadOptionsMenu : public EditorToolOptionsMenu {
	ScenarioToolRoadOptionsMenu(EditorInteractive&,
	                            ScenarioPlaceRoadTool&,
	                            UI::UniqueWindow::Registry&);
	~ScenarioToolRoadOptionsMenu() {
	}

protected:
	void think() override;

private:
	EditorInteractive& eia();
	ScenarioPlaceRoadTool& tool_;

	UI::Box main_box_;
	UI::Panel buttons_;
	UI::Checkbox force_, create_primary_, create_secondary_;
	UI::Radiogroup type_;
	UI::MultilineTextarea info_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_SCENARIO_TOOL_ROAD_OPTIONS_MENU_H
