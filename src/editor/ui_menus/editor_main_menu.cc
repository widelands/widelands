/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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
#include "ui_unique_window.h"
#include "ui_textarea.h"
#include "editorinteractive.h"
#include "ui_button.h"
#include "editor_main_menu_new_map.h"
#include "editor_main_menu_map_options.h"
#include "error.h"
#include "fullscreen_menu_fileview.h"

static const char* EDITOR_README =
"   README for the Widelands Map Editor\n"
"\n"
"\n"
"Introduction\n"
"------------\n"
"\n"
"This Editor is intended for players who'd like to design their own maps to use with Widelands "
"As you can see, this Editor is heavy work in progress and as the Editor becomes better and better "
"this text will also get longer and more complete. Probably it will also contain a little tutorial "
"in a while\n"
"\n"
"\n"
"Keyboard shortcuts\n"
"------------------\n"
"\n"
"SPACE    Toggles build-help\n"
"M        Toggles minimap\n"
"T        Toggles tools menu\n"
"\n"
"1-7      Changes tools size\n"
"SHIFT    (Hold) Selects first alternative tool while pressed\n"
"ALT      (Hold) Selects second alternative tool while pressed\n"
"\n"
"\n";



/*
===============
Editor_Main_Menu::Editor_Main_Menu

Create all the buttons etc...
===============
*/
Editor_Main_Menu::Editor_Main_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 130, 200, "Main Menu")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Editor Main Menu", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // UIButtons
   const int offsx=5;
   const int offsy=30;
   const int spacing=5;
   const int width=get_inner_w()-offsx*2;
   const int height=20;
   int posx=offsx;
   int posy=offsy;

   UIButton* b=new UIButton(this, posx, posy, width, height, 1);
   b->set_title("New Map");
   b->clicked.set(this, &Editor_Main_Menu::new_map_btn);
   posy+=height+spacing;

   b=new UIButton(this, posx, posy, width, height, 1);
   b->set_title("Load Map");
   b->clicked.set(this, &Editor_Main_Menu::load_btn);
   posy+=height+spacing;

   b=new UIButton(this, posx, posy, width, height, 1);
   b->set_title("Save Map");
   b->clicked.set(this, &Editor_Main_Menu::save_btn);
   posy+=height+spacing;

   posy+=spacing;
   b=new UIButton(this, posx, posy, width, height, 1);
   b->set_title("Map Options");
   b->clicked.set(this, &Editor_Main_Menu::map_options_btn);
   posy+=height+spacing;

   posy+=spacing;
   b=new UIButton(this, posx, posy, width, height, 1);
   b->set_title("View Readme");
   b->clicked.set(this, &Editor_Main_Menu::readme_btn);
   posy+=height+spacing;

   posy+=spacing;
   b=new UIButton(this, posx, posy, width, height, 0);
   b->set_title("Exit Editor");
   b->clicked.set(this, &Editor_Main_Menu::exit_btn);
   posy+=height+spacing;

	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===========
Editor_Main_Menu UIButton functions

called, when buttons get clicked
===========
*/
void Editor_Main_Menu::new_map_btn() {
   new Main_Menu_New_Map(m_parent);
   delete this;
}
void Editor_Main_Menu::load_btn() {
   log("TODO: Editor_Main_Menu::load_btn()\n");
   /*
   m_parent->get_editor()->get_objects()->cleanup(m_parent->get_editor());
   g_anim.flush();

   //map_select_menue(m_parent->get_editor());
   //m_parent->map_changed();

   g_gr->flush(PicMod_Menu);

   m_parent->get_editor()->postload();
   m_parent->get_editor()->load_graphics();
*/
}

void Editor_Main_Menu::save_btn() {
   log("TODO: Editor_Main_Menu::save_btn()\n");
}
void Editor_Main_Menu::map_options_btn() {
   new Main_Menu_Map_Options(m_parent);
   delete this;
}
void Editor_Main_Menu::exit_btn() {
   m_parent->exit_editor();
}
void Editor_Main_Menu::readme_btn() {
   textview_window(m_parent, &m_window_readme, "Editor README", EDITOR_README);
}

/*
===============
Editor_Main_Menu::~Editor_Main_Menu

Unregister from the registry pointer
===============
*/
Editor_Main_Menu::~Editor_Main_Menu()
{
}


