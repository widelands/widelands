/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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

#ifndef FONT_H
#define FONT_H

#include <boost/shared_ptr.hpp>

#include <SDL_ttf.h>

#include "io/fileread.h"
#include "rgbcolor.h"

struct f;
namespace UI {

/**
 * Margin around text that is kept to make space for the caret.
 */
#define LINE_MARGIN 1


/**
 * Wrapper object around a font.
 *
 * Fonts in our sense are defined by the general font shape (given by the font
 * name) and the size of the font.
 */
struct Font {
	static void shutdown();
	static Font * get(const std::string & name, int size);
	static Font * ui_big();
	static Font * ui_small();
	static Font * ui_ultrasmall();

	uint32_t height() const;

	TTF_Font * get_ttf_font() const {return m_font;}

private:
	Font(const std::string & name, int size);
	~Font();

	FileRead m_fontfile;
	TTF_Font * m_font;
};

/**
 * Text style combines font with other characteristics like color
 * and style (italics, bold).
 */
struct TextStyle {
	TextStyle() :
		font(0),
		fg(255, 255, 255),
		bg(0, 0, 0),
		bold(false),
		italics(false),
		underline(false)
	{}

	static TextStyle makebold(Font * font, RGBColor fg) {
		TextStyle ts;
		ts.font = font;
		ts.bold = true;
		ts.fg = fg;
		return ts;
	}

	static const TextStyle & ui_big();
	static const TextStyle & ui_small();
	static const TextStyle & ui_ultrasmall();
	uint32_t calc_bare_width(const std::string & text) const;

	Font * font;
	RGBColor fg;
	RGBColor bg; //TODO get rid of this
	bool bold : 1;
	bool italics : 1;
	bool underline : 1;

	bool operator== (const TextStyle & o) const {
		return
			font == o.font && fg == o.fg && bg == o.bg &&
			bold == o.bold && italics == o.italics && underline == o.underline;
	}
	bool operator!= (const TextStyle & o) const {
		return !(*this == o);
	}
};

} // namespace UI

#endif // FONT_H
