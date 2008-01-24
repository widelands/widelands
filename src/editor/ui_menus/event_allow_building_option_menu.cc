/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "event_allow_building_option_menu.h"

#include "editor_game_base.h"
#include "editorinteractive.h"
#include "events/event_allow_building.h"
#include "graphic.h"
#include "i18n.h"
#include "map.h"
#include "player.h"
#include "tribe.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"
#include "ui_window.h"

#include <stdio.h>


inline Editor_Interactive & Event_Allow_Building_Option_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


#define spacing 5
Event_Allow_Building_Option_Menu::Event_Allow_Building_Option_Menu
(Editor_Interactive & parent, Widelands::Event_Allow_Building & event)
:
UI::Window(&parent, 0, 0, 200, 280, _("Allow Building Event Options").c_str()),
m_event(event),
m_player  (m_event.get_player()),
m_building(-1) //  FIXME negative value!
{
   const int32_t offsx=5;
   const int32_t offsy=25;
   int32_t posx=offsx;
   int32_t posy=offsy;

	if (m_player < 1)
		m_player = 1;

   // Fill the building infos
	Widelands::Editor_Game_Base & editor = eia().egbase();
	if
		(Widelands::Tribe_Descr const * const tribe = editor.get_tribe
		 (editor.map().get_scenario_player_tribe(m_player).c_str()))
	{
		Widelands::Building_Descr::Index const nr_buildings =
			tribe->get_nrbuildings();
		for (Widelands::Building_Descr::Index i = 0; i < nr_buildings; ++i) {
			Widelands::Building_Descr & building = *tribe->get_building_descr(i);
			if (building.get_buildable() or building.get_enhanced_building()) {
				std::string const & name = building.name();
				if (name == m_event.get_building())
					m_building = m_buildings.size();
				m_buildings.push_back(name);
			}
		}
	}

   // Name editbox
   new UI::Textarea(this, spacing, posy, 50, 20, _("Name:"), Align_CenterLeft);
   m_name=new UI::Edit_Box(this, spacing+60, posy, get_inner_w()-2*spacing-60, 20, 0, 0);
	m_name->set_text(event.name().c_str());
   posy+=20+spacing;

   // Player
   new UI::Textarea(this, spacing, posy, 70, 20, _("Player: "), Align_CenterLeft);
   m_player_ta=new UI::Textarea(this, spacing+70, posy, 20, 20, "2", Align_Center);

	new UI::IDButton<Event_Allow_Building_Option_Menu, int32_t>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Allow_Building_Option_Menu::clicked, this, 15);

	new UI::IDButton<Event_Allow_Building_Option_Menu, int32_t>
		(this,
		 spacing + 110, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Allow_Building_Option_Menu::clicked, this, 16);

   posy+=20+spacing;

   // Building
   new UI::Textarea(this, spacing, posy, 70, 20, _("Building: "), Align_CenterLeft);

	new UI::IDButton<Event_Allow_Building_Option_Menu, int32_t>
		(this,
		 spacing + 70, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
		 &Event_Allow_Building_Option_Menu::clicked, this, 23);

	new UI::IDButton<Event_Allow_Building_Option_Menu, int32_t>
		(this,
		 spacing + 90, posy, 20, 20,
		 0,
		 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
		 &Event_Allow_Building_Option_Menu::clicked, this, 24);

   posy+=20+spacing;
   m_building_ta=new UI::Textarea(this, 0, posy, get_inner_w(), 20, _("Headquarters"), Align_Center);
   posy+=20+spacing;

   // Enable
   new UI::Textarea(this, spacing, posy, 150, 20, _("Allow Building: "), Align_CenterLeft);
   m_allow=new UI::Checkbox(this, spacing+150, posy);
	m_allow->set_state(m_event.get_allow());
   posy+=20+spacing;

   // Ok/Cancel Buttons
   posy+=spacing; // Extra space
   posx=(get_inner_w()/2)-60-spacing;

	new UI::Button<Event_Allow_Building_Option_Menu>
		(this,
		 posx, posy, 60, 20,
		 0,
		 &Event_Allow_Building_Option_Menu::clicked_ok, this,
		 _("Ok"));

   posx=(get_inner_w()/2)+spacing;

	new UI::IDButton<Event_Allow_Building_Option_Menu, int32_t>
		(this,
		 posx, posy, 60, 20,
		 1,
		 &Event_Allow_Building_Option_Menu::end_modal, this, 0,
		 _("Cancel"));

   set_inner_size(get_inner_w(), posy+20+spacing);
   center_to_parent();
   update();
}

/*
 * cleanup
 */
Event_Allow_Building_Option_Menu::~Event_Allow_Building_Option_Menu() {
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Event_Allow_Building_Option_Menu::handle_mousepress
(const Uint8 btn, int32_t, int32_t)
{if (btn == SDL_BUTTON_RIGHT) {end_modal(0); return true;} return false;}
bool Event_Allow_Building_Option_Menu::handle_mouserelease
(const Uint8, int32_t, int32_t)
{return false;}


void Event_Allow_Building_Option_Menu::clicked_ok() {
	if (char const * const name = m_name->get_text()) {
		if
			(Widelands::Event * const registered_event =
			 eia().egbase().map().mem()[name])
			if (registered_event != & m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _("There is another event registered with the name \"%s\". "
					   "Choose another name.")
					 .c_str(),
					 name);
				UI::Modal_Message_Box mb
					(get_parent(),
					 _("Name in use"), buffer,
					 UI::Modal_Message_Box::OK);
				mb.run();
				return;
			}
		m_event.set_name(name);
	}
	if (m_event.get_player()!=m_player && m_event.get_player() != -1)
		eia().unreference_player_tribe(m_event.get_player(), &m_event);
	if (m_event.get_player()!=m_player) {
		m_event.set_player(m_player);
		eia().reference_player_tribe(m_player, &m_event);
	}
	m_event.set_building(m_buildings[m_building].c_str());
	m_event.set_allow(m_allow->get_state());
	eia().set_need_save(true);
            end_modal(1);
}


void Event_Allow_Building_Option_Menu::clicked(int32_t i) {
	switch (i) {
	case 15: ++m_player;   break;
	case 16: --m_player;   break;

	case 23: ++m_building; break;
	case 24: --m_building; break;
	}
   update();
}

/*
 * update function: update all UI elements
 */
void Event_Allow_Building_Option_Menu::update() {
	if (m_player <= 0)
		m_player = 1;
	Widelands::Player_Number const nr_players =
		eia().egbase().map().get_nrplayers();
	if (m_player > nr_players) m_player = nr_players;

	if (m_building < 0)
		m_building = 0;
	if (m_building >= static_cast<int32_t>(m_buildings.size()))
		m_building = m_buildings.size() - 1;

   std::string curbuild=_("<invalid player tribe>");
	if (!m_buildings.size()) {
      m_player=-1;
      m_building=-1;
	} else {
      curbuild=m_buildings[m_building];
	}

   char buf[200];
   sprintf(buf, "%i", m_player);
   m_player_ta->set_text(buf);

   sprintf(buf, "\"%s\"", curbuild.c_str());
   m_building_ta->set_text(buf);
}
