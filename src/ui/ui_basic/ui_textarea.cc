/*
 * Copyright (C) 2002, 2006-2007 by the Widelands Development Team
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

#include "ui_textarea.h"
#include "constants.h"
#include "graphic.h"

namespace UI {
/**
  Initialize a Textarea. For non-multiline textareas, the dimensions are set
  automatically, depending on the text.
  For multiline textareas, only the height and vertical position is adjusted
  automatically. A multiline Textarea differs from a Multiline_Textarea in that
  Multiline_Textarea provides scrollbars.
  */
   Textarea::Textarea(Panel *parent, int x, int y, std::string text, Align align)
: Panel(parent, x, y, 0, 0)
{
   set_handle_mouse(false);
   set_think(false);

   m_align = align;
   m_multiline = false;

   set_font(UI_FONT_SMALL, UI_FONT_CLR_FG);

   set_text(text);
}

   Textarea::Textarea(Panel *parent, int x, int y, int w, int h, std::string text, Align align, bool multiline)
: Panel(parent, x, y, w, h)
{
   set_handle_mouse(false);
   set_think(false);

   m_align = align;
   m_multiline = multiline;
   set_font(UI_FONT_SMALL, UI_FONT_CLR_FG);

   set_text(text);
}


/**
  Free allocated resources
  */
Textarea::~Textarea()
{
}


/**
  Set the text of the Textarea. Size is automatically adjusted
  */
void Textarea::set_text(std::string text)
{
   collapse(); // collapse() implicitly updates

   m_text = text;
   expand();
}


/**
  Change the alignment
  */
void Textarea::set_align(Align align)
{
   collapse();
   m_align = align;
   expand();
}


/**
  Redraw the Textarea
  */
void Textarea::draw(RenderTarget* dst)
{
	if (m_text.length()) g_fh->draw_string
		(*dst,
		 m_fontname, m_fontsize, m_fcolor, RGBColor(107, 87, 55),
		 Point
		 (m_align & Align_HCenter ?
		  get_w() / 2 : m_align & Align_Right  ? get_w() : 0,
		  m_align & Align_VCenter ?
		  get_h() / 2 : m_align & Align_Bottom ? get_h() : 0),
		 m_text.c_str(),
		 m_align,
		 m_multiline ? get_w() : -1);
}


/**
  Reduce the Textarea to size 0x0 without messing up the alignment
  */
void Textarea::collapse()
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
  Expand the size of the Textarea until it fits the size of the text
  */
void Textarea::expand()
{
   if (!m_text.length())
      return;

   int x = get_x();
   int y = get_y();
   int w, h;

   g_fh->get_size(m_fontname, m_fontsize, m_text.c_str(), &w, &h, m_multiline ? get_w() : -1);

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
};
