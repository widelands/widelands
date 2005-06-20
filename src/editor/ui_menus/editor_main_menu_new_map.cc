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
#include <stdio.h>
#include <vector>
#include "editor_main_menu_new_map.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "editorinteractive.h"
#include "map.h"
#include "world.h"
#include "error.h"
#include "editor.h"

/*
===============
Main_Menu_New_Map::Main_Menu_New_Map

Create all the buttons etc...
===============
*/
Main_Menu_New_Map::Main_Menu_New_Map(Editor_Interactive *parent)
	: UIWindow(parent, (parent->get_w()-140)/2, (parent->get_h()-150)/2, 140, 150, "New Map")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "New Map Options", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // UIButtons
   char buf[250];
   const int offsx=5;
   const int offsy=30;
   const int spacing=5;
   const int width=get_inner_w()-offsx*2;
   const int height=20;
   int posx=offsx;
   int posy=offsy;
   m_w=0; m_h=0;
   sprintf(buf, "Width: %i", MAP_DIMENSIONS[m_w]);
   m_width=new UITextarea(this, posx+spacing+20, posy+7, buf, Align_Left);
   UIButton* b = new UIButton(this, posx, posy, 20, 20, 1, 0);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   b = new UIButton(this, get_inner_w()-spacing-20, posy, 20, 20, 1, 1);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=20+spacing+spacing;

   sprintf(buf, "Height: %i", MAP_DIMENSIONS[m_h]);
   m_height=new UITextarea(this, posx+spacing+20, posy+7, buf, Align_Left);
   b = new UIButton(this, posx, posy, 20, 20, 1, 2);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   b = new UIButton(this, get_inner_w()-spacing-20, posy, 20, 20, 1, 3);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=20+spacing+spacing;

   // get all worlds
   m_worlds=new std::vector<std::string>;
   World::get_all_worlds(m_worlds);

   assert(m_worlds->size());
   m_currentworld=0;
   m_world=new UIButton(this, posx, posy, width, height, 1, 4);
   m_world->set_title((*m_worlds)[m_currentworld].c_str());
   m_world->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=height+spacing+spacing+spacing;

   b=new UIButton(this, posx, posy, width, height, 0, 5);
   b->set_title("Create Map");
   b->clickedid.set(this, &Main_Menu_New_Map::button_clicked);
   posy+=height+spacing;


   }


/*
===========
Main_Menu_New_Map UIButton functions

called, when button get clicked
===========
*/
void Main_Menu_New_Map::button_clicked(int n) {
   switch(n) {
      case 0: m_w++; break;
      case 1: m_w--; break;
      case 2: m_h++; break;
      case 3: m_h--; break;
      case 4:
              ++m_currentworld;
              if(m_currentworld==m_worlds->size()) m_currentworld=0;
              m_world->set_title((*m_worlds)[m_currentworld].c_str());
              break;
      case 5:
              {
                 Map* m_map=m_parent->get_egbase()->get_map();
                 // Clean all the stuff up, so we can load
                 m_parent->get_editor()->cleanup_for_load(true, false);

                 m_map->create_empty_map(MAP_DIMENSIONS[m_w],MAP_DIMENSIONS[m_h],(*m_worlds)[m_currentworld]);

                 // Postload the world which provides all the immovables found on a map
                 m_map->get_world()->postload(m_parent->get_editor());

                 m_parent->get_editor()->postload();
                 m_parent->get_editor()->load_graphics();

                 m_map->recalc_whole_map();

                 m_parent->set_need_save(true);

                 die();
                 return ;
              }
   }

   char buf[200];
   if(m_w<0) m_w=0;
   if(m_w>=NUMBER_OF_MAP_DIMENSIONS) m_w=NUMBER_OF_MAP_DIMENSIONS-1;
   if(m_h<0) m_h=0;
   if(m_h>=NUMBER_OF_MAP_DIMENSIONS) m_h=NUMBER_OF_MAP_DIMENSIONS-1;
   sprintf(buf, "Width: %i", MAP_DIMENSIONS[m_w]);
   m_width->set_text(buf);
   sprintf(buf, "Height: %i", MAP_DIMENSIONS[m_h]);
   m_height->set_text(buf);
}

/*
===============
Main_Menu_New_Map::~Main_Menu_New_Map

Unregister from the registry pointer
===============
*/
Main_Menu_New_Map::~Main_Menu_New_Map()
{
   delete m_worlds;
}


