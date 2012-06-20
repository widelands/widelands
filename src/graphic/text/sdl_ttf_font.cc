/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#include <string>
#include <exception>

#include <SDL_ttf.h>
#include <boost/format.hpp>

#include "rt_errors.h"
#include "rt_render.h"
#include "sdl_ttf_font.h"

using namespace std;
using namespace boost;

namespace RT {

class SDLTTF_Font : public IFont {
public:
	virtual ~SDLTTF_Font();
	SDLTTF_Font(TTF_Font* ttf);

	void dimensions(string, int, uint32_t * w, uint32_t * h);
	virtual SDL_Surface * render(string, SDL_Color clr, int);
	uint32_t ascent() const;

private:
	void m_set_style(int);

	TTF_Font * m_font;
	int m_style;
};

SDLTTF_Font::SDLTTF_Font(TTF_Font * font) {
	m_font = font;
	m_style = TTF_STYLE_NORMAL;
}

SDLTTF_Font::~SDLTTF_Font() {
	TTF_CloseFont(m_font);
	m_font = 0;
}

void SDLTTF_Font::dimensions(string txt, int style, uint32_t * gw, uint32_t * gh) {
	m_set_style(style);

	int w, h;
	TTF_SizeUTF8(m_font, txt.c_str(), &w, &h);
	*gw = w; *gh = h;
}

SDL_Surface * SDLTTF_Font::render(string txt, SDL_Color clr, int style) {
	m_set_style(style);

	SDL_Surface * text_surface = TTF_RenderUTF8_Blended(m_font, txt.c_str(), clr);

	if (not text_surface)
		throw RenderError((format("Rendering '%s' gave the error: %s") % txt % TTF_GetError()).str());
	return text_surface;
}

uint32_t SDLTTF_Font::ascent() const {
	return TTF_FontAscent(m_font);
}

void SDLTTF_Font::m_set_style(int style) {
	int sdl_style = TTF_STYLE_NORMAL;
	if (style & BOLD) sdl_style |= TTF_STYLE_BOLD;
	if (style & ITALIC) sdl_style |= TTF_STYLE_ITALIC;
	if (style & UNDERLINE) sdl_style |= TTF_STYLE_UNDERLINE;

	// Remember the last style. This should avoid that SDL_TTF flushes its
	// glyphcache all too often
	if (sdl_style == m_style)
		return;
	m_style = sdl_style;
	TTF_SetFontStyle(m_font, sdl_style);
}

class SDLTTF_FontLoaderFromFile : public IFontLoader {
public:
	SDLTTF_FontLoaderFromFile();
	virtual ~SDLTTF_FontLoaderFromFile();
	virtual IFont * load(std::string face, int ptsize);
};

SDLTTF_FontLoaderFromFile::SDLTTF_FontLoaderFromFile() {
	TTF_Init();
}
SDLTTF_FontLoaderFromFile::~SDLTTF_FontLoaderFromFile() {
	TTF_Quit();
}

IFont* SDLTTF_FontLoaderFromFile::load(std::string face, int ptsize) {
	TTF_Font * mfont = TTF_OpenFontIndex(("fonts/" + face).c_str(), ptsize, 0);
	if(!mfont)
		throw BadFont((format("Font loading error for %s, %i pts: %s") % face % ptsize % TTF_GetError()).str());

	return new SDLTTF_Font(mfont);
}

IFontLoader * ttf_fontloader_from_file() {
	return new SDLTTF_FontLoaderFromFile();
}

}

