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

#ifndef EVENT_MESSAGE_BOX_OPTION_MENU_H
#define EVENT_MESSAGE_BOX_OPTION_MENU_H

#include "ui_basic/window.h"

#include <vector>


struct Editor_Interactive;
namespace Widelands {struct Event_Message_Box;}
namespace UI {
template <typename T> struct Callback_Button;
struct EditBox;
struct Checkbox;
template <typename T> struct Listselect;
struct Multiline_Editbox;
struct Textarea;
};

/**
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Event_Message_Box_Option_Menu : public UI::Window {
	Event_Message_Box_Option_Menu
		(Editor_Interactive &, Widelands::Event_Message_Box &);

	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	static const uint32_t MAX_BUTTONS = 4;
	Editor_Interactive & eia();

	void update();
	void clicked_ok                        ();
	void clicked_number_of_buttons_decrease();
	void clicked_number_of_buttons_increase();
	void clicked_trigger_sel_decrease      ();
	void clicked_trigger_sel_increase      ();
	void ls_selected    (uint32_t);
	void edit_box_edited(int32_t);

	Widelands::Event_Message_Box & m_event;

	struct Button_Descr {
		std::string name;
		int32_t trigger;
	} m_buttons[MAX_BUTTONS];

	UI::Checkbox           * m_is_modal;
	UI::EditBox            * m_caption;
	UI::EditBox            * m_name;
	UI::EditBox            * m_window_title;
	UI::EditBox            * m_button_name;
	UI::Textarea           * m_nr_buttons_ta;
	uint32_t                 m_nr_buttons;
	uint32_t                 m_ls_selected;
	UI::Multiline_Editbox  * m_text;
	UI::Textarea           * m_current_trigger_ta;
	UI::Listselect<void *> * m_buttons_ls;

	std::vector<int32_t> m_button_triggers;
};

#endif
