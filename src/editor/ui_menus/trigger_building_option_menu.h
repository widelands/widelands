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

#ifndef TRIGGER_BUILDING_OPTION_MENU_H
#define TRIGGER_BUILDING_OPTION_MENU_H

#include "player_area.h"
#include "building.h"
#include "trigger/trigger_building.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_textarea.h"
#include "ui_window.h"

#include <vector>

struct Editor_Interactive;
namespace UI {
struct EditBox;
struct Textarea;
};

/**
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Trigger_Building_Option_Menu : public UI::Window {
	Trigger_Building_Option_Menu
		(Editor_Interactive &, Widelands::Trigger_Building &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
	void clicked_ok();
	void clicked_decrease_x_coordinate(uint8_t);
	void clicked_increase_x_coordinate(uint8_t);
	void clicked_decrease_y_coordinate(uint8_t);
	void clicked_increase_y_coordinate(uint8_t);
	void clicked_change_player        (bool up);
	void clicked_decrement_building   ();
	void clicked_increment_building   ();
	void clicked_decrement_count      ();
	void clicked_increment_count      ();
	void clicked_decrement_radius     ();
	void clicked_increment_radius     ();

	Widelands::Trigger_Building &                       m_trigger;
	Widelands::Player_Area<>                            m_player_area;
	Widelands::Trigger_Building::Count_Type             m_count;
	Widelands::Building_Index                           m_building;
	UI::Textarea                                        m_label_name;
	UI::EditBox                                         m_name;
	UI::Textarea                                        m_label_player;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, bool> m_decrement_player;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, bool> m_increment_player;
	UI::Textarea                                        m_label_building;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_decrement_building;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_increment_building;
	UI::Textarea                                        m_label_count;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_decrement_count;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_increment_count;
	UI::Textarea                                        m_label_coords;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_decrease_y_100;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_decrease_y_10;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_decrease_y_1;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_decrease_x_100;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_decrease_x_10;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_decrease_x_1;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_increase_x_1;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_increase_x_10;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_increase_x_100;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_increase_y_1;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_increase_y_10;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, uint8_t>
		m_increase_y_100;
	UI::Textarea                                        m_label_radius;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_decrement_radius;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_increment_radius;
	UI::Callback_Button<Trigger_Building_Option_Menu>   m_button_ok;
	UI::Callback_IDButton<Trigger_Building_Option_Menu, int32_t> m_button_cancel;
};

#endif
