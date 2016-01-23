/*
 * Copyright (C) 2002-2004, 2008-2011 by the Widelands Development Team
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

#ifndef WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_H
#define WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

class EditorInteractive;

/**
 * This represents the main menu
*/
struct EditorMainMenu : public UI::UniqueWindow {
	EditorMainMenu(EditorInteractive &, UI::UniqueWindow::Registry &);

private:
	EditorInteractive & eia();
	UI::Box box_;
	UI::Button button_new_map_;
	UI::Button button_new_random_map_;
	UI::Button button_load_map_;
	UI::Button button_save_map_;
	UI::Button button_map_options_;
	UI::Button button_view_readme_;
	UI::Button button_exit_editor_;

	UI::UniqueWindow::Registry window_readme_;

	void exit_btn       ();
	void load_btn       ();
	void save_btn       ();
	void new_map_btn    ();
	void new_random_map_btn    ();
	void map_options_btn();
};


#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_H
