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
#include "ui_basic.h"

/**
Initialize a UITextarea. For non-multiline textareas, the dimensions are set
automatically, depending on the text.
For multiline textareas, only the height and vertical position is adjusted
automatically. A multiline UITextarea differs from a UIMultiline_Textarea in that
Multiline_Textarea provides scrollbars.
*/
UITextarea::UITextarea(UIPanel *parent, int x, int y, std::string text, Align align)
	: UIPanel(parent, x, y, 0, 0)
{
	set_handle_mouse(false);
	set_think(false);

	m_align = align;
	m_multiline = false;

	set_text(text);
}

UITextarea::UITextarea(UIPanel *parent, int x, int y, int w, int h, std::string text, Align align, bool multiline)
	: UIPanel(parent, x, y, w, h)
{
	set_handle_mouse(false);
	set_think(false);

	m_align = align;
	m_multiline = multiline;

	set_text(text);
}


/**
Free allocated resources
*/
UITextarea::~UITextarea()
{
}


/**
Set the text of the UITextarea. Size is automatically adjusted
*/
void UITextarea::set_text(std::string text)
{
	collapse(); // collapse() implicitly updates

	m_text = text;
	expand();
}


/**
Change the alignment
*/
void UITextarea::set_align(Align align)
{
	collapse();
	m_align = align;
	expand();
}


/**
Redraw the UITextarea
*/
void UITextarea::draw(RenderTarget* dst)
{
	if (m_text.length())
		{
		int x = 0;
		int y = 0;

		if (m_align & Align_HCenter)
			x += get_w()/2;
		else if (m_align & Align_Right)
			x += get_w();

		if (m_align & Align_VCenter)
			y += get_h()/2;
		else if (m_align & Align_Bottom)
			y += get_h();

		g_font->draw_string(dst, x, y, m_text.c_str(), m_align, m_multiline ? get_w() : -1);
		}
}


/**
Reduce the UITextarea to size 0x0 without messing up the alignment
*/
void UITextarea::collapse()
{
	int x = get_x();
	int y = get_y();
	int w = get_w();
	int h = get_h();

	if (!m_multiline)
	{
		if (m_align & Align_HCenter)
			x += w >> 1;
		else if (m_align & Align_Right)
			x += w;
	}

	if (m_align & Align_VCenter)
		y += h >> 1;
	else if (m_align & Align_Bottom)
		y += h;

	set_pos(x, y);
	set_size(m_multiline ? get_w() : 0, 0);
}


/**
Expand the size of the UITextarea until it fits the size of the text
*/
void UITextarea::expand()
{
	if (!m_text.length())
		return;

	int x = get_x();
	int y = get_y();
	int w, h;

	g_font->get_size(m_text.c_str(), &w, &h, m_multiline ? get_w() : -1);

	if (!m_multiline)
	{
		if (m_align & Align_HCenter)
			x -= w >> 1;
		else if (m_align & Align_Right)
			x -= w;
	}

	if (m_align & Align_VCenter)
		y -= h >> 1;
	else if (m_align & Align_Bottom)
		y -= h;

	set_pos(x, y);
	set_size(m_multiline ? get_w() : w, h);
}

