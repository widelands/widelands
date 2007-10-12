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

#include "trigger_building_option_menu.h"

#include "i18n.h"
#include "trigger/trigger_building.h"
#include "editorinteractive.h"
#include "map.h"
#include "graphic.h"
#include "player.h"
#include "tribe.h"

#include "ui_window.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"

#include <stdio.h>


Trigger_Building_Option_Menu::Trigger_Building_Option_Menu(Editor_Interactive* parent, Trigger_Building* trigger) :
UI::Window(parent, 0, 0, 180, 280, _("Building Trigger Options").c_str()),
m_trigger (trigger),
m_parent     (parent),
m_player_area(trigger->m_player_area)
{
   m_building=-1;
   const int32_t offsx=5;
   const int32_t offsy=25;
   int32_t spacing=5;
   int32_t posx=offsx;
   int32_t posy=offsy;

   // Fill the building infos
	if
		(const Tribe_Descr * const tribe = m_parent->egbase().get_tribe
		 (m_parent->egbase().map()
		  .get_scenario_player_tribe(m_player_area.player_number).c_str()))
	{
		for (int32_t i = 0; i < tribe->get_nrbuildings(); ++i) {
         Building_Descr* b=tribe->get_building_descr(i);
         if (!b->get_buildable() && !b->get_enhanced_building()) continue;
         std::string name=b->name();
         std::string trig_name= m_trigger->get_building();
         if (name==trig_name) m_building=m_buildings.size();
         m_buildings.push_back(name);
		}
	}

   // Name editbox
   new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UI::Edit_Box(this, spacing+60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
   m_name->set_text(trigger->get_name());
   posy+=20+spacing;

   // Player
   new UI::Textarea(this, spacing, posy, 70, 20, _("Player: "), Align_CenterLeft);
   m_player_ta=new UI::Textarea(this, spacing+70, posy, 20, 20, "2", Align_Center);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 15);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 16);

   posy+=20+spacing;

   // Building
   new UI::Textarea(this, spacing, posy, 70, 20, _("Building: "), Align_CenterLeft);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 70, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 23);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
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

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 25);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
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

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 3);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 4);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 5);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 6);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 60, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 7);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 60, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 8);

   new UI::Textarea(this, spacing+20, posy + 20, 20, 20, "X: ", Align_CenterLeft);
   m_x_ta=new UI::Textarea(this, spacing+40, posy + 20, 20, 20, "X: ", Align_CenterLeft);

   // Y
   int32_t oldspacing=spacing;
   spacing=get_inner_w()/2+spacing;

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 9);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 10);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 11);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 20, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 12);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 40, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 13);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
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

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 70, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 17);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 70, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 18);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 19);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 90, posy + 40, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 20);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Trigger_Building_Option_Menu::clicked, this, 21);

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
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

	new UI::IDButton<Trigger_Building_Option_Menu, int32_t>
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
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Trigger_Building_Option_Menu::handle_mousepress(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Trigger_Building_Option_Menu::handle_mouserelease(const Uint8, int32_t, int32_t)
{return false;}


void Trigger_Building_Option_Menu::clicked_ok() {
	if (m_name->get_text()) m_trigger->set_name(m_name->get_text());
	const Player_Number trigger_player_number =
		m_trigger->m_player_area.player_number;
	if (trigger_player_number != m_player_area.player_number) {
		if (trigger_player_number)
			m_parent->unreference_player_tribe(trigger_player_number, m_trigger);
		m_parent->reference_player_tribe(m_player_area.player_number, m_trigger);
	}
	m_trigger->m_player_area = m_player_area;
	m_trigger->set_building      (m_buildings[m_building].c_str());
	m_trigger->set_building_count(m_count);
	m_parent ->set_need_save     (true);
	end_modal(1);
}


void Trigger_Building_Option_Menu::clicked(int32_t i) {
	switch (i) {
	case  3: m_player_area.x      += 100; break;
	case  4: m_player_area.x      -= 100; break;
	case  5: m_player_area.x      +=  10; break;
	case  6: m_player_area.x      -=  10; break;
	case  7: m_player_area.x      +=   1; break;
	case  8: m_player_area.x      -=   1; break;
	case  9: m_player_area.y      += 100; break;
	case 10: m_player_area.y      -= 100; break;
	case 11: m_player_area.y      +=  10; break;
	case 12: m_player_area.y      -=  10; break;
	case 13: m_player_area.y      +=   1; break;
	case 14: m_player_area.y      -=   1; break;

	case 15: ++m_player_area.player_number; break;
	case 16: --m_player_area.player_number; break;

	case 17: m_player_area.radius += 100; break;
	case 18: m_player_area.radius -= 100; break;
	case 19: m_player_area.radius +=  10; break;
	case 20: m_player_area.radius -=  10; break;
	case 21: m_player_area.radius +=   1; break;
	case 22: m_player_area.radius -=   1; break;

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
void Trigger_Building_Option_Menu::update() {
	if (m_player_area.x < 0) m_player_area.x = 0;
	if (m_player_area.y < 0) m_player_area.y = 0;
	const Map & map = m_parent->egbase().map();
	const X_Coordinate mapwidth  = map.get_width ();
	const Y_Coordinate mapheight = map.get_height();
	if (m_player_area.x >= static_cast<int32_t>(mapwidth))
		m_player_area.x = mapwidth  - 1;
	if (m_player_area.y >= static_cast<int32_t>(mapheight))
		m_player_area.y = mapheight - 1;
	m_player_area.field = map.get_field(m_player_area);

	if (m_player_area.player_number < 1) m_player_area.player_number = 1;
	const Player_Number nr_players = map.get_nrplayers();
	if (nr_players < m_player_area.player_number)
		m_player_area.player_number = nr_players;

	if (m_player_area.radius < 1) m_player_area.radius = 1;

   if (m_count<1) m_count=1;

   if (m_building>=static_cast<int32_t>(m_buildings.size())) m_building=m_buildings.size()-1;

   std::string curbuild=_("<invalid player tribe>");
	if (not m_buildings.size()) {
		m_player_area.player_number = 0;
      m_building=-1;
	} else {
      curbuild=m_buildings[m_building];
	}

   char buf[200];
	sprintf(buf, "%i", m_player_area.x);
   m_x_ta->set_text(buf);
	sprintf(buf, "%i", m_player_area.y);
   m_y_ta->set_text(buf);

	sprintf(buf, "%i", m_player_area.player_number);
   m_player_ta->set_text(buf);

	sprintf(buf, "%i", m_player_area.radius);
   m_area_ta->set_text(buf);

   sprintf(buf, "%i", m_count);
   m_count_ta->set_text(buf);

   sprintf(buf, "\"%s\"", curbuild.c_str());
   m_building_ta->set_text(buf);
}
