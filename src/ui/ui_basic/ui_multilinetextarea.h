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

#ifndef __S__MULTILINE_TEXTAREA_H
#define __S__MULTILINE_TEXTAREA_H

#include <string>
#include "font.h"
#include "ui_panel.h"

class UIScrollbar;

/**
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed.
 * The textarea transparently handles explicit line-breaks and word wrapping.
 *
 * Do not use it blindly for big texts: the font handler needs to re-break the
 * entire text whenever the textarea is drawn, this is a trade-off which greatly
 * simplifies this class.
 */
class UIMultiline_Textarea : public UIPanel {
   public:
      enum ScrollMode {
         ScrollNormal = 0,    ///< (default) only explicit or forced scrolling
         ScrollLog = 1,       ///< follow the bottom of the text
      };

   public:
      UIMultiline_Textarea(UIPanel *parent, int x, int y, uint w, uint h, const char *text,
            Align align = Align_Left);
      ~UIMultiline_Textarea();

      std::string get_text() const { return m_text; }
      ScrollMode get_scrollmode() const { return m_scrollmode; }

      void set_text(const char *text);
      void set_align(Align align);
      void set_scrollpos(int pixels);
      void set_scrollmode(ScrollMode mode);

      inline uint get_eff_w() { return get_w(); }

      // Drawing and event handlers
      void draw(RenderTarget* dst);

   private:
	Align				m_align;
	std::string		m_text;
   UIScrollbar*		m_scrollbar;
	ScrollMode     m_scrollmode;
   int				m_textheight;	///< total height of wrapped text, in pixels
	int				m_textpos;		///< current scrolling position in pixels (0 is top)
};

#endif // __S__MULTILINE_TEXTAREA_H
