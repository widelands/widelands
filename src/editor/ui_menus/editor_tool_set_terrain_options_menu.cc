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


#include "editor_set_both_terrain_tool.h"
#include "editor_tool_set_terrain_options_menu.h"
#include "editorinteractive.h"
#include "i18n.h"
#include "keycodes.h"
#include "map.h"
#include "rendertarget.h"
#include "ui_button.h"
#include "ui_panel.h"
#include "ui_textarea.h"
#include "ui_checkbox.h"
#include "wlapplication.h"
#include "world.h"

/*
=================================================

class Editor_Tool_Set_Terrain_Tool_Options_Menu

=================================================
*/

/*
===============
Editor_Tool_Set_Terrain_Tool_Options_Menu::Editor_Tool_Set_Terrain_Tool_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Set_Terrain_Tool_Options_Menu::Editor_Tool_Set_Terrain_Tool_Options_Menu(Editor_Interactive *parent, int index,
						Editor_Set_Both_Terrain_Tool* sbt, UIUniqueWindowRegistry *registry)
	: Editor_Tool_Options_Menu(parent, index, registry, _("Terrain Select"))
{
   m_sbt=sbt;

   const int space=5;
   const int xstart=5;
   const int ystart=25;
   const int yend=25;
   int nr_textures=get_parent()->get_map()->get_world()->get_nr_terrains();
   int textures_in_row=(int)(sqrt((float)nr_textures));
   if(textures_in_row*textures_in_row<nr_textures) { textures_in_row++; }

   set_inner_size((textures_in_row)*(TEXTURE_W+1+space)+xstart, (textures_in_row)*(TEXTURE_H+1+space)+ystart+yend);

   int ypos=ystart;
   int xpos=xstart;
   int cur_x=0;

   int check[] = {
      0,                   // "green"
      TERRAIN_DRY,         // "dry"
      TERRAIN_DRY|TERRAIN_MOUNTAIN,   // "mountain"
      TERRAIN_DRY|TERRAIN_UNPASSABLE, // "unpassable"
      TERRAIN_ACID|TERRAIN_DRY|TERRAIN_UNPASSABLE, // "dead" or "acid"
      TERRAIN_UNPASSABLE|TERRAIN_DRY|TERRAIN_WATER,
   };

   m_checkboxes.resize(nr_textures);

   for(int checkfor=0; checkfor<6; checkfor++) {
      int i=1;
      while(i<=nr_textures) {
         if(cur_x==textures_in_row) { cur_x=0; ypos+=TEXTURE_H+1+space; xpos=xstart; }
         // Get Terrain
         Terrain_Descr* ter=get_parent()->get_map()->get_world()->get_terrain(i-1);
         if(ter->get_is()!=check[checkfor]) { i++; continue; }

         // Create a surface for this
         int picw, pich;
         g_gr->get_picture_size(g_gr->get_picture( PicMod_Game,  g_gr->get_maptexture_picture(i) ), &picw, &pich);
         uint surface=g_gr->create_surface(picw,pich);

         // Get the rendertarget for this
         RenderTarget* target=g_gr->get_surface_renderer(surface);

         // firts, blit the terrain texture
         target->blit(0,0,g_gr->get_picture( PicMod_Game,  g_gr->get_maptexture_picture(i) ));

         int small_picw, small_pich;
         g_gr->get_picture_size(g_gr->get_picture( PicMod_Game,  "pics/terrain_water.png" ), &small_picw, &small_pich);

         int pic_x=1;
         int pic_y=pich-small_pich-1;

         // Check is green
         if(ter->get_is()==0) {
            target->blit(pic_x,pic_y,g_gr->get_picture( PicMod_Game,  "pics/terrain_green.png" ));
            pic_x+=small_picw+1;
         }
         else if(ter->get_is()&TERRAIN_WATER) {
            target->blit(pic_x, pic_y,g_gr->get_picture( PicMod_Game,  "pics/terrain_water.png" ));
            pic_x+=small_picw+1;
         }
         else if(ter->get_is()&TERRAIN_MOUNTAIN) {
            target->blit(pic_x, pic_y,g_gr->get_picture( PicMod_Game,  "pics/terrain_mountain.png" ));
            pic_x+=small_picw+1;
         }
         else if(ter->get_is()&TERRAIN_ACID) {
            target->blit(pic_x, pic_y,g_gr->get_picture( PicMod_Game,  "pics/terrain_dead.png" ));
            pic_x+=small_picw+1;
         }
         else if(ter->get_is()&TERRAIN_UNPASSABLE) {
            target->blit(pic_x, pic_y,g_gr->get_picture( PicMod_Game,  "pics/terrain_unpassable.png" ));
            pic_x+=small_picw+1;
         }
         else if(ter->get_is()&TERRAIN_DRY) {
            target->blit(pic_x, pic_y,g_gr->get_picture( PicMod_Game,  "pics/terrain_dry.png" ));
            pic_x+=small_picw+1;
         }

         // Save this surface, so we can free it later on
         m_surfaces.push_back(surface);

         UICheckbox* cb=new UICheckbox(this, xpos , ypos, surface);

         cb->set_size(TEXTURE_W+1, TEXTURE_H+1);
         cb->set_id(i-1);
         cb->set_state(m_sbt->is_enabled(i-1));
         cb->changedtoid.set(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::selected);

         m_checkboxes[i-1]=cb;
         xpos+=TEXTURE_W+1+space;
         ++cur_x;
         ++i;
      }
   }
   ypos+=TEXTURE_H+1+space+5;

   UITextarea* ta=new UITextarea(this, 0, 5, _("Choose Terrain Menu"), Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);


   std::string buf=_("Current: ");
   int j=m_sbt->get_nr_enabled();
   for(int i=0; j; i++) {
      if(m_sbt->is_enabled(i)) {
         buf+=get_parent()->get_map()->get_world()->get_terrain(i)->get_name();
         buf+=" ";
         --j;
      }
   }
   m_textarea=new UITextarea(this, 5, get_inner_h()-25, get_inner_w()-10, 20, buf, Align_Center);
}

/*
 * Cleanup
 */
Editor_Tool_Set_Terrain_Tool_Options_Menu::~Editor_Tool_Set_Terrain_Tool_Options_Menu()  {
   for(uint i=0; i<m_surfaces.size(); i++)
      g_gr->free_surface(m_surfaces[i]);
}

/* do nothing */
void Editor_Tool_Set_Terrain_Tool_Options_Menu::do_nothing(int n, bool t) {
}

/*
===========
Editor_Tool_Set_Terrain_Tool_Options_Menu::selected()

===========
*/
void Editor_Tool_Set_Terrain_Tool_Options_Menu::selected(int n, bool t) {
	bool multiselect = WLApplication::get()->get_key_state(KEY_LCTRL) | WLApplication::get()->get_key_state(KEY_RCTRL);
   if(t==false && (!multiselect || m_sbt->get_nr_enabled()==1)) { m_checkboxes[n]->set_state(true); return; }

   if(!multiselect) {
      int i=0;
      while(m_sbt->get_nr_enabled()) {
         m_sbt->enable(i++,false);
      }
      // Disable all checkboxes
      for(i=0; i<((int)m_checkboxes.size()); i++) {
         if(i==n) continue;
         m_checkboxes[i]->changedtoid.set(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::do_nothing);
         m_checkboxes[i]->set_state(false);
         m_checkboxes[i]->changedtoid.set(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::selected);
      }
   }

   m_sbt->enable(n,t);
   select_correct_tool();

   std::string buf=_("Current: ");
   int j=m_sbt->get_nr_enabled();
   for(int i=0; j; i++) {
      if(m_sbt->is_enabled(i)) {
         buf+=get_parent()->get_map()->get_world()->get_terrain(i)->get_name();
         buf+=" ";
         --j;
      }
   }

   m_textarea->set_text(buf.c_str());
   m_textarea->set_pos((get_inner_w()-m_textarea->get_w())/2, m_textarea->get_y());
}
