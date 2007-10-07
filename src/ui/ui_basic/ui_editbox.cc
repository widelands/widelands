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

#include "ui_editbox.h"

#include <SDL_keysym.h>

namespace UI {
/*
=================================================

class Edit_Box

=================================================
*/

/**
constructor
*/
Edit_Box::Edit_Box
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const uint32_t background,
	 const int32_t id)
	:
	Basic_Button
	(parent,
	 x, y, w, h,
	 true, false,
	 background,
	 0,
	 "",
	 "")
{

   set_think(false);

   m_keyboard_grabbed=false;
   m_maxchars=0xffff;
   m_text="";
   m_lasttext="";

   m_id=id;

   set_handle_mouse(true);
   set_can_focus(true);
   set_draw_caret(true);
}

/**
destructor
*/
Edit_Box::~Edit_Box() {
   grab_mouse(false);
   set_can_focus(false);
}

/**
the mouse was clicked on this editbox
*/
bool Edit_Box::handle_mousepress(const Uint8 btn, int32_t, int32_t) {
	if (btn != SDL_BUTTON_LEFT) return false;

	if (not m_keyboard_grabbed) {
      set_can_focus(true);
      focus();
      m_keyboard_grabbed=true;
      m_lasttext=m_text;
      return true;
	}
	return m_keyboard_grabbed;
}
bool Edit_Box::handle_mouserelease(const Uint8 btn, int32_t, int32_t)
{return btn == SDL_BUTTON_LEFT and m_keyboard_grabbed;}

/**
a key event must be handled
*/
bool Edit_Box::handle_key(bool down, int32_t code, char c) {
	if (down) {
		switch (code) {
		case SDLK_ESCAPE:
            set_text(m_lasttext.c_str());
			Basic_Button::handle_mouserelease(0, 0, 0);
            set_can_focus(false);
            m_keyboard_grabbed=false;
            grab_mouse(false);
            return true;

		case SDLK_RETURN:
            m_lasttext=m_text;
			Basic_Button::handle_mouserelease(0, 0, 0);
            set_can_focus(false);
            m_keyboard_grabbed=false;
            grab_mouse(false);
            changed.call();
            changedid.call(m_id);
            return true;

		case SDLK_BACKSPACE:
			if (m_text.size()) {
               m_text.erase(m_text.end() - 1);
               set_title(m_text.c_str());
			}
            return true;

		case SDLK_DELETE:
            m_text.resize(0);
            set_title(m_text.c_str());
            return true;

		default:
            if (c && m_text.size()<m_maxchars) m_text+=c;
            set_title(m_text.c_str());
            return true;
		}
	}

   return false;
}

/**
handles the mousemove for this panel.
does not much, suppresses messages when the focus
is received
*/
bool Edit_Box::handle_mousemove
	(const Uint8 state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff)
{
	return m_keyboard_grabbed ?
		true : Basic_Button::handle_mousemove(state, x, y, xdiff, ydiff);
}

/**
Handles mouseins or rather mouse outs.
Hides a mouseout event from the underlying button
*/
void Edit_Box::handle_mousein(bool inside) {
   if (m_keyboard_grabbed) return;
   Basic_Button::handle_mousein(inside);
}
};
