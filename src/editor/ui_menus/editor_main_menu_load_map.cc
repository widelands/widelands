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
#include "editor_main_menu_load_map.h"
#include "widelands_map_loader.h"
#include "wexception.h"
#include "error.h"
#include "editor_game_base.h"

/*
===============
Main_Menu_Load_Map::Main_Menu_Load_Map

Create all the buttons etc...
===============
*/
Main_Menu_Load_Map::Main_Menu_Load_Map(Editor_Interactive *parent)
	: UIWindow(parent, (parent->get_w()-200)/2, (parent->get_h()-190)/2, 200, 110, "Load Map")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Load Map", Align_Left);
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
   but->clickedid.set(this, &Main_Menu_Load_Map::clicked);
   but->set_title("OK");

}

/*
===============
Main_Menu_Load_Map::~Main_Menu_Load_Map

Unregister from the registry pointer
===============
*/
Main_Menu_Load_Map::~Main_Menu_Load_Map()
{
}

/*
===========
called when the ok button has been clicked 
===========
*/
void Main_Menu_Load_Map::clicked(int id) {
   std::string filename=m_editbox->get_text();
     
   Map* m_map=m_parent->get_editor()->get_map();

   if(filename!="") {
   // Clean all the stuff up, so we can load
   m_parent->get_editor()->get_objects()->cleanup(m_parent->get_editor());
   g_anim.flush();
   g_gr->flush(0);

   m_map->cleanup();

   std::string realname="maps/";
   realname+=filename;
   realname+=WLMF_SUFFIX;
//   realname+=S2MF_SUFFIX; 
   Map_Loader* ml=m_map->get_correct_loader(realname.c_str());

   try {
      //log("[Map_Loader] Loading map '%s'\n", realname.c_str());
      ml->preload_map();
      ml->load_map_complete(m_parent->get_editor());
   }  catch(std::exception& exe) {
      // This really shoudn't fail since maps are already preloaded (in map preview)
      // and therefore valid, but if it does, a valid map must be displayed, therefore
      // we create an empty one from scratch
      m_map->cleanup();
      m_map->create_empty_map();
      m_parent->map_changed();

      std::string s="Map Loading Error!\n\nReason given:\n";
      s+=exe.what();
      UIModal_Message_Box* mbox= new UIModal_Message_Box(m_parent, "Load Map Error!!", s, UIModal_Message_Box::OK);
      mbox->run();
      delete mbox;
   }
   
   m_parent->get_editor()->postload();
   m_parent->get_editor()->load_graphics();
   
   // Tell the user interface that the map has changed
   m_parent->map_changed();
   
   delete ml;
   }
   delete this;
   
}


