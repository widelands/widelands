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

#include <string>
#include "ui_textarea.h"
#include "ui_editbox.h"
#include "ui_button.h"
#include "ui_modal_messagebox.h"
#include "editor.h"
#include "editorinteractive.h"
#include "editor_main_menu_save_map.h"
#include "widelands_map_saver.h"
#include "wexception.h"
#include "error.h"

/*
===============
Main_Menu_Save_Map::Main_Menu_Save_Map

Create all the buttons etc...
===============
*/
Main_Menu_Save_Map::Main_Menu_Save_Map(Editor_Interactive *parent)
	: UIWindow(parent, (parent->get_w()-200)/2, (parent->get_h()-190)/2, 200, 110, "Save Map")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Save Map", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   int posx=5;
   int posy=30;
   UITextarea* ta= new UITextarea(this, posx, posy+5, "Enter File Name", Align_Center);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   
   posy+=20;
   m_editbox=new UIEdit_Box(this, posx, posy, get_inner_w()-(posx*2), 20, 1, 0);
   m_editbox->set_text("noname");
   posy+=30;
   
   UIButton* but= new UIButton(this, (get_inner_w()-60)/2, posy, 60, 20, 0, 0);
   but->clickedid.set(this, &Main_Menu_Save_Map::clicked);
   but->set_title("OK");

}

/*
===============
Main_Menu_Save_Map::~Main_Menu_Save_Map

Unregister from the registry pointer
===============
*/
Main_Menu_Save_Map::~Main_Menu_Save_Map()
{
}

/*
===========
called when the ok button has been clicked 
===========
*/
void Main_Menu_Save_Map::clicked(int id) {
   std::string filename=m_editbox->get_text();
   if(filename!="") {
      std::string realname = "maps/";
      realname+=filename;
      Widelands_Map_Saver* wms=new Widelands_Map_Saver(realname, m_parent->get_editor());
      try {
         wms->save();
      } catch(std::exception& exe) {
         std::string s="Map Saving Error!\nSaved Map-File may be corrupt!\n\nReason given:\n";
         s+=exe.what();
         UIModal_Message_Box* mbox= new UIModal_Message_Box(m_parent, "Save Map Error!!", s, UIModal_Message_Box::OK);
         mbox->run();
         delete mbox;
      }
      delete wms;
   }
   die();
}


