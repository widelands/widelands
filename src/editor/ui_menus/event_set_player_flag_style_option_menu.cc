/*
 * Copyright (C) 2009 by the Widelands Development Team
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

#include "event_set_player_flag_style_option_menu.h"

#include "events/event_set_player_flag_style.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "i18n.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"

#include "font_handler.h"
#include "ui_basic/messagebox.h"

#include <cstdio>


inline Editor_Interactive & Event_Set_Player_Flag_Style_Option_Menu::eia()
{
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}

Event_Set_Player_Flag_Style_Option_Menu::
Event_Set_Player_Flag_Style_Option_Menu
	(Editor_Interactive                         & parent,
	 Widelands::Event_Set_Player_Flag_Style & event)
	:
	UI::Window
		(&parent,
		 0, 0, 320, 120,
		 _("Set Player's Flag Style Event Options")),
	m_event         (event),
	m_player_number (event.player_number()),
	m_style_index   (event.style_index()),
	label_name      (*this),
	name            (*this, event.name()),
	label_player    (*this),
	decrement_player(*this),
	increment_player(*this),
	ok              (*this),
	cancel          (*this)
{
	center_to_parent();
}


void Event_Set_Player_Flag_Style_Option_Menu::draw(RenderTarget & dst) {
	UI::Window::draw(dst);
	{
		char buffer[4];
		sprintf(buffer, "%u", m_player_number);
		UI::g_fh->draw_string
			(dst,
			 UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG, UI_FONT_CLR_BG,
			 Point(100, 60),
			 buffer, UI::Align_Center);
	}
	Widelands::Editor_Game_Base  & egbase = eia().egbase();
	Widelands::Tribe_Descr const & tribe  =
		egbase.manually_load_tribe(m_player_number);
	UI::g_fh->draw_string
		(dst,
		 UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG, UI_FONT_CLR_BG,
		 Point(315, 60),
		 tribe.flag_style_name(m_style_index), UI::Align_CenterRight);
	dst.drawanim
		(Point(160, 85),
		 tribe.flag_animation(m_style_index),
		 0,
		 &egbase.player(m_player_number));
}


bool Event_Set_Player_Flag_Style_Option_Menu::handle_mousepress
	(Uint8 const btn, int32_t const x, int32_t const y)
{
	if (btn == SDL_BUTTON_RIGHT) {
		end_modal(0);
		return true;
	} else if
		(btn == SDL_BUTTON_LEFT and
		 140 <= x and x <= 315 and 30 <= y and y <= 90)
	{
		uint8_t const new_style_index =
			eia().egbase().manually_load_tribe(m_player_number)
			.next_flag_style_index(m_style_index);
		if (new_style_index != m_style_index) {
			m_style_index = new_style_index;
			play_click();
			update(140, 30, 315 - 140 + 1, 90 - 30 + 1);
		}
		return true;
	}
	return false;
}
bool Event_Set_Player_Flag_Style_Option_Menu::handle_mouserelease
	(Uint8,           int32_t, int32_t)
{
	return false;
}


void Event_Set_Player_Flag_Style_Option_Menu::OK::clicked() const {
	Event_Set_Player_Flag_Style_Option_Menu & parent =
		ref_cast<Event_Set_Player_Flag_Style_Option_Menu, UI::Panel>
			(*get_parent());
	std::string const & name = parent.name.text();
	if (name.size()) {
		if
			(Widelands::Event * const registered_event =
			 	parent.eia().egbase().map().mem()[name])
			if (registered_event != & parent.m_event) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another event registered with the name \"%s\". "
					 	 "Choose another name."),
					 name.c_str());
				UI::WLMessageBox mb
					(parent.get_parent(),
					 _("Name in use"), buffer,
					 UI::WLMessageBox::OK);
				mb.run();
				return;
			}
		parent.m_event.set_name(name);
	}
	if (parent.m_event.player_number() != parent.m_player_number) {
		if (parent.m_event.player_number())
			parent.eia().unreference_player_tribe
				(parent.m_event.player_number(), &parent.m_event);
		parent.m_event.set_player(parent.m_player_number);
		parent.eia().reference_player_tribe
			(parent.m_player_number, &parent.m_event);
	}
	parent.m_event.set_style_index(parent.m_style_index);
	parent.eia().set_need_save(true);
	parent.end_modal(1);
}


void Event_Set_Player_Flag_Style_Option_Menu::change_player(bool const up)
{
	Widelands::Editor_Game_Base & egbase = eia().egbase();
	Widelands::Player_Number const nr_players = egbase.map().get_nrplayers();
	assert(1 < nr_players);
	assert(1 <= m_player_number);
	assert     (m_player_number <= nr_players);
	Widelands::Tribe_Descr      const & old_tribe =
		egbase.manually_load_tribe(m_player_number);
	if (up) {
		if (m_player_number == nr_players)
			m_player_number = 0;
		++m_player_number;
	} else {
		--m_player_number;
		if (not --m_player_number)
			m_player_number = nr_players;
	}
	Widelands::Tribe_Descr const & new_tribe =
		egbase.manually_load_tribe(m_player_number);
	if (&old_tribe != &new_tribe) {
		//  The new player belongs to another tribe than the old player. See if
		//  the new player's tribe has a flag style with the same name as the
		//  previously selected flag style. If not, select the first flag
		//  style in new player's tribe.
		try {
			m_style_index =
				new_tribe.flag_style_index
					(old_tribe.flag_style_name(m_style_index));
		} catch (Widelands::Tribe_Descr::Nonexistent) {
			m_style_index = 0;
		}
	}
}
