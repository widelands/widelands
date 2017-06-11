/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/main_menu_load_map.h"
#include "editor/ui_menus/main_menu_map_options.h"
#include "editor/ui_menus/main_menu_new_map.h"
#include "editor/ui_menus/main_menu_random_map.h"
#include "editor/ui_menus/main_menu_save_map.h"

// TODO(unknown): these should be defined globally for the whole UI
#define width 200
#define margin 15
#define vspacing 15

inline EditorInteractive& EditorMainMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
*/
EditorMainMenu::EditorMainMenu(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : UI::UniqueWindow(&parent, "main_menu", &registry, 2 * margin + width, 0, _("Main Menu")),
     box_(this, margin, margin, UI::Box::Vertical, width, get_h() - 2 * margin, vspacing),
     button_new_map_(
        &box_, "new_map", 0, 0, width, 0, UI::Button::Style::kWuiSecondary, _("New Map")),
     button_new_random_map_(&box_,
                            "new_random_map",
                            0,
                            0,
                            width,
                            0,
                            UI::Button::Style::kWuiSecondary,
                            _("New Random Map")),
     button_load_map_(
        &box_, "load_map", 0, 0, width, 0, UI::Button::Style::kWuiSecondary, _("Load Map")),
     button_save_map_(
        &box_, "save_map", 0, 0, width, 0, UI::Button::Style::kWuiSecondary, _("Save Map")),
     button_map_options_(
        &box_, "map_options", 0, 0, width, 0, UI::Button::Style::kWuiSecondary, _("Map Options")),
     button_exit_editor_(
        &box_, "exit", 0, 0, width, 0, UI::Button::Style::kWuiPrimary, _("Exit Editor")) {
	box_.add(&button_new_map_);
	box_.add(&button_new_random_map_);
	box_.add(&button_load_map_);
	box_.add(&button_save_map_);
	box_.add(&button_map_options_);
	box_.add(&button_exit_editor_);
	box_.set_size(width, 6 * button_new_map_.get_h() + 5 * vspacing);
	set_inner_size(get_inner_w(), box_.get_h() + 2 * margin);

	button_new_map_.sigclicked.connect(boost::bind(&EditorMainMenu::new_map_btn, this));
	button_new_random_map_.sigclicked.connect(
	   boost::bind(&EditorMainMenu::new_random_map_btn, this));
	button_load_map_.sigclicked.connect(boost::bind(&EditorMainMenu::load_btn, this));
	button_save_map_.sigclicked.connect(boost::bind(&EditorMainMenu::save_btn, this));
	button_map_options_.sigclicked.connect(boost::bind(&EditorMainMenu::map_options_btn, this));
	button_exit_editor_.sigclicked.connect(boost::bind(&EditorMainMenu::exit_btn, this));

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

void EditorMainMenu::new_map_btn() {
	new MainMenuNewMap(eia());
	die();
}

void EditorMainMenu::new_random_map_btn() {
	new MainMenuNewRandomMap(eia());
	die();
}

void EditorMainMenu::load_btn() {
	new MainMenuLoadMap(eia());
	die();
}

void EditorMainMenu::save_btn() {
	new MainMenuSaveMap(eia());
	die();
}
void EditorMainMenu::map_options_btn() {
	new MainMenuMapOptions(eia());
	die();
}
void EditorMainMenu::exit_btn() {
	eia().exit();
}
