/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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
#include "editor/editorinteractive.h"
#include "map.h"
#include "graphic/graphic.h"
#include "logic/player.h"
#include "tribe.h"

#include "ui_basic/messagebox.h"

#include <cstdio>

using Widelands::Trigger_Building;

int32_t Trigger_Building::option_menu(Editor_Interactive & eia) {
	Trigger_Building_Option_Menu m(eia, *this); return m.run();
}


inline Editor_Interactive & Trigger_Building_Option_Menu::eia() {
	return dynamic_cast<Editor_Interactive &>(*get_parent());
}


inline static void update_label_player
	(UI::Textarea & ta, Widelands::Player_Number const p)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), _("Player: %u"), p);
	ta.set_text(buffer);
}


inline static void update_label_count
	(UI::Textarea & ta, Trigger_Building::Count_Type const count)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), _("How many: %u"), count);
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


inline static void update_label_coords
	(UI::Textarea & ta, Widelands::Coords const coords)
{
	char buffer[32];
	snprintf
		(buffer, sizeof(buffer), _("(X, Y): (%i, %i)"), coords.x, coords.y);
	ta.set_text(buffer);
}


inline static void update_label_radius
	(UI::Textarea & ta, Widelands::Player_Area<>::Radius_type const radius)
{
	char buffer[32];
	snprintf(buffer, sizeof(buffer), _("Radius: %u"), radius);
	ta.set_text(buffer);
}


#define spacing 5U
#define button_width  20U
#define button_height 20U
#define button_size button_width, button_height
Trigger_Building_Option_Menu::Trigger_Building_Option_Menu
	(Editor_Interactive & parent, Trigger_Building & trigger)
:
UI::Window(&parent, 0, 0, 280, 280, _("Building Trigger Options")),
m_trigger (trigger),
m_player_area
	(Widelands::Player_Area<>
	 	(trigger.m_player_area.player_number,
	 	 Widelands::Area<>
	 	 	(trigger.m_player_area, trigger.m_player_area.radius))),
m_count      (m_trigger.m_count),
m_label_name(this, spacing, spacing, 50, 20, _("Name:"), Align_CenterLeft),
m_name
	(this,
	 m_label_name.get_x() + m_label_name.get_w() + spacing,
	 m_label_name.get_y(),
	 get_inner_w() - m_label_name.get_x() - m_label_name.get_w() - 3 * spacing,
	 20,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"), 0),
m_label_player
	(this,
	 spacing, m_label_name.get_y() + m_label_name.get_h() + spacing,
	 get_inner_w() - 2 * (2 * spacing + button_width), 20,
	 " ", Align_Left),
m_decrement_player
	(this,
	 get_inner_w() - 2 * (spacing + 20), m_label_player.get_y(), 20, 20,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_change_player, *this, false),
m_increment_player
	(this,
	 get_inner_w() - 1 * (spacing + 20), m_label_player.get_y(), 20, 20,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_change_player, *this, true),
m_label_building
	(this,
	 spacing, m_label_player.get_y() + m_label_player.get_h() + spacing,
	 m_label_player.get_w(), 20,
	 " ", Align_Left),
m_decrement_building
	(this,
	 get_inner_w() - 2 * (spacing + 20), m_label_building.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_decrement_building, *this),
m_increment_building
	(this,
	 get_inner_w() - 1 * (spacing + 20), m_label_building.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_increment_building, *this),
m_label_count
	(this,
	 spacing, m_label_building.get_y() + m_label_building.get_h() + spacing,
	 m_label_building.get_w(), 20,
	 " ", Align_Left),
m_decrement_count
	(this,
	 m_decrement_building.get_x(), m_label_count.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_decrement_count, *this,
	 std::string(),
	 1 < m_count),
m_increment_count
	(this,
	 m_increment_building.get_x(), m_label_count.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_increment_count, *this,
	 std::string(),
	 m_count < std::numeric_limits<Trigger_Building::Count_Type>::max()),
m_label_coords
	(this,
	 spacing, m_label_count.get_y() + m_label_count.get_h() + spacing,
	 get_inner_w() - 2 * spacing, 20,
	 " ", Align_Left),
m_decrease_y_100
	(this,
	 (get_inner_w() - button_width) >> 1,
	 m_label_coords.get_y() + m_label_coords.get_h(),
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
	 &Trigger_Building_Option_Menu::clicked_decrease_y_coordinate, *this, 100),
m_decrease_y_10
	(this,
	 m_decrease_y_100.get_x(), m_decrease_y_100.get_y() + button_height,
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
	 &Trigger_Building_Option_Menu::clicked_decrease_y_coordinate, *this,  10),
m_decrease_y_1
	(this,
	 m_decrease_y_10.get_x(), m_decrease_y_10.get_y() + button_height,
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_up.png"),
	 &Trigger_Building_Option_Menu::clicked_decrease_y_coordinate, *this,   1),
m_decrease_x_100
	(this,
	 m_decrease_y_1.get_x() - 3 * button_width,
	 m_decrease_y_1.get_y() + button_height,
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_decrease_x_coordinate, *this, 100),
m_decrease_x_10
	(this,
	 m_decrease_x_100.get_x() + button_width, m_decrease_x_100.get_y(),
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_decrease_x_coordinate, *this,  10),
m_decrease_x_1
	(this,
	 m_decrease_x_10.get_x() + button_width, m_decrease_x_10.get_y(),
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_decrease_x_coordinate, *this,   1),
m_increase_x_1
	(this,
	 m_decrease_x_1.get_x() + 2 * button_width, m_decrease_x_1.get_y(),
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_increase_x_coordinate, *this,   1),
m_increase_x_10
	(this,
	 m_increase_x_1.get_x() + button_width, m_increase_x_1.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_increase_x_coordinate, *this,  10),
m_increase_x_100
	(this,
	 m_increase_x_10.get_x() + button_width, m_increase_x_10.get_y(),
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_increase_x_coordinate, *this, 100),
m_increase_y_1
	(this,
	 m_decrease_y_1.get_x(), m_increase_x_100.get_y() + button_height,
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
	 &Trigger_Building_Option_Menu::clicked_increase_y_coordinate, *this,   1),
m_increase_y_10
	(this,
	 m_increase_y_1.get_x(), m_increase_y_1.get_y() + button_height,
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
	 &Trigger_Building_Option_Menu::clicked_increase_y_coordinate, *this,  10),
m_increase_y_100
	(this,
	 m_increase_y_10.get_x(), m_increase_y_10.get_y() + button_height,
	 button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_down.png"),
	 &Trigger_Building_Option_Menu::clicked_increase_y_coordinate, *this, 100),
m_label_radius
	(this,
	 spacing, m_increase_y_100.get_y() + button_height, 20, 20,
	 " ", Align_Left),
m_decrement_radius
	(this,
	 m_decrement_count.get_x(), m_label_radius.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_left.png"),
	 &Trigger_Building_Option_Menu::clicked_decrement_radius, *this,
	 std::string(),
	 0 < m_player_area.radius),
m_increment_radius
	(this,
	 m_increment_count.get_x(), m_label_radius.get_y(), button_size,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 g_gr->get_picture(PicMod_Game, "pics/scrollbar_right.png"),
	 &Trigger_Building_Option_Menu::clicked_increment_radius, *this,
	 std::string(),
	 m_player_area.radius
	 <
	 std::numeric_limits<Widelands::Player_Area<>::Radius_type>::max()),
m_button_ok
	(this,
	 (get_inner_w() >> 1) - 60 - spacing,
	 m_label_radius.get_y() + m_label_radius.get_h() + spacing,
	 3 * button_width, button_height,
	 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
	 &Trigger_Building_Option_Menu::clicked_ok, *this,
	 _("Ok")),
m_button_cancel
	(this,
	 m_button_ok.get_x() + m_button_ok.get_w() + spacing, m_button_ok.get_y(),
	 3 * button_width, button_height,
	 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
	 &Trigger_Building_Option_Menu::end_modal, *this, 0,
	 _("Cancel"))
{
	m_decrement_player  .set_repeating(true);
	m_increment_player  .set_repeating(true);
	m_decrement_building.set_repeating(true);
	m_increment_building.set_repeating(true);
	m_decrement_count   .set_repeating(true);
	m_increment_count   .set_repeating(true);
	m_decrease_y_100    .set_repeating(true);
	m_decrease_y_10     .set_repeating(true);
	m_decrease_y_1      .set_repeating(true);
	m_decrease_x_100    .set_repeating(true);
	m_decrease_x_10     .set_repeating(true);
	m_decrease_x_1      .set_repeating(true);
	m_increase_x_1      .set_repeating(true);
	m_increase_x_10     .set_repeating(true);
	m_increase_x_100    .set_repeating(true);
	m_increase_y_1      .set_repeating(true);
	m_increase_y_10     .set_repeating(true);
	m_increase_y_100    .set_repeating(true);
	m_decrement_radius  .set_repeating(true);
	m_increment_radius  .set_repeating(true);

	if (m_player_area.player_number == 0)
		m_player_area.player_number = 1;
	assert(m_count);
	Widelands::Editor_Game_Base & egbase = parent.egbase();
	Widelands::Player_Number const nr_players = egbase.map().get_nrplayers();
	assert(m_player_area.player_number <= nr_players);
	Widelands::Tribe_Descr const & tribe =
		egbase.manually_load_tribe(m_player_area.player_number);
	m_building =
		trigger.m_building_type ?
		tribe.building_index(trigger.m_building_type->name().c_str())
		:
		Widelands::Building_Index::First();
	{
		bool const has_several_players = 1 < nr_players;
		m_decrement_player.set_enabled(has_several_players);
		m_increment_player.set_enabled(has_several_players);
	}

	m_name.setText(trigger.name());

	set_inner_size
		(get_inner_w(), m_button_ok.get_y() + m_button_ok.get_h() + spacing);
	center_to_parent();
	update_label_player(m_label_player, m_player_area.player_number);
	update_label_building
		(m_label_building, *tribe.get_building_descr(m_building));
	update_label_count (m_label_count,  m_count);
	update_label_coords(m_label_coords, m_player_area);
	update_label_radius(m_label_radius, m_player_area.radius);
}


/**
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 * We are not draggable.
 */
bool Trigger_Building_Option_Menu::
	handle_mousepress(const Uint8 btn, int32_t, int32_t)
{
	if (btn == SDL_BUTTON_RIGHT) {
		end_modal(0);
		return true;
	}
	return false;
}
bool Trigger_Building_Option_Menu::
	handle_mouserelease(const Uint8, int32_t, int32_t)
{
	return false;
}


///  Change the player number 1 step in any direction. Wraps around.
void Trigger_Building_Option_Menu::clicked_change_player(const bool up) {
	Widelands::Editor_Game_Base       & egbase     = eia().egbase();
	Widelands::Tribe_Descr      const & old_tribe  =
		egbase.manually_load_tribe(m_player_area.player_number);
	Widelands::Player_Number const nr_players = egbase.map().get_nrplayers();
	assert(1 < nr_players);
	assert(1 <= m_player_area.player_number);
	assert     (m_player_area.player_number <= nr_players);
	if (up) {
		if (m_player_area.player_number == nr_players)
			m_player_area.player_number = 0;
		++m_player_area.player_number;
	} else {
		--m_player_area.player_number;
		if (0 == m_player_area.player_number)
			m_player_area.player_number = nr_players;
	}
	Widelands::Tribe_Descr const & new_tribe =
		egbase.manually_load_tribe(m_player_area.player_number);
	if (&old_tribe != &new_tribe) {
		//  The new player belongs to another tribe than the old player. See if
		//  the new player's tribe has a building with the same name as the
		//  previously selected building. If not, select the first building in
		//  the new player's tribe.
		{
			m_building =
				new_tribe.building_index
					(old_tribe.get_building_descr(m_building)->name().c_str());
			if (not m_building)
				m_building = Widelands::Building_Index::First();
		}
		update_label_building
			(m_label_building, *new_tribe.get_building_descr(m_building));
		bool const has_several_buildings =
			1 < new_tribe.get_nrbuildings().value();
		m_decrement_building.set_enabled(has_several_buildings);
		m_increment_building.set_enabled(has_several_buildings);
	}
	update_label_player(m_label_player, m_player_area.player_number);
}


void Trigger_Building_Option_Menu::clicked_increment_building() {
	Widelands::Editor_Game_Base       & egbase = eia().egbase();
	Widelands::Tribe_Descr      const & tribe  =
		egbase.manually_load_tribe(m_player_area.player_number);
	++m_building;
	if (m_building == tribe.get_nrbuildings())
		m_building = Widelands::Building_Index::First();
	update_label_building
		(m_label_building, *tribe.get_building_descr(m_building));
}


void Trigger_Building_Option_Menu::clicked_decrement_building() {
	Widelands::Editor_Game_Base       & egbase = eia().egbase();
	Widelands::Tribe_Descr      const & tribe  =
		egbase.manually_load_tribe(m_player_area.player_number);
	if (m_building == Widelands::Building_Index::First())
		m_building = tribe.get_nrbuildings();
	--m_building;
	update_label_building
		(m_label_building, *tribe.get_building_descr(m_building));
}


void Trigger_Building_Option_Menu::clicked_increment_count() {
	assert
		(m_count
		 <
		 std::numeric_limits<Trigger_Building::Count_Type>::max());
	++m_count;
	update_label_count(m_label_count, m_count);
	m_decrement_count.set_enabled(true);
	m_increment_count.set_enabled
		(m_count < std::numeric_limits<Trigger_Building::Count_Type>::max());
}


void Trigger_Building_Option_Menu::clicked_decrement_count() {
	assert(1 < m_count);
	--m_count;
	update_label_count(m_label_count, m_count);
	m_decrement_count.set_enabled(1 < m_count);
	m_increment_count.set_enabled(true);
}


void Trigger_Building_Option_Menu::clicked_decrease_x_coordinate
	(uint8_t const d)
{
	Widelands::X_Coordinate const w = eia().egbase().map().extent().w;
	while (m_player_area.x < d) m_player_area.x += w;
	m_player_area.x -= d;
	update_label_coords(m_label_coords, m_player_area);
}


void Trigger_Building_Option_Menu::clicked_increase_x_coordinate
	(uint8_t const d)
{
	m_player_area.x += d;
	m_player_area.x %= eia().egbase().map().extent().w;
	update_label_coords(m_label_coords, m_player_area);
}


void Trigger_Building_Option_Menu::clicked_decrease_y_coordinate
	(uint8_t const d)
{
	Widelands::Y_Coordinate const h = eia().egbase().map().extent().h;
	while (m_player_area.y < d) m_player_area.y += h;
	m_player_area.y -= d;
	update_label_coords(m_label_coords, m_player_area);
}


void Trigger_Building_Option_Menu::clicked_increase_y_coordinate
	(uint8_t const d)
{
	m_player_area.y += d;
	m_player_area.y %= eia().egbase().map().extent().h;
	update_label_coords(m_label_coords, m_player_area);
}


void Trigger_Building_Option_Menu::clicked_increment_radius() {
	assert
		(m_player_area.radius
		 <
		 std::numeric_limits<Widelands::Player_Area<>::Radius_type>::max());
	++m_player_area.radius;
	update_label_radius(m_label_radius, m_player_area.radius);
	m_decrement_radius.set_enabled(true);
	m_increment_radius.set_enabled
		(m_player_area.radius
		 <
		 std::numeric_limits<Widelands::Player_Area<>::Radius_type>::max());
}


void Trigger_Building_Option_Menu::clicked_decrement_radius() {
	assert(0 < m_player_area.radius);
	--m_player_area.radius;
	update_label_radius(m_label_radius, m_player_area.radius);
	m_decrement_radius.set_enabled(m_player_area.radius);
	m_increment_radius.set_enabled(true);
}


void Trigger_Building_Option_Menu::clicked_ok() {
	std::string const & name = m_name.text();
	if (name.size()) {
		if
			(Widelands::Trigger * const registered_trigger =
			 eia().egbase().map().mtm()[name])
			if (registered_trigger != & m_trigger) {
				char buffer[256];
				snprintf
					(buffer, sizeof(buffer),
					 _
					 	("There is another trigger registered with the name "
					 	 "\"%s\". Choose another name."),
					 name.c_str());
				UI::MessageBox mb
					(get_parent(),
					 _("Name in use"), buffer,
					 UI::MessageBox::OK);
				mb.run();
				return;
			}
		m_trigger.set_name(name);
	}
	Widelands::Player_Number const trigger_player_number =
		m_trigger.m_player_area.player_number;
	if (trigger_player_number != m_player_area.player_number) {
		if (trigger_player_number)
			eia().unreference_player_tribe(trigger_player_number, &m_trigger);
		eia().reference_player_tribe(m_player_area.player_number, &m_trigger);
	}
	Widelands::Editor_Game_Base & egbase = eia().egbase();
	m_trigger.m_player_area =
		Widelands::Player_Area<Widelands::Area<Widelands::FCoords> >
			(m_player_area.player_number,
			 Widelands::Area<Widelands::FCoords>
			 	(Widelands::FCoords(m_player_area, &egbase.map()[m_player_area]),
			 	 m_player_area.radius));
	m_trigger.m_building_type =
		egbase.manually_load_tribe(m_player_area.player_number)
		.get_building_descr(m_building);
	m_trigger.m_count = m_count;
	eia().set_need_save(true);
	end_modal(1);
}
