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

#include "editorinteractive.h"
#include "editor_place_bob_tool.h"
#include "editor_tool_place_bob_options_menu.h"
#include "keycodes.h"
#include "map.h"
#include "system.h"
#include "ui_box.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_tabpanel.h"
#include "ui_textarea.h"
#include "wlapplication.h"
#include "world.h"

/*
=================================================

class Editor_Tool_Place_Bob_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Place_Bob_Options_Menu::Editor_Tool_Place_Bob_Options_Menu

constructor
===========
*/
Editor_Tool_Place_Bob_Options_Menu::Editor_Tool_Place_Bob_Options_Menu(Editor_Interactive* parent, int index,
		Editor_Place_Bob_Tool* pit, UIUniqueWindowRegistry* registry) :
   Editor_Tool_Options_Menu(parent, index, registry, _("Bobs Menu")) {
   const int max_items_in_tab=24;
   const int min_items_in_tab=12;

   m_pit=pit;

   const int space=5;
   const int xstart=5;
   const int ystart=15;
   const int yend=15;
   int nr_bobs=get_parent()->get_map()->get_world()->get_nr_bobs();
   int bobs_in_row=(int)(sqrt((float)nr_bobs));
   if(bobs_in_row*bobs_in_row<nr_bobs) { bobs_in_row++; }
   if(bobs_in_row>max_items_in_tab) bobs_in_row=max_items_in_tab;
   if(bobs_in_row<min_items_in_tab) bobs_in_row=min_items_in_tab;

   UITab_Panel* m_tabpanel=new UITab_Panel(this, 0, 0, 1);
   m_tabpanel->set_snapparent(true);
   UIBox* box=new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);
   m_tabpanel->add(g_gr->get_picture( PicMod_Game,  "pics/menu_tab_buildbig.png"), box );


   int width=0;
   int height=0;
   for(int j=0; j<nr_bobs; j++) {
      int w,h;
		Bob_Descr* descr = get_parent()->get_map()->get_world()->get_bob_descr(j);
      g_gr->get_picture_size(
            g_gr->get_picture(PicMod_Game, descr->get_picture()), &w, &h);
      if(w>width) width=w;
      if(h>height) height=h;
   }

   box->set_inner_size((bobs_in_row)*(width+1+space)+xstart, (bobs_in_row)*(height+1+space)+ystart+yend);

   int ypos=ystart;
   int xpos=xstart;
   int cur_x=0;
   int i=0;
   while(i<nr_bobs) {
      if(cur_x==bobs_in_row) {
         cur_x=0;
         ypos=ystart;
         xpos=xstart;
         box->resize();
         box=new UIBox(m_tabpanel, 0, 0, UIBox::Horizontal);
         m_tabpanel->add(g_gr->get_picture( PicMod_Game,  "pics/menu_tab_buildbig.png"), box );
      }

		Bob_Descr* descr = get_parent()->get_map()->get_world()->get_bob_descr(i);
      UICheckbox* cb= new UICheckbox(box, xpos, ypos,
            g_gr->get_picture( PicMod_Game,  descr->get_picture() ));

      cb->set_size(width, height);
      cb->set_id(i);
      cb->set_state(m_pit->is_enabled(i));
      cb->changedtoid.set(this, &Editor_Tool_Place_Bob_Options_Menu::clicked);
      m_checkboxes.push_back(cb);
      box->add(cb, Align_Left);
      box->add_space(space);
      xpos+=width+1+space;
      ++cur_x;
      ++i;
   }
   ypos+=height+1+space+5;

   m_tabpanel->activate(0);
   box->resize();
   m_tabpanel->resize();
}

/*
 * Cleanup
 */
Editor_Tool_Place_Bob_Options_Menu::~Editor_Tool_Place_Bob_Options_Menu(void) {
}

/*
===========
   void Editor_Tool_Place_Bob_Options_Menu::clicked()

this is called when one of the state boxes is toggled
===========
*/
void Editor_Tool_Place_Bob_Options_Menu::clicked(int n, bool t) {
   bool multiselect = Sys_GetKeyState(KEY_LCTRL) | Sys_GetKeyState(KEY_RCTRL);
   if(t==false && (!multiselect || m_pit->get_nr_enabled()==1)) { m_checkboxes[n]->set_state(true); return; }

   if(!multiselect) {
      int i=0;
      while(m_pit->get_nr_enabled()) {
         m_pit->enable(i++,false);
      }
      // Disable all checkboxes
      for(i=0; i<((int)m_checkboxes.size()); i++) {
         if(i==n) continue;
         m_checkboxes[i]->changedtoid.set(this, &Editor_Tool_Place_Bob_Options_Menu::do_nothing);
         m_checkboxes[i]->set_state(false);
         m_checkboxes[i]->changedtoid.set(this, &Editor_Tool_Place_Bob_Options_Menu::clicked);
      }
   }

   m_pit->enable(n,t);
   select_correct_tool();
}

/* do nothing */
void Editor_Tool_Place_Bob_Options_Menu::do_nothing(int n, bool t) {
}
