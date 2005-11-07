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
#include "editor_tool_noise_height_options_menu.h"
#include "editorinteractive.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_noise_height_tool.h"

/*
=================================================

class Editor_Tool_Noise_Height_Options_Menu

=================================================
*/

/*
===============
Editor_Tool_Noise_Height_Options_Menu::Editor_Tool_Noise_Height_Options_Menu

Create all the buttons etc...
===============
*/
Editor_Tool_Noise_Height_Options_Menu::Editor_Tool_Noise_Height_Options_Menu(Editor_Interactive *parent, int index,
				Editor_Noise_Height_Tool* nht, UIUniqueWindowRegistry* registry)
   : Editor_Tool_Options_Menu(parent, index, registry, _("Noise Height Options"))
{
   char buf[250];
   sprintf(buf, "%s: %i", _("Minimum"), 10);
   m_textarea_lower=new UITextarea(this, 10, 25, buf);
   sprintf(buf, "%s: %i", _("Maximum"), 10);
   m_textarea_upper=new UITextarea(this, 105, 25, buf);

   UIButton* b = new UIButton(this, 30, 40, 20, 20, 0, 0);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b=new UIButton(this, 50, 40, 20, 20, 0, 1);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b=new UIButton(this, 130, 40, 20, 20, 0, 2);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_up.png" ));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b=new UIButton(this, 150, 40, 20, 20, 0, 3);
   b->set_pic(g_gr->get_picture( PicMod_UI,  "pics/scrollbar_down.png" ));
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);

   set_inner_size(200, 115);

   UITextarea* ta=new UITextarea(this, 3, 5, _("Noise Height Tool Options"), Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, 5);

   int posy=70;
   int width=20;
   int spacing=5;
   int height=20;
   ta=new UITextarea(this, 0, 0, _("Set Value"), Align_Left);
   ta->set_pos((get_inner_w()-ta->get_w())/2, posy+5);
   posy+=20;

   m_set=new UITextarea(this, 0, 0, "99", Align_Left);
   m_set->set_pos((get_inner_w()-m_set->get_w())/2, posy+5);
   b=new UIButton(this, m_set->get_x()-width-spacing, posy, width, height, 1, 4);
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_up.png" ));
   b=new UIButton(this, m_set->get_x()+m_set->get_w()+spacing, posy, width, height, 1, 5);
   b->clickedid.set(this, &Editor_Tool_Noise_Height_Options_Menu::button_clicked);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/scrollbar_down.png" ));

   m_nht=nht;

   update();
}

/*
===============
Editor_Tool_Noise_Height_Options_Menu::update()

Update all textareas
===============
*/
void Editor_Tool_Noise_Height_Options_Menu::update(void) {
   char buf[200];
   int up, low;
   m_nht->get_values(&low, &up);
   sprintf(buf, "%s: %i", _("Minimum"), low);
   m_textarea_lower->set_text(buf);
   sprintf(buf, "%s: %i", _("Maximum"), up);
   m_textarea_upper->set_text(buf);

   sprintf(buf, "%i", m_nht->get_sht()->get_set_to());
   m_set->set_text(buf);
   
   select_correct_tool();

}

/*
==============
Editor_Tool_Noise_Height_Options_Menu::button_clicked()

called when a button is clicked
==============
*/
void Editor_Tool_Noise_Height_Options_Menu::button_clicked(int n) {
   int up, low, set;
   set=m_nht->get_sht()->get_set_to();
   m_nht->get_values(&low, &up);
   switch(n) {
      case 0: ++low; break;
      case 1: --low; break;
      case 2: ++up; break;
      case 3: --up; break;
      case 4: ++set; break;
      case 5: --set; break;
   }
   if(low>MAX_FIELD_HEIGHT) low=MAX_FIELD_HEIGHT;
   if(low<0) low=0;
   if(up>MAX_FIELD_HEIGHT) up=MAX_FIELD_HEIGHT;
   if(up<0) up=0;
   if(set>MAX_FIELD_HEIGHT) set=MAX_FIELD_HEIGHT;
   if(set<0) set=0;

   m_nht->set_values(low, up);
   m_nht->get_sht()->set_set_to(set);

   update();
}


