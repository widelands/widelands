/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "textarea.h"

#include "font_handler.h"

namespace UI {

Textarea::Textarea
	(Panel * const parent,
	 const int32_t x, const int32_t y,
	 const std::string & text, const Align align, const bool multiline)
	:
		Panel      (parent, x, y, 0, 0),
		m_text     (text),
		m_align    (align),
		m_multiline(multiline)
{
	set_handle_mouse(false);
	set_think       (false);
	set_font        (UI_FONT_SMALL, UI_FONT_CLR_FG);
}

Textarea::Textarea
	(Panel *  const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const Align align, const bool multiline)
	:
		Panel      (parent, x, y, w, h),
		m_align    (align),
		m_multiline(multiline),
		m_fontname (UI_FONT_NAME),
		m_fontsize (UI_FONT_SIZE_SMALL),
		m_fcolor   (UI_FONT_CLR_FG)
{
	set_handle_mouse(false);
	set_think(false);
}

Textarea:: Textarea
	(Panel * const parent,
	 const int32_t x, const int32_t y, const uint32_t w, const uint32_t h,
	 const std::string & text, const Align align,
	 const bool multiline)
	:
		Panel      (parent, x, y, w, h),
		m_align    (align),
		m_multiline(multiline),
		m_fontname (UI_FONT_NAME),
		m_fontsize (UI_FONT_SIZE_SMALL),
		m_fcolor   (UI_FONT_CLR_FG)
{
	set_handle_mouse(false);
	set_think(false);
	set_text(text);
}

/**
 * Set the text of the Textarea. Size is automatically adjusted
 */
void Textarea::set_text(const std::string & text) {
	collapse(); // collapse() implicitly updates

	m_text = text;
	expand();
}

std::string Textarea::get_text() {
	return m_text;
}


/**
 * Change the alignment
 */
void Textarea::set_align(const Align align) {
	collapse();
	m_align = align;
	expand();
}


/**
 * Redraw the Textarea
 */
void Textarea::draw(RenderTarget & dst)
{
	if (m_text.length())
		UI::g_fh->draw_string
			(dst,
			 m_fontname, m_fontsize, m_fcolor, UI_FONT_CLR_BG,
			 Point
			 	(m_align & Align_HCenter ?
			 	 get_w() / 2 : m_align & Align_Right  ? get_w() : 0,
			 	 m_align & Align_VCenter ?
			 	 get_h() / 2 : m_align & Align_Bottom ? get_h() : 0),
			 m_text,
			 m_align,
			 m_multiline ? get_w() : std::numeric_limits<uint32_t>::max());
}


/**
 * Reduce the Textarea to size 0x0 without messing up the alignment
 */
void Textarea::collapse()
{
	int32_t x = get_x();
	int32_t y = get_y();
	int32_t w = get_w();
	int32_t h = get_h();

	if (not m_multiline) {
		if      (m_align & Align_HCenter)
			x += w >> 1;
		else if (m_align & Align_Right)
			x += w;
	}

	if      (m_align & Align_VCenter)
		y += h >> 1;
	else if (m_align & Align_Bottom)
		y += h;

	set_pos(Point(x, y));
	set_size(m_multiline ? get_w() : 0, 0);
}


/**
 * Expand the size of the Textarea until it fits the size of the text
 */
void Textarea::expand()
{
	if (!m_text.length())
		return;

	int32_t x = get_x();
	int32_t y = get_y();
	uint32_t w, h;

	UI::g_fh->get_size
		(m_fontname,
		 m_fontsize,
		 m_text,
		 w, h,
		 m_multiline ? get_w() : -1);

	if (not m_multiline) {
		if      (m_align & Align_HCenter)
			x -= w >> 1;
		else if (m_align & Align_Right)
			x -= w;
	}

	if      (m_align & Align_VCenter)
		y -= h >> 1;
	else if (m_align & Align_Bottom)
		y -= h;

	set_pos(Point(x, y));
	set_size(m_multiline ? get_w() : w, h);
}

}
