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

#include <stdio.h>
#include "editor_tool_change_resources_options_menu.h"
#include "editorinteractive.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_radiobutton.h"
#include "editor_increase_resources_tool.h"
#include "editor_set_resources_tool.h"
#include "map.h"
#include "world.h"
#include "graphic.h"
#include "error.h"
#include "overlay_manager.h"

/*
=================================================

class Editor_Tool_Change_Resources_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Change_Resources_Options_Menu::Editor_Tool_Change_Resources_Options_Menu()

constructor
===========
*/
Editor_Tool_Change_Resources_Options_Menu::Editor_Tool_Change_Resources_Options_Menu(Editor_Interactive* parent, int index,
      Editor_Increase_Resources_Tool* iht, UIUniqueWindowRegistry* registry) :
   Editor_Tool_Options_Menu(parent, index, registry, "Resources Tools Options") {

   m_parent=parent;
   m_irt=iht;
   m_drt=iht->get_dht();
   m_srt=iht->get_sht();


   // Here we go
   const int space=5;
   const int xstart=5;
   const int ystart=15;
   const int yend=15;
   int nr_resources=get_parent()->get_map()->get_world()->get_nr_resources();
   int resources_in_row=(int)(sqrt((float)nr_resources));
   if(resources_in_row*resources_in_row<nr_resources) { resources_in_row++; }
   int i=1;

   int w=0, h=0;
   int width=0, height=0;
   for(i=0; i<nr_resources; i++) {
      Resource_Descr* res=parent->get_map()->get_world()->get_resource(i);
      std::string editor_pic=res->get_editor_pic(100000);
      int picid=g_gr->get_picture(PicMod_Game, editor_pic.c_str(), true);
      g_gr->get_picture_size(picid, &w,&h);
      if(w>width) width=w;
      if(h>height) height=h;
   }

   int innsize= (resources_in_row)*(width+1+space)+xstart > 135 ? (resources_in_row)*(width+1+space)+xstart : 135;
   set_inner_size(innsize, (resources_in_row)*(height+1+space)+ystart+yend+80);

   int offsx=5;
   int offsy=30;
   int spacing=5;
   int resources=20;
   int posx=offsx;
   int posy=offsy;
   int button_width=20;
   UITextarea* ta=new UITextarea(this, 0, 0, "Resources Tool Options", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   ta=new UITextarea(this, 0, 0, "In/Decrease Value", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=spacing+button_width;
   UIButton* b=new UIButton(this, posx, posy, button_width, resources, 1, 0);
   b->clickedid.set(this, &Editor_Tool_Change_Resources_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", true));
   b=new UIButton(this, get_inner_w()-spacing-button_width, posy, button_width, resources, 1, 1);
   b->clickedid.set(this, &Editor_Tool_Change_Resources_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", true));
   m_increase=new UITextarea(this, 0, 0, "5", Align_Left);
   m_increase->set_pos((get_inner_w()-m_increase->get_w())/2, posy+5);
   posy+=button_width+spacing+spacing;

   ta=new UITextarea(this, 0, 0, "Set Value", Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=button_width+spacing;
   b=new UIButton(this, posx, posy, button_width, resources, 1, 2);
   b->clickedid.set(this, &Editor_Tool_Change_Resources_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png", true));
   b=new UIButton(this, get_inner_w()-spacing-button_width, posy, button_width, resources, 1, 3);
   b->clickedid.set(this, &Editor_Tool_Change_Resources_Options_Menu::clicked);
   b->set_pic(g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png", true));
   m_set=new UITextarea(this, 0, 0, "5", Align_Left);
   m_set->set_pos((get_inner_w()-m_set->get_w())/2, posy+5);
   posy+=button_width+spacing;

   m_cur_selection=new UITextarea(this, 0, 0, "Current Selection", Align_Left);

   // Now the available resources
   posx=xstart;
   int cur_x=0;
   i=0;
   m_radiogroup=new UIRadiogroup();
   m_radiogroup->changed.set(this, &Editor_Tool_Change_Resources_Options_Menu::selected);
   m_radiogroup->clicked.set(this, &Editor_Tool_Change_Resources_Options_Menu::selected);

   while(i<nr_resources) {
      Resource_Descr* res=parent->get_map()->get_world()->get_resource(i);
      if(cur_x==resources_in_row) { cur_x=0; posy+=height+1+space; posx=xstart; }

      std::string editor_pic=res->get_editor_pic(100000);
      int picid=g_gr->get_picture(PicMod_Game, editor_pic.c_str(),true);


      m_radiogroup->add_button(this, posx,posy,picid);

      posx+=width+1+space;
      ++cur_x;
      ++i;
   }
   posy+=height+1+space+5;

   m_radiogroup->set_state(m_irt->get_cur_res());

   update();
}

/*
===========
Editor_Tool_Change_Resources_Options_Menu::clicked()

called when one button is clicked
===========
*/
void Editor_Tool_Change_Resources_Options_Menu::clicked(int n) {
   int increase=m_irt->get_changed_by();
   int set=m_srt->get_set_to();

   assert(m_irt->get_changed_by()==m_drt->get_changed_by());

   if(n==0) {
      ++increase;
   } else if(n==1) {
      --increase;
      if(increase<0) increase=0;
   }
   m_irt->set_changed_by(increase);
   m_drt->set_changed_by(increase);

   if(n==2) {
      ++set;
   } else if(n==3) {
      --set;
      if(set<0) set=0;
   }
   m_srt->set_set_to(set);
   select_correct_tool();

   update();
}

/*
 * called when a resource has been selected
 */
void Editor_Tool_Change_Resources_Options_Menu::selected(void) {
   int n=m_radiogroup->get_state();

   m_srt->set_cur_res(n);
   m_irt->set_cur_res(n);
   m_drt->set_cur_res(n);

   m_parent->get_map()->get_overlay_manager()->register_overlay_callback_function(&Editor_Change_Resource_Tool_Callback, static_cast<void*>(m_parent->get_map()), n);
   m_parent->get_map()->recalc_whole_map();
   select_correct_tool();

   update();
}

/*
===========
Editor_Tool_Change_Resources_Options_Menu::update()

Update all the textareas, so that they represent the correct values
===========
*/
void Editor_Tool_Change_Resources_Options_Menu::update(void) {
   char buf[250];
   sprintf(buf, "%i", m_irt->get_changed_by());
   m_increase->set_text(buf);
   sprintf(buf, "%i", m_srt->get_set_to());
   m_set->set_text(buf);

   int cursel=m_srt->get_cur_res();
   if(cursel)
      m_cur_selection->set_text(m_parent->get_map()->get_world()->get_resource(m_srt->get_cur_res())->get_name());
   else
      m_cur_selection->set_text("");
   m_cur_selection->set_pos((get_inner_w()-m_cur_selection->get_w())/2, get_inner_h()-20);
}


