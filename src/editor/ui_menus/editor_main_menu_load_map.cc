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
#include "editor_set_starting_pos_tool.h"
#include "overlay_manager.h"
#include "world.h"
#include "map_loader.h"

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

      ml->load_map_complete(m_parent->get_editor(), true);
   }  catch(std::exception& exe) {
      // This really shoudn't fail since maps are already preloaded (in map preview)
      // and therefore valid, but if it does, a valid map must be displayed, therefore
      // we create an empty one from scratch
      m_map->cleanup();
      m_map->create_empty_map();

      std::string s="Map Loading Error!\n\nReason given:\n";
      s+=exe.what();
      UIModal_Message_Box* mbox= new UIModal_Message_Box(m_parent, "Load Map Error!!", s, UIModal_Message_Box::OK);
      mbox->run();
      delete mbox;
   }
  
   m_parent->get_editor()->postload();
   m_parent->get_editor()->load_graphics();
     
   // Now update all the visualisations
   // Player positions
   std::string text;
   int i=0;
   for(i=1; i<=m_parent->get_map()->get_nrplayers(); i++) {
      text="pics/editor_player_";
      text+=static_cast<char>(((i)/10) + 0x30);
      text+=static_cast<char>(((i)%10) + 0x30);
      text+="_starting_pos.png";
      Coords fc=m_parent->get_map()->get_starting_pos(i);

      if(fc.x==-1 && fc.y==-1) continue;
      int w, h;
      int picid=g_gr->get_picture(PicMod_Game, text.c_str(), RGBColor(0,0,255));
      g_gr->get_picture_size(picid, &w, &h);
      m_parent->get_map()->get_overlay_manager()->register_overlay(fc,picid,8, Coords(w/2,STARTING_POS_HOTSPOT_Y));
   }

   /* Resources, we do not calculate default resources, therefore we do
    * not expect to meet them here. */ 
   uint x,y;
   for(y=0; y<m_map->get_height(); y++) {
      for(x=0; x<m_map->get_width(); x++) {
         Field *f=m_map->get_field(Coords(x,y));  
         int res=f->get_resources();
         int amount=f->get_resources_amount();
         std::string immname="";
         RGBColor clrkey;
         if(amount) 
            immname = m_parent->get_editor()->get_map()->get_world()->get_resource(res)->get_editor_pic(amount, &clrkey);
         if(immname!="") {
            int picid=g_gr->get_picture(PicMod_Game, immname.c_str(), clrkey); 
            m_parent->get_map()->get_overlay_manager()->register_overlay(Coords(x,y),picid,4);
         } 
      }
   }
  

   delete ml;
   }
   die();
}


