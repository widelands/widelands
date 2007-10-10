/*
 * Copyright (C) 2003, 2006-2007 by the Widelands Development Team
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

#ifndef included_ui_editbox_h
#define included_ui_editbox_h

#include <SDL_keyboard.h>
#include <stdint.h>
#include <string>
#include "ui_button.h"

namespace UI {
/**
a editbox can be clicked, then the user can change
it's text (title). When return is pressed, the
editbox is unfocused, the keyboard released and
a callback function is called
*/
struct Edit_Box : private Basic_Button {
     Edit_Box(Panel *parent, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t background, int32_t id);
     virtual ~Edit_Box();

     Signal changed;
     Signal1<int32_t> changedid;

	const char* get_text() {return m_text.c_str();}
     void set_text(const char* text) {m_lasttext=m_text=text; set_title(text);}
     void set_maximum_chars(int32_t n) {m_maxchars=n;}
	int32_t get_maximum_chars() const {return m_maxchars;}

	bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);
	bool handle_mousemove(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff);
     bool handle_key(bool down, SDL_keysym code);
     void handle_mousein(bool);

protected:
	void send_signal_clicked() const {};

private:
     bool m_keyboard_grabbed;
     uint32_t m_maxchars;
     std::string m_text, m_lasttext;
     int32_t m_id;
};
};

#endif // included_ui_editbox_h
