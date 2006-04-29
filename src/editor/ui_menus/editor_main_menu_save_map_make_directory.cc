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

#include "editor_main_menu_save_map_make_directory.h"
#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "system.h"
#include "wlapplication.h"

/*
 * Create this
 */
Main_Menu_Save_Map_Make_Directory::Main_Menu_Save_Map_Make_Directory(UIPanel* parent, const char* dirname) :
  UIWindow(parent, 0, 0, 230, 120, _("Make Directory")) {
     // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Make Directory"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // Command
   new UITextarea(this, spacing, posy, _("Enter Directory Name: "), Align_Left);
   posy+=20+spacing;

   // Filename editbox
   m_edit=new UIEdit_Box(this, spacing, posy, get_inner_w()-2*spacing, 20, 1, 0);
   m_edit->set_text(dirname);
   m_dirname=dirname;
   m_edit->changed.set(this, &Main_Menu_Save_Map_Make_Directory::edit_changed);

   // Buttons
   posx=5;
   posy=get_inner_h()-30;
   UIButton* but= new UIButton(this, get_inner_w()/2-spacing-80, posy, 80, 20, 0, 1);
   but->clickedid.set(this, &Main_Menu_Save_Map_Make_Directory::clicked);
   but->set_title(_("OK"));
   if(!m_dirname.size())
      but->set_enabled(false);
   m_ok_button=but;
   but= new UIButton(this, get_inner_w()/2+spacing, posy, 80, 20, 1, 0);
   but->clickedid.set(this, &Main_Menu_Save_Map_Make_Directory::clicked);
   but->set_title(_("Cancel"));

   center_to_parent();
}

/*
 * handle mouseclick for a modal
 */
bool Main_Menu_Save_Map_Make_Directory::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * Editbox changed
 */
void Main_Menu_Save_Map_Make_Directory::edit_changed(void) {
   std::string text=m_edit->get_text();
   if(text.size()) {
      m_ok_button->set_enabled(true);
      m_dirname=text;
   }
}

/*
 * Button has been clicked
 */
void Main_Menu_Save_Map_Make_Directory::clicked(int i) {
   end_modal(i);
}
