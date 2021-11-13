/*
 * Copyright (C) 2006-2021 by the Widelands Development Team
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

#include "graphic/text/sdl_ttf_font.h"

#include <memory>

#include "base/string.h"
#include "graphic/sdl_utils.h"
#include "graphic/text/rt_errors.h"

static const int SHADOW_OFFSET = 1;
static const SDL_Color SHADOW_CLR = {0, 0, 0, SDL_ALPHA_OPAQUE};

namespace RT {

SdlTtfFont::SdlTtfFont(TTF_Font* font,
                       const std::string& face,
                       int ptsize,
                       std::string* ttf_memory_block)
   : font_(font),
     style_(TTF_STYLE_NORMAL),
     font_name_(face),
     ptsize_(ptsize),
     ttf_file_memory_block_(ttf_memory_block) {
}

SdlTtfFont::~SdlTtfFont() {
	TTF_CloseFont(font_);
	font_ = nullptr;
}

void SdlTtfFont::dimensions(const std::string& txt, int style, uint16_t* gw, uint16_t* gh) {
	set_style(style);

	int w, h;
	TTF_SizeUTF8(font_, txt.c_str(), &w, &h);

	if (style & SHADOW) {
		w += SHADOW_OFFSET;
		h += SHADOW_OFFSET;
	}
	*gw = w;
	*gh = h;
}

std::shared_ptr<const Image> SdlTtfFont::render(const std::string& txt,
                                                const RGBColor& clr,
                                                int style,
                                                TextureCache* texture_cache) {
	const std::string hash =
	   bformat("ttf:%s:%s:%i:%02x%02x%02x:%i", font_name_, ptsize_, txt, static_cast<int>(clr.r),
	           static_cast<int>(clr.g), static_cast<int>(clr.b), style);
	std::shared_ptr<const Image> rv = texture_cache->get(hash);
	if (rv != nullptr) {
		return rv;
	}

	set_style(style);

	SDL_Surface* text_surface = nullptr;

	SDL_Color sdlclr = {clr.r, clr.g, clr.b, SDL_ALPHA_OPAQUE};
	if (style & SHADOW) {
		SDL_Surface* tsurf = TTF_RenderUTF8_Blended(font_, txt.c_str(), sdlclr);
		SDL_Surface* shadow = TTF_RenderUTF8_Blended(font_, txt.c_str(), SHADOW_CLR);
		text_surface = empty_sdl_surface(shadow->w + SHADOW_OFFSET, shadow->h + SHADOW_OFFSET);
		CLANG_DIAG_OFF("-Wunknown-pragmas")
		CLANG_DIAG_OFF("-Wzero-as-null-pointer-constant")
		SDL_FillRect(text_surface, nullptr,
		             SDL_MapRGBA(text_surface->format, 255, 255, 255, SDL_ALPHA_TRANSPARENT));
		CLANG_DIAG_ON("-Wzero-as-null-pointer-constant")
		CLANG_DIAG_ON("-Wunknown-pragmas")

		if (text_surface->format->BitsPerPixel != 32) {
			SDL_FreeSurface(tsurf);
			SDL_FreeSurface(shadow);
			throw RenderError("SDL_TTF did not return a 32 bit surface for shadow text. Giving up!");
		}

		SDL_Rect dstrct1 = {0, 0, 0, 0};
		SDL_SetSurfaceAlphaMod(shadow, SDL_ALPHA_OPAQUE);
		SDL_SetSurfaceBlendMode(shadow, SDL_BLENDMODE_NONE);
		SDL_BlitSurface(shadow, nullptr, text_surface, &dstrct1);

		uint32_t* spix = static_cast<uint32_t*>(tsurf->pixels);
		uint32_t* dpix = static_cast<uint32_t*>(text_surface->pixels);

		// Alpha Blend the Text onto the Shadow. This is really slow, but it is
		// the only compatible way to do it using SDL 1.2. SDL 2.0 offers more
		// functionality but is not yet released.
		uint8_t sr, sg, sb, sa, dr, dg, db, da, outa, outr = 0, outg = 0, outb = 0;
		for (int y = 0; y < tsurf->h; ++y) {
			for (int x = 0; x < tsurf->w; ++x) {
				size_t sidx = (y * tsurf->pitch + 4 * x) / 4;
				size_t didx = ((y + SHADOW_OFFSET) * text_surface->pitch + (x + SHADOW_OFFSET) * 4) / 4;

				SDL_GetRGBA(spix[sidx], tsurf->format, &sr, &sg, &sb, &sa);
				SDL_GetRGBA(dpix[didx], text_surface->format, &dr, &dg, &db, &da);

				outa = (255 * sa + da * (255 - sa)) / 255;
				if (outa) {
					outr = (255 * sa * sr + da * dr * (255 - sa)) / outa / 255;
					outg = (255 * sa * sg + da * dg * (255 - sa)) / outa / 255;
					outb = (255 * sa * sb + da * db * (255 - sa)) / outa / 255;
				}
				dpix[didx] = SDL_MapRGBA(text_surface->format, outr, outg, outb, outa);
			}
		}
		SDL_FreeSurface(tsurf);
		SDL_FreeSurface(shadow);
	} else {
		text_surface = TTF_RenderUTF8_Blended(font_, txt.c_str(), sdlclr);
	}

	if (!text_surface) {
		throw RenderError(bformat("Rendering '%s' gave the error: %s", txt, TTF_GetError()));
	}

	return texture_cache->insert(hash, std::make_shared<Texture>(text_surface));
}

uint16_t SdlTtfFont::ascent(int style) const {
	uint16_t rv = TTF_FontAscent(font_);
	if (style & SHADOW) {
		rv += SHADOW_OFFSET;
	}
	return rv;
}

void SdlTtfFont::set_style(int style) {
	int sdl_style = TTF_STYLE_NORMAL;
	if (style & UNDERLINE) {
		sdl_style |= TTF_STYLE_UNDERLINE;
	}

	// Remember the last style. This should avoid that SDL_TTF flushes its
	// glyphcache all too often
	if (sdl_style == style_) {
		return;
	}
	style_ = sdl_style;
	TTF_SetFontStyle(font_, sdl_style);
}

}  // namespace RT
