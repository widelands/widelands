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

#ifndef WL_EDITOR_UI_MENUS_MAIN_MENU_MAP_OPTIONS_H
#define WL_EDITOR_UI_MENUS_MAIN_MENU_MAP_OPTIONS_H

#include "logic/note_map_options.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/dropdown.h"
#include "ui_basic/editbox.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilineeditbox.h"
#include "ui_basic/spinbox.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

/**
 * This is the Main Options Menu. Here, information
 * about the current map are displayed and you can change
 * author, name and description
 */
struct MainMenuMapOptions : public UI::UniqueWindow {
	MainMenuMapOptions(EditorInteractive&, UI::UniqueWindow::Registry& registry);

private:
	EditorInteractive& eia();
	void changed();
	void update();
	void clicked_ok();
	void clicked_cancel();
	void add_tag_checkbox(UI::Box* box, std::string tag, std::string displ_name);

	const unsigned int padding_, indent_, labelh_, checkbox_space_, butw_, max_w_;

	UI::Button ok_, cancel_;

	UI::Box tab_box_;
	UI::TabPanel tabs_;
	UI::Box main_box_;
	UI::Box tags_box_;
	UI::Box teams_box_;

	UI::EditBox name_, author_;
	UI::Textarea size_;
	UI::MultilineEditbox* descr_;
	UI::MultilineEditbox* hint_;

	// Tag, Checkbox
	std::map<std::string, UI::Checkbox*> tags_checkboxes_;
	UI::Dropdown<std::string> balancing_dropdown_;

	UI::Listselect<std::string> teams_list_;
	UI::SpinBox* waterway_length_box_;

	UI::UniqueWindow::Registry& registry_;
};

#endif  // end of include guard: WL_EDITOR_UI_MENUS_MAIN_MENU_MAP_OPTIONS_H
