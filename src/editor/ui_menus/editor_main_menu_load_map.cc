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
#include "ui_listselect.h"
#include "ui_multilinetextarea.h"
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
	: UIWindow(parent, 0, 0, 500, 300, "Load Map")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Load Map", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;
   
   // listselect
   m_ls=new UIListselect(this, posx, posy, get_inner_w()/2-spacing, get_inner_h()-spacing-offsy-40);
   m_ls->selected.set(this, &Main_Menu_Load_Map::selected);

	// Fill it with the files: Widelands map files
	g_fs->FindFiles("maps", "*"WLMF_SUFFIX, &m_mapfiles);
	g_fs->FindFiles("maps", "*"S2MF_SUFFIX, &m_mapfiles);

   Map* map=new Map();
	for(filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
		const char *name = pname->c_str();
       
      Map_Loader* m_ml = map->get_correct_loader(name);
      try {
         m_ml->preload_map(true);
         m_ls->add_entry(map->get_name(), reinterpret_cast<void*>(const_cast<char*>(name)));
      } catch(wexception& ) {
         // we simply skip illegal entries
      }
	   delete m_ml;
			
   }
   m_ls->sort();
   delete map;

   // the descriptive areas
   // Name
   posx=get_inner_w()/2+spacing;
   posy+=20;
   new UITextarea(this, posx, posy, 150, 20, "Name: ", Align_CenterLeft);
   m_name=new UITextarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Author
   new UITextarea(this, posx, posy, 150, 20, "Author: ", Align_CenterLeft);
   m_author=new UITextarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Size
   new UITextarea(this, posx, posy, 70, 20, "Size: ", Align_CenterLeft);
   m_size=new UITextarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // World
   new UITextarea(this, posx, posy, 70, 20, "World: ", Align_CenterLeft);
   m_world=new UITextarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Players
   new UITextarea(this, posx, posy, 70, 20, "Players: ", Align_CenterLeft);
   m_nrplayers=new UITextarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;
   

   // Description
   new UITextarea(this, posx, posy, 70, 20, "Descr: ", Align_CenterLeft);
   m_descr=new UIMultiline_Textarea(this, posx+70, posy, get_inner_w()-posx-spacing-70, get_inner_h()-posy-spacing-40, "---", Align_CenterLeft);
 
   posx=5;
   posy=get_inner_h()-30;
   
   UIButton* but= new UIButton(this, get_inner_w()/2-spacing-80, posy, 80, 20, 0, 1);
   but->clickedid.set(this, &Main_Menu_Load_Map::clicked);
   but->set_title("OK");
   but= new UIButton(this, get_inner_w()/2+spacing, posy, 80, 20, 1, 0);
   but->clickedid.set(this, &Main_Menu_Load_Map::clicked);
   but->set_title("Cancel");

   center_to_parent();

   if(m_ls->get_nr_entries())
      m_ls->select(0);
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
   if(id==1) {
      std::string filename=static_cast<const char*>(m_ls->get_selection());

      Map* m_map=m_parent->get_editor()->get_map();

      if(filename!="") {
         // Clean all the stuff up, so we can load
         m_parent->get_editor()->get_objects()->cleanup(m_parent->get_editor());
         g_anim.flush();
         g_gr->flush(0);

         m_map->cleanup();

         Map_Loader* ml=m_map->get_correct_loader(filename.c_str());

         try {
            //log("[Map_Loader] Loading map '%s'\n", realname.c_str());
            ml->preload_map(true);

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
            int picid=g_gr->get_picture(PicMod_Game, text.c_str(), true);
            g_gr->get_picture_size(picid, &w, &h);
            m_parent->get_map()->get_overlay_manager()->register_overlay(fc,picid,8, Coords(w/2,STARTING_POS_HOTSPOT_Y));
         }

         /* Resources. we do not calculate default resources, therefore we do
          * not expect to meet them here. */ 
         uint x,y;
         for(y=0; y<m_map->get_height(); y++) {
            for(x=0; x<m_map->get_width(); x++) {
               Field *f=m_map->get_field(Coords(x,y));  
               int res=f->get_resources();
               int amount=f->get_resources_amount();
               std::string immname="";
               if(amount) 
                  immname = m_parent->get_editor()->get_map()->get_world()->get_resource(res)->get_editor_pic(amount);
               if(immname!="") {
                  int picid=g_gr->get_picture(PicMod_Game, immname.c_str(), true); 
                  m_parent->get_map()->get_overlay_manager()->register_overlay(Coords(x,y),picid,4);
               } 
            }
         }

         // Touch all triggers once, so that they do not get deleted even
         // when unreferenced
         for(i=0; i<m_map->get_number_of_triggers(); i++)
            m_map->reference_trigger(m_map->get_trigger(i));

         // Tell the interactive that the map is saved and all
         m_parent->set_need_save(false);

         delete ml;
      }
   } 
   die();
}

/*
 * called when a item is selected
 */
void Main_Menu_Load_Map::selected(int i) {
   const char* name=static_cast<const char*>(m_ls->get_selection());

   Map* map=new Map();
   Map_Loader* m_ml = map->get_correct_loader(name);
   m_ml->preload_map(true); // This has worked before, no problem
   delete m_ml;

   m_name->set_text(map->get_name());
   m_author->set_text(map->get_author());
   m_descr->set_text(map->get_description());
   m_world->set_text(map->get_world_name());
   
   char buf[200];
   sprintf(buf, "%i", map->get_nrplayers());
   m_nrplayers->set_text(buf);
   
   sprintf(buf, "%ix%i", map->get_width(), map->get_height());
   m_size->set_text(buf);
   
   delete map;
}
