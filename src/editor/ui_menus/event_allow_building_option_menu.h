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

#ifndef EVENT_ALLOW_BUILDING_OPTION_MENU_H
#define EVENT_ALLOW_BUILDING_OPTION_MENU_H

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_window.h"
#include "ui_textarea.h"

#include "widelands.h"

#include <string>
#include <cstring>
#include <vector>

class Editor_Interactive;
namespace Widelands {struct Event_Allow_Building;};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Event_Allow_Building_Option_Menu : public UI::Window {
	Event_Allow_Building_Option_Menu
		(Editor_Interactive &, Widelands::Event_Allow_Building &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	Editor_Interactive & eia();
	void clicked_ok();
	void clicked_change_player     (bool up);
	void clicked_decrement_building();
	void clicked_increment_building();

	Widelands::Event_Allow_Building & m_event;
	Widelands::Player_Number                                m_player;
	Widelands::Building_Index                               m_building;
	UI::Textarea                                            m_label_name;
	UI::EditBox                                             m_name;
	UI::Textarea                                            m_label_player;
	UI::IDButton<Event_Allow_Building_Option_Menu, bool>    m_decrement_player;
	UI::IDButton<Event_Allow_Building_Option_Menu, bool>    m_increment_player;
	UI::Textarea                                            m_label_building;
	UI::Button<Event_Allow_Building_Option_Menu>            m_decrement_building;
	UI::Button<Event_Allow_Building_Option_Menu>            m_increment_building;
	UI::Textarea                                            m_label_allow;
	UI::Checkbox                                            m_allow;
	UI::Button<Event_Allow_Building_Option_Menu>            m_button_ok;
	UI::IDButton<Event_Allow_Building_Option_Menu, int32_t> m_button_cancel;
};

#endif
