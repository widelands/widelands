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

#ifndef EVENT_MESSAGEBOX_H
#define EVENT_MESSAGEBOX_H

#include "event.h"

#include <vector>

namespace UI {struct Panel;};

namespace Widelands {

struct Trigger;
struct Trigger_Null;
struct Editor_Game_Base;

/**
 * This event shows a message box
 */
struct Event_Message_Box : public Event, public Referencer<Trigger> {
	Event_Message_Box(char const * const Name, State);
	~Event_Message_Box();

	std::string identifier() const {return "Event (message box): " + name();}

	int32_t option_menu(Editor_Interactive &);

	State run(Game *);

	void Read (Section &, Editor_Game_Base       &);
	void Write(Section &, Editor_Game_Base const &) const;

	void set_text(const char * str) {m_text = str;}
	const char * get_text() const {return m_text.c_str();}
	void set_window_title(const char* str) {m_window_title = str;}
	const char * get_window_title() const {return m_window_title.c_str();}
	void set_is_modal(bool t) {m_is_modal = t;}
	bool get_is_modal() const {return m_is_modal;}
	void set_pos(int32_t posx, int32_t posy) {m_posx=posx; m_posy=posy;}
	int32_t get_posx() const {return m_posx;}
	int32_t get_posy() const {return m_posy;}
	void set_dimensions(int32_t w, int32_t h) {m_width = w; m_height = h;}
	int32_t get_w() const {return m_width;}
	int32_t get_h() const {return m_height;}
	void set_button_trigger(uint8_t button_number, Trigger_Null *);
	Trigger_Null * get_button_trigger(int32_t);
	void set_button_name(int32_t, std::string);
	char const * get_button_name(int32_t);
	void set_nr_buttons(int32_t);
	int32_t get_nr_buttons() const {return m_buttons.size();}

	enum {
		Right = 0,
		Left,
		Center_under,
		Center_over,
	};

private:
	struct Button_Descr {
		std::string    name;
		Trigger_Null * trigger;
	};

	std::string m_text;
	std::string m_window_title;
	bool m_is_modal;

	std::vector<Button_Descr> m_buttons;
	UI::Panel               * m_window;
	int32_t m_posx, m_posy;
	int32_t m_width, m_height;
};

};

#endif
