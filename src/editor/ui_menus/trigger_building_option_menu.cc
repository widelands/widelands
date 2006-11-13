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
#include "i18n.h"
#include "trigger_building_option_menu.h"
#include "trigger_building.h"
#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "editorinteractive.h"
#include "error.h"
#include "map.h"
#include "graphic.h"
#include "editor.h"
#include "player.h"
#include "tribe.h"
#include "util.h"

Trigger_Building_Option_Menu::Trigger_Building_Option_Menu(Editor_Interactive* parent, Trigger_Building* trigger) :
UI::Window(parent, 0, 0, 180, 280, _("Building Trigger Options").c_str()),
m_trigger (trigger),
m_parent  (parent)
{
   Coords pt=trigger->get_coords();
   m_x=pt.x;
   m_y=pt.y;
   m_area=m_trigger->get_area();
   m_player=m_trigger->get_player();
   m_count=m_trigger->get_building_count();
   m_building=-1;
   const int offsx=5;
   const int offsy=25;
   int spacing=5;
   int posx=offsx;
   int posy=offsy;

   if(m_player<1) m_player=1;

   // Fill the building infos
   Tribe_Descr* tribe = m_parent->get_editor()->get_tribe(m_parent->get_map()->get_scenario_player_tribe(m_player).c_str());
   int i=0;
   if(tribe) {
      for(i=0; i<tribe->get_nrbuildings(); i++) {
         Building_Descr* b=tribe->get_building_descr(i);
         if(!b->get_buildable() && !b->get_enhanced_building()) continue;
         std::string name=b->get_name();
         std::string trig_name= m_trigger->get_building();
         if(name==trig_name) m_building=m_buildings.size();
         m_buildings.push_back(name);
      }
   }

   // Name editbox
   new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UI::Edit_Box(this, spacing+60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
   m_name->set_text( trigger->get_name() );
   posy+=20+spacing;

   // Player
   new UI::Textarea(this, spacing, posy, 70, 20, _("Player: "), Align_CenterLeft);
   m_player_ta=new UI::Textarea(this, spacing+70, posy, 20, 20, "2", Align_Center);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 15);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 16);

   posy+=20+spacing;

   // Building
   new UI::Textarea(this, spacing, posy, 70, 20, _("Building: "), Align_CenterLeft);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 70, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 23);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 24);

   posy+=20+spacing;
   m_building_ta=new UI::Textarea(this, 0, posy, get_inner_w(), 20, _("Headquarters"), Align_Center);
   posy+=20+spacing;

   // Count
   new UI::Textarea(this, spacing, posy, 70, 20, _("How many: "), Align_CenterLeft);
   m_count_ta=new UI::Textarea(this, spacing+70, posy, 20, 20, "2", Align_Center);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 25);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 130, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 26);

   posy+=20+spacing;

   // Set Field Buttons
   new UI::Textarea(this, spacing, posy, get_inner_w(), 15, _("Current position: "), Align_CenterLeft);
   posy+=20+spacing;
   // X

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 20, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 3);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 20, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 4);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 40, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 5);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 40, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 6);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 60, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 7);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 60, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 8);

   new UI::Textarea(this, spacing+20, posy + 20, 20, 20, "X: ", Align_CenterLeft);
   m_x_ta=new UI::Textarea(this, spacing+40, posy + 20, 20, 20, "X: ", Align_CenterLeft);

   // Y
   int oldspacing=spacing;
   spacing=get_inner_w()/2+spacing;

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 9);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 10);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 20, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 11);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 20, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 12);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 40, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 13);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 40, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 14);

   new UI::Textarea(this, spacing, posy + 20, 20, 20, "Y: ", Align_CenterLeft);
   m_y_ta=new UI::Textarea(this, spacing+20, posy + 20, 20, 20, "Y: ", Align_CenterLeft);
   spacing=oldspacing;
   posy+=60+spacing;

   // Area
   new UI::Textarea(this, spacing, posy + 20, 70, 20, _("Area: "), Align_CenterLeft);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 70, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 17);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 70, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 18);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 19);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 90, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 20);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 21);

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 spacing + 110, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 22);

   m_area_ta=new UI::Textarea(this, spacing+90, posy + 20, 20, 20, "2", Align_Center);
   posy+=60+spacing;


   // Ok/Cancel Buttons
   posy+=spacing; // Extra space
   posx=(get_inner_w()/2)-60-spacing;
	new UI::Button<Trigger_Building_Option_Menu>
		(this,
		 posx, posy, 60, 20,
		 0,
		 &Trigger_Building_Option_Menu::clicked_ok, this,
		 _("Ok"));
   posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Trigger_Building_Option_Menu, int>
		(this,
		 posx, posy, 60, 20,
		 1,
		 &Trigger_Building_Option_Menu::end_modal, this, 0,
		_("Cancel"));

   set_inner_size(get_inner_w(), posy + 20+spacing);
   center_to_parent();
   update();
}

/*
 * cleanup
 */
Trigger_Building_Option_Menu::~Trigger_Building_Option_Menu(void) {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Trigger_Building_Option_Menu::handle_mousepress(const Uint8 btn, int, int)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Trigger_Building_Option_Menu::handle_mouserelease(const Uint8, int, int)
{return false;}


void Trigger_Building_Option_Menu::clicked_ok() {
	if (m_name->get_text()) m_trigger->set_name(m_name->get_text());
	m_trigger->set_coords(Coords(m_x, m_y));
	if (m_trigger->get_player() != m_player && m_trigger->get_player() != -1)
		m_parent->unreference_player_tribe(m_trigger->get_player(), m_trigger);
	if (m_trigger->get_player() != m_player) {
		m_trigger->set_player(m_player);
		m_parent->reference_player_tribe(m_player, m_trigger);
	}
	m_trigger->set_area          (m_area);
	m_trigger->set_building      (m_buildings[m_building].c_str());
	m_trigger->set_building_count(m_count);
	m_parent ->set_need_save     (true);
	end_modal(1);
}


void Trigger_Building_Option_Menu::clicked(int i) {
   switch(i) {
      case 3: m_x+=100; break;
      case 4: m_x-=100; break;
      case 5: m_x+=10; break;
      case 6: m_x-=10; break;
      case 7: m_x+=1; break;
      case 8: m_x-=1; break;
      case 9: m_y+=100; break;
      case 10: m_y-=100; break;
      case 11: m_y+=10; break;
      case 12: m_y-=10; break;
      case 13: m_y+=1; break;
      case 14: m_y-=1; break;

      case 15: m_player++; break;
      case 16: m_player--; break;

      case 17: m_area+=100; break;
      case 18: m_area-=100; break;
      case 19: m_area+=10; break;
      case 20: m_area-=10; break;
      case 21: m_area+=1; break;
      case 22: m_area-=1; break;

      case 23: m_building++; break;
      case 24: m_building--; break;

      case 25: m_count++; break;
      case 26: m_count--; break;
   }
   update();
}

/*
 * update function: update all UI elements
 */
void Trigger_Building_Option_Menu::update(void) {
   if(m_x<0) m_x=0;
   if(m_y<0) m_y=0;
   if(m_x>=((int)m_parent->get_map()->get_width())) m_x=m_parent->get_map()->get_width()-1;
   if(m_y>=((int)m_parent->get_map()->get_height())) m_y=m_parent->get_map()->get_height()-1;

   if(m_player<=0) m_player=1;
   if(m_player>m_parent->get_map()->get_nrplayers()) m_player=m_parent->get_map()->get_nrplayers();

   if(m_area<1) m_area=1;
   if(m_count<1) m_count=1;

   if(m_building<0) m_building=0;
   if(m_building>=static_cast<int>(m_buildings.size())) m_building=m_buildings.size()-1;

   std::string curbuild=_("<invalid player tribe>");
   if(!m_buildings.size()) {
      m_player=-1;
      m_building=-1;
   } else {
      curbuild=m_buildings[m_building];
   }

   char buf[200];
   sprintf(buf, "%i", m_x);
   m_x_ta->set_text(buf);
   sprintf(buf, "%i", m_y);
   m_y_ta->set_text(buf);

   sprintf(buf, "%i", m_player);
   m_player_ta->set_text(buf);

   sprintf(buf, "%i", m_area);
   m_area_ta->set_text(buf);

   sprintf(buf, "%i", m_count);
   m_count_ta->set_text(buf);

   sprintf(buf, "\"%s\"", curbuild.c_str());
   m_building_ta->set_text(buf);
}
