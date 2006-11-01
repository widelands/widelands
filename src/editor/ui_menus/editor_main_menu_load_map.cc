/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include <stdio.h>
#include <string>
#include "graphic.h"
#include "i18n.h"
#include "layeredfilesystem.h"
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
	: UI::Window(parent, 0, 0, 500, 300, _("Load Map").c_str())
{
   m_parent=parent;

   // Caption
   UI::Textarea* tt=new UI::Textarea(this, 0, 0, _("Load Map"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // listselect
   m_ls=new UI::Listselect<const char * const>(this, posx, posy, get_inner_w()/2-spacing, get_inner_h()-spacing-offsy-40);
   m_ls->selected.set(this, &Main_Menu_Load_Map::selected);
   m_ls->double_clicked.set(this, &Main_Menu_Load_Map::double_clicked);

   // the descriptive areas
   // Name
   posx=get_inner_w()/2+spacing;
   posy+=20;
   new UI::Textarea(this, posx, posy, 150, 20, _("Name: "), Align_CenterLeft);
   m_name=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Author
   new UI::Textarea(this, posx, posy, 150, 20, _("Author: "), Align_CenterLeft);
   m_author=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Size
   new UI::Textarea(this, posx, posy, 70, 20, _("Size: "), Align_CenterLeft);
   m_size=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // World
   new UI::Textarea(this, posx, posy, 70, 20, _("World: "), Align_CenterLeft);
   m_world=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Players
   new UI::Textarea(this, posx, posy, 70, 20, _("Players: "), Align_CenterLeft);
   m_nrplayers=new UI::Textarea(this, posx+70, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;


   // Description
   new UI::Textarea(this, posx, posy, 70, 20, _("Descr: "), Align_CenterLeft);
   m_descr=new UI::Multiline_Textarea(this, posx+70, posy, get_inner_w()-posx-spacing-70, get_inner_h()-posy-spacing-40, "---", Align_CenterLeft);

   posx=5;
   posy=get_inner_h()-30;

   UI::Button* but= new UI::Button(this, get_inner_w()/2-spacing-80, posy, 80, 20, 0, 1);
   but->clickedid.set(this, &Main_Menu_Load_Map::clicked);
   but->set_title(_("OK").c_str());
   m_ok_btn=but;
   but->set_enabled(false);
   but= new UI::Button(this, get_inner_w()/2+spacing, posy, 80, 20, 1, 0);
   but->clickedid.set(this, &Main_Menu_Load_Map::clicked);
   but->set_title(_("Cancel").c_str());


   m_basedir="maps";
   m_curdir="maps";

   fill_list();

   center_to_parent();
   move_to_top();
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
      // Ok
		std::string filename(m_ls->get_selection());

      if(g_fs->IsDirectory(filename.c_str()) && !Widelands_Map_Loader::is_widelands_map( filename)) {
	      m_curdir=g_fs->FS_CanonicalizeName(filename);
         m_ls->clear();
         m_mapfiles.clear();
         fill_list();
      } else {
         load_map(filename);
         die();
      }
   } else {
      // Cancel
      die();
   }
}

/*
 * called when a item is selected
 */
void Main_Menu_Load_Map::selected(uint) {
	const char * const name = m_ls->get_selection();

   m_ok_btn->set_enabled(true);

   if(!g_fs->IsDirectory(name) || Widelands_Map_Loader::is_widelands_map( name )) {
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
   } else {
      m_name->set_text("");
      m_author->set_text("");
      m_descr->set_text("");
      m_world->set_text("");
      m_nrplayers->set_text("");
      m_size->set_text("");
   }
}

/*
 * An Item has been doubleclicked
 */
void Main_Menu_Load_Map::double_clicked(uint) {clicked(1);}

/*
 * fill the file list
 */
void Main_Menu_Load_Map::fill_list(void) {
   // Fill it with all files we find.
   g_fs->FindFiles(m_curdir, "*", &m_mapfiles, 1);

   // First, we add all directorys
   // We manually add the parent directory
   if(m_curdir!=m_basedir) {
	   m_parentdir=g_fs->FS_CanonicalizeName(m_curdir+"/..");
      m_ls->add_entry("<parent>", m_parentdir.c_str(), false, g_gr->get_picture( PicMod_Game,  "pics/ls_dir.png" ));
   }

   for(filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
      const char *name = pname->c_str();
      if(!strcmp(FileSystem::FS_Filename(name),".")) continue;
      if(!strcmp(FileSystem::FS_Filename(name),"..")) continue; // Upsy, appeared again. ignore
      if(!strcmp(FileSystem::FS_Filename(name),"CVS")) continue; // HACK: we skip CVS dir (which is in normal checkout present) for aesthetic reasons
      if(!g_fs->IsDirectory(name)) continue;
      if(Widelands_Map_Loader::is_widelands_map( name )) continue;

      m_ls->add_entry(FileSystem::FS_Filename(name), name, false, g_gr->get_picture( PicMod_Game,  "pics/ls_dir.png" ));
   }

   Map* map=new Map();

   for(filenameset_t::iterator pname = m_mapfiles.begin(); pname != m_mapfiles.end(); pname++) {
      const char *name = pname->c_str();

      Map_Loader* m_ml = map->get_correct_loader(name);
      if(!m_ml) continue;

      try {
         m_ml->preload_map(true);
         std::string pic="";
         switch(m_ml->get_type()) {
            case Map_Loader::WLML: pic="pics/ls_wlmap.png"; break;
            case Map_Loader::S2ML: pic="pics/ls_s2map.png"; break;
         }
	 m_ls->add_entry(FileSystem::FS_Filename(name), name, false, g_gr->get_picture( PicMod_Game,  pic.c_str() ));
      } catch(_wexception& ) {
         // we simply skip illegal entries
      }
      delete m_ml;

   }
   delete map;

   if(m_ls->get_nr_entries())
      m_ls->select(0);
}

/*
 * Load map complete
 */
void Main_Menu_Load_Map::load_map(std::string filename) {
   Map* m_map=m_parent->get_editor()->get_map();

   if(filename!="") {
      m_parent->get_editor()->cleanup_for_load(true, false);

      Map_Loader* ml = m_map->get_correct_loader(filename.c_str());

//      try {
         //log("[Map_Loader] Loading map '%s'\n", realname.c_str());
         ml->preload_map(true);

         ml->load_map_complete(m_parent->get_editor(), true);
/*      }  catch(std::exception& exe) {
         // This really shoudn't fail since maps are already preloaded (in map preview)
         // and therefore valid, but if it does, a valid map must be displayed, therefore
         // we create an empty one from scratch
         m_parent->get_editor()->cleanup_for_load(true, false);
         m_map->create_empty_map();

         std::string s="Map Loading Error!\n\nReason given:\n";
         s+=exe.what();
         UI::Modal_Message_Box* mbox= new UI::Modal_Message_Box(m_parent, "Load Map Error!!", s, UI::Modal_Message_Box::OK);
         mbox->run();
         delete mbox;
      }
*/
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

         if (fc.is_invalid()) continue;
			uint w, h;
			const uint picid=g_gr->get_picture(PicMod_Game,  text.c_str());
			g_gr->get_picture_size(picid, w, h);
         // only register, when theres no building there
         BaseImmovable* imm = m_parent->get_map()->get_field(fc)->get_immovable();
         if(imm && imm->get_type() == Map_Object::BUILDING) continue;

         // no building, place overlay
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
               int picid=g_gr->get_picture( PicMod_Game,  immname.c_str() );
               m_parent->get_map()->get_overlay_manager()->register_overlay(Coords(x,y),picid,4);
            }
         }
      }

      // Tell the interactive that the map is saved and all
      m_parent->set_need_save(false);

      // Redraw everything
      m_parent->need_complete_redraw();

      delete ml;
   }
}
