/*
 * Copyright (C) 2002 by Widelands Development Team
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

#ifndef __S__MULTILINE_EDITBOX_H
#define __S__MULTILINE_EDITBOX_H

#include <string>
#include "font_handler.h"
#include "ui_multilinetextarea.h"
#include "ui_signal.h"

class UIScrollbar;

/*
 * This behaves like a editbox, but looks like
 * a Multiline Textarea
 *
 * Shitf + del or Shift + backspace deletes all text
 */
class UIMultiline_Editbox : public UIMultiline_Textarea {
   public:
      UIMultiline_Editbox(UIPanel *parent, int x, int y, uint w, uint h, const char *text);
      ~UIMultiline_Editbox();

      // Changed event
      UISignal changed;

      // Drawing and event handlers
      void draw(RenderTarget* dst);
      void set_maximum_chars(int n) { m_maxchars=n; }
      int get_maximum_chars(void) { return m_maxchars; }

      // Event functions
      bool handle_mouseclick(uint btn, bool down, int x, int y);
      bool handle_key(bool down, int code, char c);
      void set_text(const char* str);

   private:
	   static const int ms_darken_value=-20;
      uint m_cur_pos;
      uint m_char_pos;
      uint m_line_pos;
      uint m_maxchars;
      bool m_needs_update;
};

#endif // __S__MULTILINE_EDITBOX_H
