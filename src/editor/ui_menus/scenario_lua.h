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

#ifndef WL_EDITOR_UI_MENUS_SCENARIO_LUA_H
#define WL_EDITOR_UI_MENUS_SCENARIO_LUA_H

#include <map>
#include <memory>

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/listselect.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

/// The control center for the scenario scripting
struct ScenarioLuaEditor : public UI::UniqueWindow {
	ScenarioLuaEditor(EditorInteractive&, UI::UniqueWindow::Registry&);

private:
	EditorInteractive& eia();

	UI::TabPanel tabs_;

	void update_variables(std::string sel = "");
	void update_functions(std::string sel = "");
	void update_variable_buttons();
	void update_function_body();

	void clicked_add_variable();
	void clicked_edit_variable();
	void clicked_delete_variable();
	void clicked_add_function();
	void clicked_edit_function();
	void clicked_delete_function();

	// Functions tab
	std::unique_ptr<UI::Box> functions_box_, functions_buttonbox_, functions_lowerbox_,
	   functions_sidepanel_;
	std::unique_ptr<UI::Button> functions_button_add_, functions_button_edit_,
	   functions_button_delete_;
	std::unique_ptr<UI::Dropdown<std::string>> functions_dropdown_;
	std::unique_ptr<UI::Listselect<std::string>> functions_body_;

	// Variables tab
	std::unique_ptr<UI::Box> variables_box_, variables_buttonbox_;
	std::unique_ptr<UI::Button> variables_button_add_, variables_button_edit_,
	   variables_button_delete_;
	std::unique_ptr<UI::Table<uintptr_t>> variables_list_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_SCENARIO_LUA_H
