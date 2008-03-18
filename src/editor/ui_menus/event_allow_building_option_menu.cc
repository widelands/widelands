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

#include "ui_messagebox.h"

#include <stdio.h>


inline Editor_Interactive & Event_Allow_Building_Option_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


inline static void update_label_player
(UI::Textarea & ta, Widelands::Player_Number const p)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), _("Player: %u"), p);
	ta.set_text(buffer);
}


inline static void update_label_building
(UI::Textarea & ta, Widelands::Building_Descr const & building)
{
	char buffer[128];
	snprintf
		(buffer, sizeof(buffer), _("Building: %s"), building.descname().c_str());
	ta.set_text(buffer);
}


#define spacing 5
#define button_width  20U
#define button_height 20U
#define button_size button_width, button_height
Event_Allow_Building_Option_Menu::Event_Allow_Building_Option_Menu
(Editor_Interactive & parent, Widelands::Event_Allow_Building & event)
:
UI::Window(&parent, 0, 0, 200, 280, _("Allow Building Event Options")),
m_event   (event),
m_player  (m_event.m_player),
m_building(m_event.m_building),
m_label_name(this, spacing, spacing, 50, 20, _("Name:"), Align_CenterLeft),

m_name
(this,
 m_label_name.get_x() + m_label_name.get_w() + spacing, m_label_name.get_y(),
 get_inner_w() - m_label_name.get_x() - m_label_name.get_w() - 3 * spacing, 20,
 0, 0),

m_label_player
(this,
 spacing, m_label_name.get_y() + m_label_name.get_h() + spacing,
 get_inner_w() - 2 * (2 * spacing + button_width), 20,
 " ", Align_Left),

m_decrement_player
(this,
 get_inner_w() - 2 * (spacing + 20), m_label_player.get_y(), 20, 20,
 0,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
 &Event_Allow_Building_Option_Menu::clicked_change_player, this, false),

m_increment_player
(this,
 get_inner_w() - 1 * (spacing + 20), m_label_player.get_y(), 20, 20,
 0,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
 &Event_Allow_Building_Option_Menu::clicked_change_player, this, true),

m_label_building
(this,
 spacing, m_label_player.get_y() + m_label_player.get_h() + spacing,
 m_label_player.get_w(), 20,
 " ", Align_Left),

m_decrement_building
(this,
 get_inner_w() - 2 * (spacing + 20), m_label_building.get_y(), button_size,
 0,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
 &Event_Allow_Building_Option_Menu::clicked_decrement_building, this),

m_increment_building
(this,
 get_inner_w() - 1 * (spacing + 20), m_label_building.get_y(), button_size,
 0,
 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
 &Event_Allow_Building_Option_Menu::clicked_increment_building, this),

m_label_allow
(this,
 spacing,
 m_increment_building.get_y() + m_increment_building.get_h() + spacing,
 get_inner_w() - 3 * spacing - 20, 20,
 _("Allow Building: "), Align_CenterLeft),

m_allow(this, get_inner_w() - spacing - 20, m_label_allow.get_y()),

m_button_ok
(this,
 (get_inner_w() >> 1) - 60 - spacing,
 m_label_allow.get_y() + m_label_allow.get_h() + spacing,
 3 * button_width, button_height,
 0,
 &Event_Allow_Building_Option_Menu::clicked_ok, this,
 _("Ok")),

m_button_cancel
(this,
 m_button_ok.get_x() + m_button_ok.get_w() + spacing, m_button_ok.get_y(),
 3 * button_width, button_height,
 1,
 &Event_Allow_Building_Option_Menu::end_modal, this, 0,
 _("Cancel"))
{
	if (m_player == 0)
		m_player = 1;
	Widelands::Editor_Game_Base const & egbase = parent.egbase();
	assert(m_player <= egbase.map().get_nrplayers());
	Widelands::Tribe_Descr const & tribe =
		*egbase.get_tribe
		(egbase.map()
		 .get_scenario_player_tribe(m_player).c_str());
	if (not m_building)
		m_building = static_cast<Widelands::Building_Index::value_t>(0);
	{
		const bool has_several_players = 1 < egbase.map().get_nrplayers();
		m_decrement_player.set_enabled(has_several_players);
		m_increment_player.set_enabled(has_several_players);
	}

	m_name.setText(event.name());

	set_inner_size
		(get_inner_w(), m_button_ok.get_y() + m_button_ok.get_h() + spacing);
	center_to_parent();
	update_label_player(m_label_player, m_player);
	update_label_building
		(m_label_building, *tribe.get_building_descr(m_building));
	m_allow.set_state(m_event.get_allow());
}


/**
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
	const std::string& name = m_name.text();
	if (name.size()) {
		if
			(Widelands::Event * const registered_event =
			 eia().egbase().map().mem()[name])
			if (registered_event != & m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 ("There is another event registered with the name \"%s\". "
					  "Choose another name."),
					 name.c_str());
				UI::MessageBox mb
					(get_parent(),
					 _("Name in use"), buffer,
					 UI::MessageBox::OK);
				mb.run();
				return;
			}
		m_event.set_name(name);
	}
	if (m_event.m_player != m_player) {
		if (m_event.m_player)
			eia().unreference_player_tribe(m_event.m_player, &m_event);
		m_event.m_player = m_player;
		eia().reference_player_tribe(m_player, &m_event);
	}
	m_event.m_building = m_building;
	m_event.set_allow(m_allow.get_state());
	eia().set_need_save(true);
	end_modal(1);
}


///  Change the player number 1 step in any direction. Wraps around.
void Event_Allow_Building_Option_Menu::clicked_change_player(const bool up) {
	Widelands::Editor_Game_Base const & egbase    = eia().egbase();
	Widelands::Map              const & map       = egbase.map();
	Widelands::Tribe_Descr      const & old_tribe =
		egbase.player(m_player).tribe();
	Widelands::Player_Number const nr_players = map.get_nrplayers();
	assert(1 < nr_players);
	assert(1 <= m_player);
	assert     (m_player <= nr_players);
	if (up) {
		if (m_player == nr_players)
			m_player = 0;
		++m_player;
	} else {
		--m_player;
		if (0 == m_player)
			m_player = nr_players;
	}
	Widelands::Tribe_Descr const & new_tribe = egbase.player(m_player).tribe();
	if (&old_tribe != &new_tribe) {
		//  The new player belongs to another tribe than the old player. See if
		//  the new player's tribe has a building with the same name as the
		//  previously selected building. If not, select the first building in
		//  the new player's tribe.
		{
			const int i =
				new_tribe.get_building_index
				(old_tribe.get_building_descr(m_building)->name().c_str());
			m_building =
				static_cast<Widelands::Building_Index::value_t>(i == -1 ? 0 : i);
		}
		update_label_building
			(m_label_building, *new_tribe.get_building_descr(m_building));
		const bool has_several_buildings = 1 < new_tribe.get_nrbuildings();
		m_decrement_building.set_enabled(has_several_buildings);
		m_increment_building.set_enabled(has_several_buildings);
	}
	update_label_player(m_label_player, m_player);
}


void Event_Allow_Building_Option_Menu::clicked_increment_building() {
	Widelands::Editor_Game_Base const & egbase = eia().egbase();
	Widelands::Tribe_Descr      const & tribe = egbase.player(m_player).tribe();
	m_building =
		static_cast<Widelands::Building_Index::value_t>(m_building.value() + 1);
	if (m_building.value() == tribe.get_nrbuildings())
		m_building = static_cast<Widelands::Building_Index::value_t>(0);
	update_label_building
		(m_label_building, *tribe.get_building_descr(m_building));
}


void Event_Allow_Building_Option_Menu::clicked_decrement_building() {
	Widelands::Editor_Game_Base const & egbase = eia().egbase();
	Widelands::Tribe_Descr      const & tribe = egbase.player(m_player).tribe();
	if (0 == m_building.value())
		m_building = tribe.get_nrbuildings();
	m_building =
		static_cast<Widelands::Building_Index::value_t>(m_building.value() - 1);
	update_label_building
		(m_label_building, *tribe.get_building_descr(m_building));
}
