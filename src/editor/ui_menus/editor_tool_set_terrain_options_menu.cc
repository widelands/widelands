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


#include "editor_tool_set_terrain_options_menu.h"
#include "editorinteractive.h"
#include "ui_panel.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "map.h"
#include "world.h"
#include "ui_checkbox.h"
#include "editor_set_both_terrain_tool.h"
#include "keycodes.h"

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
Editor_Tool_Set_Terrain_Tool_Options_Menu::Editor_Tool_Set_Terrain_Tool_Options_Menu(Editor_Interactive *parent,
						Editor_Set_Both_Terrain_Tool* sbt, UIUniqueWindowRegistry *registry)
	: Editor_Tool_Options_Menu(parent, registry, "Terrain Select")
{
   m_sbt=sbt;
   m_multiselect=false;

   const int space=5;
   const int xstart=5;
   const int ystart=25;
   const int yend=25;
   int nr_textures=get_parent()->get_map()->get_world()->get_nr_terrains();
   int textures_in_row=(int)(sqrt((float)nr_textures));
   if(textures_in_row*textures_in_row<nr_textures) { textures_in_row++; }
   int i=1;

   set_inner_size((textures_in_row)*(TEXTURE_W+1+space)+xstart, (textures_in_row)*(TEXTURE_H+1+space)+ystart+yend);

   int ypos=ystart;
   int xpos=xstart;
   int cur_x=0;
   while(i<=nr_textures) {
      if(cur_x==textures_in_row) { cur_x=0; ypos+=TEXTURE_H+1+space; xpos=xstart; }

      UICheckbox* cb=new UICheckbox(this, xpos , ypos, g_gr->get_picture(PicMod_Game, g_gr->get_maptexture_picture(i), false));

      cb->set_size(TEXTURE_W+1, TEXTURE_H+1);
      cb->set_id(i-1);
      cb->set_state(m_sbt->is_enabled(i-1));
      cb->changedtoid.set(this, &Editor_Tool_Set_Terrain_Tool_Options_Menu::selected);

      m_checkboxes.push_back(cb);
      xpos+=TEXTURE_W+1+space;
      ++cur_x;
      ++i;
   }
   ypos+=TEXTURE_H+1+space+5;

   UITextarea* ta=new UITextarea(this, 0, 5, "Choose Terrain Menu", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);


   std::string buf="Current: ";
   int j=m_sbt->get_nr_enabled();
   for(int i=0; j; i++) {
      if(m_sbt->is_enabled(i)) {
         buf+=get_parent()->get_map()->get_world()->get_terrain(i)->get_name();
         buf+=" ";
         --j;
      }
   }

   set_can_focus(true);
   focus();

   m_textarea=new UITextarea(this, 5, ypos, buf);
   m_textarea->set_pos((get_inner_w()-m_textarea->get_w())/2, ypos);
}
      
/*
 * Cleanup
 */
Editor_Tool_Set_Terrain_Tool_Options_Menu::~Editor_Tool_Set_Terrain_Tool_Options_Menu()  {
   set_can_focus(false);
}

/*
 * handle key. When STRG is pressed, set multiselect to on
 */
bool Editor_Tool_Set_Terrain_Tool_Options_Menu::handle_key(bool down, int code, char c) {
   if(code==KEY_LCTRL || code==KEY_RCTRL) m_multiselect=down;
   return false;
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
   if(t==false && (!m_multiselect || m_sbt->get_nr_enabled()==1)) { m_checkboxes[n]->set_state(true); return; }

   if(!m_multiselect) {
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

   std::string buf="Current: ";
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
