/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__FONT_HANDLER_H
#define __S__FONT_HANDLER_H

#include <vector>
#include <map>
#include "font.h"

class RenderTarget;

enum Align {
	Align_Left = 0,
	Align_HCenter = 1,
	Align_Right = 2,
	Align_Horizontal = 3,

	Align_Top = 0,
	Align_VCenter = 4,
	Align_Bottom = 8,
	Align_Vertical = 12,

	Align_TopLeft = 0,
	Align_CenterLeft = Align_VCenter,
	Align_BottomLeft = Align_Bottom,

	Align_TopCenter = Align_HCenter,
	Align_Center = Align_HCenter|Align_VCenter,
	Align_BottomCenter = Align_HCenter|Align_Bottom,

	Align_TopRight = Align_Right,
	Align_CenterRight = Align_Right|Align_VCenter,
	Align_BottomRight = Align_Right|Align_Bottom,
};

   
/** class Font_Handler
 *
 * This class generates font Pictures out of strings and returns them
 */
class Font_Handler {
public:
	Font_Handler();
   ~Font_Handler();
   
   void reload_all();
	void load_font(std::string name, int size, RGBColor fg, RGBColor bg);
   void unload_font(std::string name, int size = 0);

	int calc_linewidth(std::string font, int size, const char* string, int wrap, const char** nextline);
	void draw_string(RenderTarget* dst, std::string font, int size, RGBColor fg, RGBColor bg, int x, int y, const char* string, Align align = Align_Left,
						  int wrap = -1, int mark_char = -1, int mark_value=-1);
	void get_size(std::string font, int size, const char* string, int* pw, int* ph, int wrap = -1);
	int get_fontheight(std::string font, int size);

private:
	std::vector<Font*>		m_fonts;		// map of all fonts
   
   Font* find_correct_font(std::string name, int size);
   Font* find_correct_font(std::string name, int size, RGBColor fg, RGBColor bg);
   
};

extern Font_Handler* g_fh;	// the default font

#endif

