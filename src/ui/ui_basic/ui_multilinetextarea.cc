/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include "font.h"
#include "types.h"
#include "ui_multilinetextarea.h"
#include "ui_scrollbar.h"

/**
Initialize a textarea that supports multiline strings.
*/
UIMultiline_Textarea::UIMultiline_Textarea(UIPanel *parent, int x, int y, uint w, uint h,
                                       const char *text, Align align)
	: UIPanel(parent, x, y, w - 24, h)
{
	set_handle_mouse(false);
	set_think(false);

	set_align(align);

	m_textpos = 0;
	m_textheight = 0;
	m_scrollmode = ScrollNormal;

	m_scrollbar = new UIScrollbar(parent, x+get_w(), y, 24, h, false);
	m_scrollbar->moved.set(this, &UIMultiline_Textarea::set_scrollpos);

	m_scrollbar->set_pagesize(h - 2*g_font->get_fontheight());
	m_scrollbar->set_steps(1);

	if (text)
		set_text(text);
}


/**
Free allocated resources
*/
UIMultiline_Textarea::~UIMultiline_Textarea()
{
}


/**
Replace the current text with a new one.
Fix up scrolling state if necessary.
*/
void UIMultiline_Textarea::set_text(const char *text)
{
	if (!text)
		{
		// Clear the field
		m_text = "";

		m_textheight = 0;
		m_textpos = 0;
		m_scrollbar->set_steps(1);
		}
	else
		{
		bool setbottom = false;

		if (m_scrollmode == ScrollLog) {
			if (m_textpos >= m_textheight - get_h() - g_font->get_fontheight())
				setbottom = true;
		}

		m_text = text;

		g_font->get_size(text, 0, &m_textheight, get_eff_w());

		if (setbottom || m_textpos > m_textheight - get_h())
			m_textpos = m_textheight - get_h();
		if (m_textpos < 0)
			m_textpos = 0;

		m_scrollbar->set_steps(m_textheight - get_h());
		m_scrollbar->set_pos(m_textpos);
		}

	update(0, 0, get_eff_w(), get_h());
}


/**
Change alignment of the textarea
*/
void UIMultiline_Textarea::set_align(Align align)
{
	// don't allow vertical alignment as it doesn't make sense
	m_align = (Align)(align & Align_Horizontal);
}


/**
Scroll to the given position.
*/
void UIMultiline_Textarea::set_scrollpos(int pixels)
{
	m_textpos = pixels;

	update(0, 0, get_eff_w(), get_h());
}


/**
Change the scroll mode. This will not change the current scroll position;
it only affects the behaviour of set_text().
*/
void UIMultiline_Textarea::set_scrollmode(ScrollMode mode)
{
	m_scrollmode = mode;
}


/**
Redraw the textarea
*/
void UIMultiline_Textarea::draw(RenderTarget* dst)
{
   if (m_text.length())
   {
      int x = 0;

      // Only HAlignment allowed
      if (m_align & Align_HCenter)
         x += get_w()/2;
      else if (m_align & Align_Right)
         x += get_w();

      // Let the font handler worry about all the complicated stuff..
      g_font->draw_string(dst, x, 0 - m_textpos, m_text.c_str(), m_align, get_eff_w());
   }
}

