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

#ifndef WL_EDITOR_UI_MENUS_SCENARIO_TOOL_WORKER_OPTIONS_MENU_H
#define WL_EDITOR_UI_MENUS_SCENARIO_TOOL_WORKER_OPTIONS_MENU_H

#include <memory>

#include "editor/tools/scenario_worker_tool.h"
#include "editor/ui_menus/tool_options_menu.h"
#include "ui_basic/box.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/icongrid.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/spinbox.h"

class EditorInteractive;

// NOCOM refactor: Use a tabpanel with one tab per player, offering only the player's tribe's
// workers. Get rid of the shipname box. Remove the no-tribe/tribe-changed code.

struct ScenarioToolWorkerOptionsMenu : public EditorToolOptionsMenu {
	ScenarioToolWorkerOptionsMenu(EditorInteractive&,
	                              ScenarioPlaceWorkerTool&,
	                              UI::UniqueWindow::Registry&);
	~ScenarioToolWorkerOptionsMenu() {
	}

	void update();

private:
	EditorInteractive& eia();
	ScenarioPlaceWorkerTool& tool_;

	void select_player();
	void select_ware();
	void toggle_item(int32_t);
	void update_text_and_spinner();

	UI::Box box_, bottombox_;
	UI::Dropdown<Widelands::PlayerNumber> players_;
	UI::IconGrid item_types_;
	UI::SpinBox experience_;
	UI::Dropdown<Widelands::DescriptionIndex> ware_;
	UI::EditBox shipname_;
	UI::MultilineTextarea selected_items_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_SCENARIO_TOOL_WORKER_OPTIONS_MENU_H
