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

#ifndef __S__FONT_H
#define __S__FONT_H

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


/** class Font
 *
 * This class generates font Pictures out of strings and returns them
 */
class Font {
public:
	static void reload_all();
	static Font* load(const char* name);
	
	void addref();
	void release();

	int calc_linewidth(const char* string, int wrap, const char** nextline);
	void draw_string(RenderTarget* dst, int x, int y, const char* string, Align align = Align_Left,
						  int wrap = -1);
	void get_size(const char* string, int* pw, int* ph, int wrap = -1);
	int get_fontheight();
	
private:
	Font(const char* name);
	~Font();
	
	void do_load();
	
	struct Char {
		int	width;
		uint	pic;
	};
	
	int	m_refs;
	char*	m_name;
	int	m_height;			// height of the font
	Char	m_pictures[96];
	
	static std::map<const char*, Font*>		s_fonts;		// map of all fonts
};

extern Font* g_font;	// the default font

#endif /* __S__FONT_H */
