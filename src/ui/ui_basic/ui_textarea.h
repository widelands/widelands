/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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

#ifndef __S__TEXTAREA_H
#define __S__TEXTAREA_H

#include <string>
#include "font_handler.h"
#include "ui_panel.h"

namespace UI {
/**
 * This defines a non responsive (to clicks) text area, where a text
 * can easily be printed
 */
struct Textarea : public Panel {
	Textarea(Panel *parent, int x, int y, std::string text, Align align = Align_Left);
	Textarea(Panel *parent, int x, int y, int w, int h, std::string text,
			   Align align = Align_Left, bool multiline = false);
	~Textarea();

	void set_text(std::string text);
	void set_align(Align align);

	// Drawing and event handlers
	void draw(RenderTarget* dst);

   inline void set_font(std::string name, int size, RGBColor fg) { m_fontname=name; m_fontsize=size; m_fcolor=fg; set_text(m_text.c_str()); }

private:
	void collapse();
	void expand();

	std::string m_text;
	Align       m_align;
	bool        m_multiline;
   std::string    m_fontname;
   int            m_fontsize;
   RGBColor       m_fcolor;
};
};

#endif // __S__TEXTAREA_H
