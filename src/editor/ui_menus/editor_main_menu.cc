/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "editor/ui_menus/editor_main_menu_load_map.h"
#include "editor/ui_menus/editor_main_menu_map_options.h"
#include "editor/ui_menus/editor_main_menu_new_map.h"
#include "editor/ui_menus/editor_main_menu_random_map.h"
#include "editor/ui_menus/editor_main_menu_save_map.h"
#include "ui_fsmenu/fileview.h"

//TODO(unknown): these should be defined globally for the whole UI
#define width 200
#define height 20
#define margin 15
#define hmargin margin
#define vmargin margin
#define vspacing 15

// NOCOM(GunChleoc): buttons need more height for Arabic.

inline EditorInteractive & EditorMainMenu::eia() {
	return dynamic_cast<EditorInteractive&>(*get_parent());
}

/**
 * Create all the buttons etc...
*/
EditorMainMenu::EditorMainMenu
	(EditorInteractive & parent, UI::UniqueWindow::Registry & registry)
:
	UI::UniqueWindow
		(&parent, "main_menu", &registry, 2 * hmargin + width,
		 260, _("Main Menu")),
	m_button_new_map
		(this, "new_map",
		 hmargin, vmargin + 0 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but1.png"),
		 _("New Map")),
	m_button_new_random_map
		(this, "new_random_map",
		 hmargin, vmargin + 1 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but1.png"),
		 _("New Random Map")),
	m_button_load_map
		(this, "load_map",
		 hmargin, vmargin + 2 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but1.png"),
		 _("Load Map")),
	m_button_save_map
		(this, "save_map",
		 hmargin, vmargin + 3 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but1.png"),
		 _("Save Map")),
	m_button_map_options
		(this, "map_options",
		 hmargin, vmargin + 4 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but1.png"),
		 _("Map Options")),
	m_button_view_readme
		(this, "readme",
		 hmargin, vmargin + 5 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but1.png"),
		 _("View Readme")),
	m_button_exit_editor
		(this, "exit",
		 hmargin, vmargin + 6 * (height + vspacing), width, height,
		 g_gr->images().get("pics/but0.png"),
		 _("Exit Editor"))
{
	m_button_new_map.sigclicked.connect(boost::bind(&EditorMainMenu::new_map_btn, this));
	m_button_new_random_map.sigclicked.connect(boost::bind(&EditorMainMenu::new_random_map_btn, this));
	m_button_load_map.sigclicked.connect(boost::bind(&EditorMainMenu::load_btn, this));
	m_button_save_map.sigclicked.connect(boost::bind(&EditorMainMenu::save_btn, this));
	m_button_map_options.sigclicked.connect(boost::bind(&EditorMainMenu::map_options_btn, this));

	m_window_readme.open_window = [this] {
		fileview_window(eia(), m_window_readme, "txts/editor_readme.lua");
	};
	m_button_view_readme.sigclicked.connect(
	   boost::bind(&UI::UniqueWindow::Registry::toggle, m_window_readme));

	m_button_exit_editor.sigclicked.connect(boost::bind(&EditorMainMenu::exit_btn, this));

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/**
 * Called, when buttons get clicked
*/
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
void EditorMainMenu::exit_btn() {eia().exit();}
