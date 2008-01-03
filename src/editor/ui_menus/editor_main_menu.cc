/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "editor_main_menu.h"
#include "editor_main_menu_load_map.h"
#include "editor_main_menu_map_options.h"
#include "editor_main_menu_new_map.h"
#include "editor_main_menu_save_map.h"
#include "editorinteractive.h"
#include "fullscreen_menu_fileview.h"
#include "i18n.h"

/*
===============
Editor_Main_Menu::Editor_Main_Menu

Create all the buttons etc...
===============
*/
#define width 120
#define height 20
#define margin 15
#define hmargin margin
#define vmargin margin
#define vspacing 15
Editor_Main_Menu::Editor_Main_Menu(Editor_Interactive *parent, UI::UniqueWindow::Registry *registry)
:
UI::UniqueWindow(parent, registry, 2 * hmargin + width, 225, _("Main Menu")),
m_parent(*parent),
m_button_new_map
(this,
 hmargin, vmargin + 0 * (height + vspacing), width, height,
 1,
 &Editor_Main_Menu::new_map_btn, this,
 _("New Map")),

m_button_load_map
(this,
 hmargin, vmargin + 1 * (height + vspacing), width, height,
 1,
 &Editor_Main_Menu::load_btn, this,
 _("Load Map")),

m_button_save_map
(this,
 hmargin, vmargin + 2 * (height + vspacing), width, height,
 1,
 &Editor_Main_Menu::save_btn, this,
 _("Save Map")),

m_button_map_options
(this,
 hmargin, vmargin + 3 * (height + vspacing), width, height,
 1,
 &Editor_Main_Menu::map_options_btn, this,
 _("Map Options")),

m_button_view_readme
(this,
 hmargin, vmargin + 4 * (height + vspacing), width, height,
 1,
 &Editor_Main_Menu::readme_btn, this,
 _("View Readme")),

m_button_exit_editor
(this,
 hmargin, vmargin + 5 * (height + vspacing), width, height,
 0,
 &Editor_Main_Menu::exit_btn, this,
 _("Exit Editor"))
{
	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===========
Editor_Main_Menu UI::Button functions

called, when buttons get clicked
===========
*/
void Editor_Main_Menu::new_map_btn() {
	new Main_Menu_New_Map(&m_parent);
	die();
}
void Editor_Main_Menu::load_btn() {
	new Main_Menu_Load_Map(&m_parent);
	die();
}

void Editor_Main_Menu::save_btn() {
	new Main_Menu_Save_Map(&m_parent);
	die();
}
void Editor_Main_Menu::map_options_btn() {
	new Main_Menu_Map_Options(&m_parent);
	die();
}
void Editor_Main_Menu::exit_btn() {m_parent.exit();}
void Editor_Main_Menu::readme_btn() {
	fileview_window(&m_parent, &m_window_readme, "txts/editor_readme");
}
