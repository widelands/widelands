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

////////////////////////////////////////////////////////////////////////////////////////7

/** class Textarea
 *
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 *
 * Depends: class Graph::Pic
 * 			class Font_Handler
 */

/*
===============
Textarea::Textarea

Initialize a Textarea. The dimension are set automatically, depending on the text.

Args: parent	parent panel
      x			coordinates of the textarea
      y
      text		text on the Textarea (can be 0)
      align		alignment for the text
===============
*/
Textarea::Textarea(Panel *parent, int x, int y, const char *text, Align align)
	: Panel(parent, x, y, 0, 0)
{
	set_handle_mouse(false);
	set_think(false);

	m_align = align;
	
	if (text)
		set_text(text);
}

/*
===============
Textarea::~Textarea

Free allocated resources
===============
*/
Textarea::~Textarea()
{
}


/*
===============
Textarea::set_text

Set the text of the Textarea. Size is automatically adjusted
===============
*/
void Textarea::set_text(const char *text)
{
	collapse(); // collapse() implicitly updates
	
	if (text) {
		m_text = text;
		expand();
	} else
		m_text = "";
}


/*
===============
Textarea::set_align

Change the alignment
===============
*/
void Textarea::set_align(Align align)
{
	collapse();
	m_align = align;
	expand();
}


/*
===============
Textarea::draw

Redraw the Textarea
===============
*/
void Textarea::draw(Bitmap *dst, int ofsx, int ofsy)
{
	if (m_text.length())
		{
		int x = ofsx;
		int y = ofsy;
		
		if (m_align & Align_HCenter)
			x += get_w()/2;
		else if (m_align & Align_Right)
			x += get_w();
		
		if (m_align & Align_VCenter)
			y += get_h()/2;
		else if (m_align & Align_Bottom)
			y += get_h();
		
		g_fh.draw_string(dst, x, y, m_text.c_str(), m_align);
		}
}


/*
===============
Textarea::collapse

Reduce the Textarea to size 0x0 without messing up the alignment
===============
*/
void Textarea::collapse()
{
	int x = get_x();
	int y = get_y();
	int w = get_w();
	int h = get_h();

	if (m_align & Align_HCenter)
		x += w >> 1;
	else if (m_align & Align_Right)
		x += w;

	if (m_align & Align_VCenter)
		y += h >> 1;
	else if (m_align & Align_Bottom)
		y += h;

	set_pos(x, y);
	set_size(0, 0);
}


/*
===============
Textarea::expand

Expand the size of the Textarea until it fits the size of the text
===============
*/
void Textarea::expand()
{
	if (!m_text.length())
		return;

	int x = get_x();
	int y = get_y();
	int w, h;
	 
	g_fh.get_size(m_text.c_str(), &w, &h);
	
	if (m_align & Align_HCenter)
		x -= w >> 1;
	else if (m_align & Align_Right)
		x -= w;

	if (m_align & Align_VCenter)
		y -= h >> 1;
	else if (m_align & Align_Bottom)
		y -= h;

	set_pos(x, y);
	set_size(w, h);
}

