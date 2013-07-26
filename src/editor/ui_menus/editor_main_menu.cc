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

#include "editor/editorinteractive.h"
#include "editor/ui_menus/editor_main_menu_load_map.h"
#include "editor/ui_menus/editor_main_menu_map_options.h"
#include "editor/ui_menus/editor_main_menu_new_map.h"
#include "editor/ui_menus/editor_main_menu_random_map.h"
#include "editor/ui_menus/editor_main_menu_save_map.h"
#include "i18n.h"
#include "ui_fsmenu/fileview.h"

//TODO: these should be defined globally for the whole UI
#define width 150
#define height 20
#define margin 15
#define hmargin margin
#define vmargin margin
#define vspacing 15

inline Editor_Interactive & Editor_Main_Menu::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}

/**
 * Create all the buttons etc...
*/
Editor_Main_Menu::Editor_Main_Menu
	(Editor_Interactive & parent, UI::UniqueWindow::Registry & registry)
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
	m_button_new_map.sigclicked.connect(boost::bind(&Editor_Main_Menu::new_map_btn, this));
	m_button_new_random_map.sigclicked.connect(boost::bind(&Editor_Main_Menu::new_random_map_btn, this));
	m_button_load_map.sigclicked.connect(boost::bind(&Editor_Main_Menu::load_btn, this));
	m_button_save_map.sigclicked.connect(boost::bind(&Editor_Main_Menu::save_btn, this));
	m_button_map_options.sigclicked.connect(boost::bind(&Editor_Main_Menu::map_options_btn, this));
	m_button_view_readme.sigclicked.connect(boost::bind(&Editor_Main_Menu::readme_btn, this));
	m_button_exit_editor.sigclicked.connect(boost::bind(&Editor_Main_Menu::exit_btn, this));

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/**
 * Called, when buttons get clicked
*/
void Editor_Main_Menu::new_map_btn() {
	new Main_Menu_New_Map(eia());
	die();
}

void Editor_Main_Menu::new_random_map_btn() {
	new Main_Menu_New_Random_Map(eia());
	die();
}

void Editor_Main_Menu::load_btn() {
	new Main_Menu_Load_Map(eia());
	die();
}

void Editor_Main_Menu::save_btn() {
	new Main_Menu_Save_Map(eia());
	die();
}
void Editor_Main_Menu::map_options_btn() {
	new Main_Menu_Map_Options(eia());
	die();
}
void Editor_Main_Menu::exit_btn() {eia().exit();}
void Editor_Main_Menu::readme_btn() {
	fileview_window(eia(), m_window_readme, "txts/editor_readme");
}
