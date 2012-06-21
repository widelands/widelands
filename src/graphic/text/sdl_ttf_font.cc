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
#include "sdl_helper.h"

using namespace std;
using namespace boost;

static const int SHADOW_OFFSET = 1;
static const SDL_Color SHADOW_CLR = { 0, 0, 0, SDL_ALPHA_OPAQUE};

namespace RT {

class SDLTTF_Font : public IFont {
public:
	virtual ~SDLTTF_Font();
	SDLTTF_Font(TTF_Font* ttf);

	void dimensions(string, int, uint32_t * w, uint32_t * h);
	virtual SDL_Surface * render(string, SDL_Color clr, int);
	uint32_t ascent(int) const;

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

	if (style & SHADOW) {
		w += SHADOW_OFFSET; h += SHADOW_OFFSET;
	}
	*gw = w; *gh = h;
}

SDL_Surface * SDLTTF_Font::render(string txt, SDL_Color clr, int style) {
	m_set_style(style);

	SDL_Surface * text_surface = 0;

	if (style & SHADOW) {
		SDL_Surface * tsurf = TTF_RenderUTF8_Blended(m_font, txt.c_str(), clr);
		SDL_Surface * shadow = TTF_RenderUTF8_Blended(m_font, txt.c_str(), SHADOW_CLR);
		text_surface = empty_sdl_surface(shadow->w + SHADOW_OFFSET, shadow->h + SHADOW_OFFSET);

		if (text_surface->format->BitsPerPixel != 32)
			throw RenderError("SDL_TTF did not return a 32 bit surface for shadow text. Giving up!");

		SDL_Rect dstrct1 = { 0, 0, 0, 0 };
		SDL_SetAlpha(shadow, 0, SDL_ALPHA_OPAQUE);
		SDL_BlitSurface(shadow, 0, text_surface, &dstrct1);

		Uint32 * spix = (Uint32*)tsurf->pixels;
		Uint32 * dpix = (Uint32*)text_surface->pixels;

		// Alpha Blend the Text onto the Shadow. This is really slow, but it is
		// the only compatible way to do it using SDL 1.2. SDL 2.0 offers more
		// functionality but is not yet released.
		Uint8 sr, sg, sb, sa, dr, dg, db, da, outa, outr=0, outg=0, outb=0;
		for (uint32_t y = 0; y < tsurf->h; ++y) {
			for (uint32_t x = 0; x < tsurf->w; ++x) {
				size_t sidx = (y*tsurf->pitch + 4*x) / 4;
				size_t didx = ((y+SHADOW_OFFSET)*text_surface->pitch + (x+SHADOW_OFFSET)*4) / 4;

				SDL_GetRGBA(spix[sidx], tsurf->format, &sr, &sg, &sb, &sa);
				SDL_GetRGBA(dpix[didx], text_surface->format, &dr, &dg, &db, &da);

				outa = (255*sa + da*(255 - sa)) / 255;
				if (outa) {
					outr = (255 * sa*sr + da*dr*(255-sa)) / outa / 255;
					outg = (255 * sa*sg + da*dg*(255-sa)) / outa / 255;
					outb = (255 * sa*sb + da*db*(255-sa)) / outa / 255;
				}
				dpix[didx] = SDL_MapRGBA(text_surface->format, outr, outg, outb, outa);
			}
		}
		SDL_FreeSurface(tsurf);
		SDL_FreeSurface(shadow);
	} else
		text_surface= TTF_RenderUTF8_Blended(m_font, txt.c_str(), clr);

	if (not text_surface)
		throw RenderError((format("Rendering '%s' gave the error: %s") % txt % TTF_GetError()).str());
	return text_surface;
}

uint32_t SDLTTF_Font::ascent(int style) const {
	uint32_t rv = TTF_FontAscent(m_font);
	if (style & SHADOW)
		rv += SHADOW_OFFSET;
	return rv;
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

