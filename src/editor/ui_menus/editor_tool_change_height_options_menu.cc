/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
#include "editor_tool_change_height_options_menu.h"
#include "editorinteractive.h"
#include "graphic.h"
#include "i18n.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "editor_increase_height_tool.h"
#include "editor_set_height_tool.h"

/*
=================================================

class Editor_Tool_Change_Height_Options_Menu

=================================================
*/

/*
===========
Editor_Tool_Change_Height_Options_Menu::Editor_Tool_Change_Height_Options_Menu()

constructor
===========
*/
Editor_Tool_Change_Height_Options_Menu::Editor_Tool_Change_Height_Options_Menu(Editor_Interactive* parent, int index,
      Editor_Increase_Height_Tool* iht, UI::UniqueWindow::Registry* registry) :
Editor_Tool_Options_Menu
(parent, index, registry, _("Height Tools Options").c_str())
{

   m_iht=iht;
   m_dht=iht->get_dht();
   m_sht=iht->get_sht();

   int offsx=5;
   int offsy=30;
   int spacing=5;
   int height=20;
   int width=20;
   int posx=offsx;
   int posy=offsy;

   set_inner_size(135, 135);
   UI::Textarea* ta=new UI::Textarea(this, 0, 0, _("Height Tool Options"), Align_Left);
	ta->set_pos(Point((get_inner_w() - ta->get_w()) / 2, 5));

   ta=new UI::Textarea(this, 0, 0, _("In/Decrease Value"), Align_Left);
	ta->set_pos(Point((get_inner_w() - ta->get_w()) / 2, posy + 5));
   posy+=spacing+width;

	new UI::IDButton<Editor_Tool_Change_Height_Options_Menu, const Uint8>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Editor_Tool_Change_Height_Options_Menu::clicked, this, 0);

	new UI::IDButton<Editor_Tool_Change_Height_Options_Menu, const Uint8>
		(this,
		 get_inner_w() - spacing - width, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Editor_Tool_Change_Height_Options_Menu::clicked, this, 1);

	m_increase=new UI::Textarea(this, 0, 0, "5", Align_Left);
	m_increase->set_pos
		(Point((get_inner_w() - m_increase->get_w()) / 2, posy + 5));
   posy+=width+spacing+spacing;

   ta=new UI::Textarea(this, 0, 0, _("Set Value"), Align_Left);
	ta->set_pos(Point((get_inner_w() - ta->get_w()) / 2, posy + 5));
   posy+=width+spacing;
	new UI::IDButton<Editor_Tool_Change_Height_Options_Menu, const Uint8>
		(this,
		 posx, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Editor_Tool_Change_Height_Options_Menu::clicked, this, 2);
	new UI::IDButton<Editor_Tool_Change_Height_Options_Menu, const Uint8>
		(this,
		 get_inner_w() - spacing - width, posy, width, height,
		 1,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Editor_Tool_Change_Height_Options_Menu::clicked, this, 3);
   m_set=new UI::Textarea(this, 0, 0, "5", Align_Left);
	m_set->set_pos(Point((get_inner_w() - m_set->get_w()) / 2, posy + 5));
   posy+=width+spacing;

   update();
}

/*
===========
Editor_Tool_Change_Height_Options_Menu::clicked()

called when one button is clicked
===========
*/
void Editor_Tool_Change_Height_Options_Menu::clicked(const Uint8 n) {
   int increase=m_iht->get_changed_by();
   int set=m_sht->get_set_to();

   assert(m_iht->get_changed_by()==m_dht->get_changed_by());

   if(n==0) {
      ++increase;
      if(increase>MAX_FIELD_HEIGHT_DIFF) increase=MAX_FIELD_HEIGHT_DIFF;
   } else if(n==1) {
      --increase;
      if(increase<0) increase=0;
   }
   m_iht->set_changed_by(increase);
   m_dht->set_changed_by(increase);

   if(n==2) {
      ++set;
      if(set>MAX_FIELD_HEIGHT) set=MAX_FIELD_HEIGHT;
   } else if(n==3) {
      --set;
      if(set<0) set=0;
   }
   m_sht->set_set_to(set);

   select_correct_tool();
   update();
}

/*
===========
Editor_Tool_Change_Height_Options_Menu::update()

Update all the textareas, so that they represent the correct values
===========
*/
void Editor_Tool_Change_Height_Options_Menu::update(void) {
   char buf[250];
   sprintf(buf, "%i", m_iht->get_changed_by());
   m_increase->set_text(buf);
   sprintf(buf, "%i", m_sht->get_set_to());
   m_set->set_text(buf);
}
