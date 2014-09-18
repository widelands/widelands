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

#include "ui_basic/button.h"
#include "ui_basic/unique_window.h"

struct EditorInteractive;

/**
 * This represents the main menu
*/
struct EditorMainMenu : public UI::UniqueWindow {
	EditorMainMenu(EditorInteractive &, UI::UniqueWindow::Registry &);

private:
	EditorInteractive & eia();
	UI::Button m_button_new_map;
	UI::Button m_button_new_random_map;
	UI::Button m_button_load_map;
	UI::Button m_button_save_map;
	UI::Button m_button_map_options;
	UI::Button m_button_view_readme;
	UI::Button m_button_exit_editor;

	UI::UniqueWindow::Registry m_window_readme;

	void exit_btn       ();
	void load_btn       ();
	void save_btn       ();
	void new_map_btn    ();
	void new_random_map_btn    ();
	void map_options_btn();
};


#endif  // end of include guard: WL_EDITOR_UI_MENUS_EDITOR_MAIN_MENU_H
