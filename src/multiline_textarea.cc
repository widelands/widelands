/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "ui.h"
#include "font.h"

////////////////////////////////////////////////////////////////////////////////////////7

/** class Multiline_textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed.
 * The textarea transparently handles explicit line-breaks and word wrapping.
 *
 * Do not use it blindly for big texts: the font handler needs to re-break the 
 * entire text whenever the textarea is drawn, this is a trade-off which greatly
 * simplifies this class.
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */

/*
===============
Multiline_Textarea::Multiline_Textarea

Initialize a textarea that supports multiline strings.

Args: parent	parent panel
      x		coordinates of the textarea
      y
      w		size of the textarea
      h
      text	text for the textarea (can be 0)
      align	text alignment
===============
*/
Multiline_Textarea::Multiline_Textarea(Panel *parent, int x, int y, uint w, uint h,
                                       const char *text, Align align)
	: Panel(parent, x, y, w-24, h)
{
	set_handle_mouse(false);
	set_think(false);

	set_align(align);

	m_textpos = 0;
	m_textheight = 0;
	if (text)
		set_text(text);

	Scrollbar *sb = new Scrollbar(parent, x+get_w(), y, 24, h, false);
	sb->up.set(this, &Multiline_Textarea::move_up);
	sb->down.set(this, &Multiline_Textarea::move_down);
}


/*
===============
Multiline_Textarea::~Multiline_Textarea

Free allocated resources
===============
*/
Multiline_Textarea::~Multiline_Textarea()
{
}


/*
===============
Multiline_Textarea::set_text

Replace the current text with a new one.
Fix up scrolling state if necessary.
===============
*/
void Multiline_Textarea::set_text(const char *text)
{
	if (!text)
		{
		// Clear the field
		m_text = "";
		
		m_textheight = 0;
		m_textpos = 0;
		}
	else
		{
		m_text = text;
	
		g_font->get_size(text, 0, &m_textheight, get_eff_w());
	
		if (m_textpos > m_textheight - get_h())
			m_textpos = m_textheight - get_h();
		if (m_textpos < 0)
			m_textpos = 0;
		}

	update(0, 0, get_eff_w(), get_h());
}


/*
===============
Multiline_Textarea::set_align

Change alignment of the textarea
===============
*/
void Multiline_Textarea::set_align(Align align)
{
	// don't allow vertical alignment as it doesn't make sense
	m_align = (Align)(align & Align_Horizontal);
}


/*
===============
Multiline_Textarea::move_up

Scroll the area up i lines
===============
*/
void Multiline_Textarea::move_up(int i)
{
	int delta = i * g_font->get_fontheight();
	
	if (delta > m_textpos)
		delta = m_textpos;
	if (delta <= 0)
		return;

	m_textpos -= delta;

	update(0, 0, get_eff_w(), get_h());
}


/*
===============
Multiline_Textarea::move_down

Scroll down i lines
===============
*/
void Multiline_Textarea::move_down(int i)
{
	int delta = i * g_font->get_fontheight();
	
	if (m_textpos + delta > m_textheight - get_h())
		delta = m_textheight - get_h() - m_textpos;
	if (delta <= 0)
		return;

	m_textpos += delta;
	update(0, 0, get_eff_w(), get_h());
}


/*
===============
Multiline_Textarea::draw

Redraw the textarea
===============
*/
void Multiline_Textarea::draw(RenderTarget* dst)
{
	// Let the font handler worry about all the complicated stuff..
	g_font->draw_string(dst, 0, 0 - m_textpos, m_text.c_str(), m_align, get_eff_w());
}

