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

#ifndef __S__TRIGGER_BUILDING_OPTION_MENU_H
#define __S__TRIGGER_BUILDING_OPTION_MENU_H

#include "player_area.h"
#include "building.h"
#include "trigger/trigger_building.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_textarea.h"
#include "ui_window.h"

#include <vector>

class Editor_Interactive;
namespace UI {
struct Edit_Box;
struct Textarea;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Trigger_Building_Option_Menu : public UI::Window {
	Trigger_Building_Option_Menu(Editor_Interactive &, Trigger_Building &);

	bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
      void update();
	void clicked_ok();
	void clicked_decrease_x_coordinate(const uint8_t);
	void clicked_increase_x_coordinate(const uint8_t);
	void clicked_decrease_y_coordinate(const uint8_t);
	void clicked_increase_y_coordinate(const uint8_t);
	void clicked_change_player        (const bool up);
	void clicked_decrement_building   ();
	void clicked_increment_building   ();
	void clicked_decrement_count      ();
	void clicked_increment_count      ();
	void clicked_decrement_radius     ();
	void clicked_increment_radius     ();

	Trigger_Building &                                  m_trigger;
	Player_Area<>                                       m_player_area;
	Trigger_Building::Count_Type                        m_count;
	Building_Descr::Index                               m_building;
	UI::Textarea                                        m_label_name;
	UI::Edit_Box                                        m_name;
	UI::Textarea                                        m_label_player;
	UI::IDButton<Trigger_Building_Option_Menu, bool>    m_decrement_player;
	UI::IDButton<Trigger_Building_Option_Menu, bool>    m_increment_player;
	UI::Textarea                                        m_label_building;
	UI::Button<Trigger_Building_Option_Menu>            m_decrement_building;
	UI::Button<Trigger_Building_Option_Menu>            m_increment_building;
	UI::Textarea                                        m_label_count;
	UI::Button<Trigger_Building_Option_Menu>            m_decrement_count;
	UI::Button<Trigger_Building_Option_Menu>            m_increment_count;
	UI::Textarea                                        m_label_coords;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_decrease_y_100;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_decrease_y_10;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_decrease_y_1;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_decrease_x_100;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_decrease_x_10;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_decrease_x_1;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_increase_x_1;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_increase_x_10;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_increase_x_100;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_increase_y_1;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_increase_y_10;
	UI::IDButton<Trigger_Building_Option_Menu, uint8_t> m_increase_y_100;
	UI::Textarea                                        m_label_radius;
	UI::Button<Trigger_Building_Option_Menu>            m_decrement_radius;
	UI::Button<Trigger_Building_Option_Menu>            m_increment_radius;
	UI::Button<Trigger_Building_Option_Menu>            m_button_ok;
	UI::IDButton<Trigger_Building_Option_Menu, int32_t> m_button_cancel;
};

#endif
