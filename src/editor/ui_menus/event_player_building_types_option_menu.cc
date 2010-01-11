/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "event_player_building_types_option_menu.h"

#include "logic/editor_game_base.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/tribe.h"

#include "font_handler.h"
#include "ui_basic/messagebox.h"

#include <cstdio>


inline Editor_Interactive & Event_Player_Building_Types_Option_Menu::eia() {
	return ref_cast<Editor_Interactive, UI::Panel>(*get_parent());
}


Event_Player_Building_Types_Option_Menu::
Event_Player_Building_Types_Option_Menu
	(Editor_Interactive                     & parent,
	 Widelands::Event_Player_Building_Types & event)
	:
	UI::Window      (&parent, 0, 0, 320, 360, ""),
	m_event         (event),
	m_player_number (event.player_number()),
	label_name      (*this),
	name            (*this, event.name()),
	label_player    (*this),
	decrement_player(*this, 1 < parent.egbase().map().get_nrplayers()),
	increment_player(*this, 1 < parent.egbase().map().get_nrplayers()),
	clear_selection (*this),
	invert_selection(*this),
	table           (*this, event),
	ok              (*this),
	cancel          (*this)
{
	{
		char buffer[256];
		snprintf
			(buffer, sizeof(buffer),
			 _("%s Building Types Event Options"), event.action_name());
		set_title(buffer);
	}

	center_to_parent();
}


void Event_Player_Building_Types_Option_Menu::draw(RenderTarget & dst) {
	UI::Window::draw(dst);
	char buffer[4];
	sprintf(buffer, "%u", m_player_number);
	UI::g_fh->draw_string
		(dst,
		 UI_FONT_NAME, UI_FONT_SIZE_SMALL, UI_FONT_CLR_FG, UI_FONT_CLR_BG,
		 Point(100, 40),
		 buffer, UI::Align_Center);
}


Event_Player_Building_Types_Option_Menu::Decrement_Player::Decrement_Player
	(Event_Player_Building_Types_Option_Menu & parent, bool enable)
	:
	UI::Button
		(&parent,
		 65, 30, 20, 20,
		 g_gr->get_no_picture(),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 std::string(),
		 enable)
{
	set_repeating(true);
}
Event_Player_Building_Types_Option_Menu::Increment_Player::Increment_Player
	(Event_Player_Building_Types_Option_Menu & parent, bool const enable)
	:
	UI::Button
		(&parent,
		 115, 30, 20, 20,
		 g_gr->get_no_picture(),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 std::string(),
		 enable)
{
	set_repeating(true);
}


Event_Player_Building_Types_Option_Menu::Table::Table
	(Event_Player_Building_Types_Option_Menu & parent,
	 Widelands::Event_Player_Building_Types  & event)
	:
	UI::Table<uintptr_t const>(&parent, 5, 55, 310, 275)
{
	add_column (60, event.action_name(), UI::Align_HCenter, true);
	add_column (32);
	add_column (50, _("Size"),           UI::Align_HCenter);
	add_column(144, _("Name"),           UI::Align_Left);
	fill
		(parent.eia().egbase().manually_load_tribe(event.player_number()),
		 event.building_types());
}


void Event_Player_Building_Types_Option_Menu::Table::fill
	(Widelands::Tribe_Descr                                 const & tribe,
	 Widelands::Event_Player_Building_Types::Building_Types const & bld_types)
{
	for
		(struct {
		 	Widelands::Building_Index       current;
		 	Widelands::Building_Index const end;
		 } i = {Widelands::Building_Index::First(), tribe.get_nrbuildings()};
		 i.current < i.end;
		 ++i.current)
	{
		Widelands::Building_Descr const & building =
			*tribe.get_building_descr(i.current);

		//  walk all entries, add new ones if needed
		UI::Table<uintptr_t const>::Entry_Record & te = add(i.current.value());

		if (bld_types.count(i.current))
			te.set_checked(Selected, true);
		te.set_picture(Icon, building.get_buildicon());
		te.set_string (Name, building.descname     ());

		{
			char const * str = "";
			char const * pic = "pics/novalue.png";
			if (building.get_ismine()) {
				str = "M";
				pic = "pics/mine.png";
			} else switch (building.get_size()) {
			case Widelands::BaseImmovable::SMALL:
				str = "1";
				pic = "pics/small.png";
				break;
			case Widelands::BaseImmovable::MEDIUM:
				str = "2";
				pic = "pics/medium.png";
				break;
			case Widelands::BaseImmovable::BIG:
				str = "3";
				pic = "pics/big.png";
				break;
			default:
				assert(false);
			}
			te.set_picture(Size, g_gr->get_picture(PicMod_UI, pic), str);
		}
	}
}


void Event_Player_Building_Types_Option_Menu::OK::clicked() {
	Event_Player_Building_Types_Option_Menu & parent =
		ref_cast<Event_Player_Building_Types_Option_Menu, UI::Panel>
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
	Widelands::Event_Player_Building_Types::Building_Types building_types;
	for
		(struct {uint8_t current; uint8_t const end;} i =
		 	{0, parent.table.size()};
		 i.current < i.end;
		 ++i.current)
		if (parent.table.get_record(i.current).is_checked(Table::Selected))
			building_types.insert
				(Widelands::Building_Index(parent.table[i.current]));
	if (building_types.empty()) {
		char buffer[256];
		snprintf
			(buffer, sizeof(buffer),
			 _("No building types are selected. Select at least one."));
		UI::WLMessageBox mb
			(parent.get_parent(),
			 _("Nothing selected"), buffer,
			 UI::WLMessageBox::OK);
		mb.run();
		return;
	}
	parent.m_event.building_types() = building_types;
	parent.eia().set_need_save(true);
	parent.end_modal(1);
}


///  Change the player number 1 step in any direction. Wraps around.
void Event_Player_Building_Types_Option_Menu::change_player(bool const up)
{
	Widelands::Editor_Game_Base       & egbase    = eia().egbase();
	Widelands::Tribe_Descr      const & old_tribe =
		egbase.manually_load_tribe(m_player_number);
	Widelands::Player_Number const nr_players =
		eia().egbase().map().get_nrplayers();
	assert(1 < nr_players);
	assert(1 <= m_player_number);
	assert     (m_player_number <= nr_players);
	if (up) {
		if (m_player_number == nr_players)
			m_player_number = 0;
		++m_player_number;
	} else {
		--m_player_number;
		if (0 == m_player_number)
			m_player_number = nr_players;
	}
	Widelands::Tribe_Descr const & new_tribe =
		egbase.manually_load_tribe(m_player_number);

	//  When changing to a player with another tribe, try to select a set of
	//  building types that is as close as possible to the previously selected
	//  set. To do this, look in the new tribe for a building type with the same
	//  name as the selected building type has in the old tribe.
	if (&old_tribe != &new_tribe) {
		/// Set of building types that are selected in the old tribe and also
		/// exist in the new tribe.
		Widelands::Event_Player_Building_Types::Building_Types selected_common;
		for
			(struct {uint8_t current; uint8_t const end;} i = {0, table.size()};
			 i.current < i.end;
			 ++i.current)
		{
			Table::Entry_Record const & er = table.get_record(i.current);
			if (er.is_checked(Table::Selected))
				if
					(Widelands::Building_Index const index_in_new_tribe =
					 	new_tribe.building_index
					 		(old_tribe.get_building_descr
					 		 	(Widelands::Building_Index(table[i.current]))
					 		 ->name()))
				{
					assert(not selected_common.count(index_in_new_tribe));
					selected_common.insert(index_in_new_tribe);
				}
		}

		table.clear();
		table.fill(new_tribe, selected_common);
	}
}
