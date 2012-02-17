/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Some Methods taken from Wesnoth.
 * http://www.wesnoth.org
 */

#include "font_handler.h"

#include "io/filesystem/filesystem.h"
#include "font.h"
#include "helper.h"
#include "log.h"
#include "wexception.h"
#include "text_parser.h"
#include "upcast.h"

#include "rendertarget.h"
#include "surface.h"
#include "graphic/render/surface_sdl.h"
#include "wordwrap.h"

#include <SDL_image.h>
#include <SDL_ttf.h>

#include <algorithm>
#include <iostream>
#include <boost/concept_check.hpp>
#include <boost/concept_check.hpp>
#include <scripting/pdep/llimits.h>

namespace UI {

/// The global unique \ref Font_Handler object
Font_Handler * g_fh = 0;

/**
 * The line cache stores unprocessed rendered lines of text.
 */
struct LineCacheEntry {
	/*@{*/
	TextStyle style;
	std::string text;
	/*@}*/

	/*@{*/
	PictureID picture;
	uint32_t width;
	uint32_t height;
	/*@}*/
};

typedef std::list<LineCacheEntry> LineCache;

static const unsigned MaxLineCacheSize = 500;

/**
 * Internal data of the \ref Font_Handler.
 */
struct Font_Handler::Data {
	LineCache linecache;

	const LineCacheEntry & get_line(const TextStyle & style, const std::string & text);

private:
	void render_line(LineCacheEntry & lce);
};

/**
 * Plain Constructor
 */
Font_Handler::Font_Handler() :
	d(new Data)
{
	if (TTF_Init() == -1)
		throw wexception
			("True Type library did not initialize: %s\n", TTF_GetError());
}


Font_Handler::~Font_Handler() {
	Font::shutdown();
	TTF_Quit();
}


/*
 * Returns the height of the font, in pixels.
*/
uint32_t Font_Handler::get_fontheight
	(std::string const & name, int32_t const size)
{
	TTF_Font * const f = Font::get(name, size)->get_ttf_font();
	const int32_t fontheight = TTF_FontHeight(f);
	if (fontheight < 0)
		throw wexception
			("TTF_FontHeight returned a negative value, which does not have a "
			 "known meaning.");
	return fontheight;
}

/**
 * Get a cache entry for the given text (without linebreaks!) rendered
 * in the given style.
 *
 * If there is no pre-existing cache entry, a new one is created.
 */
const LineCacheEntry & Font_Handler::Data::get_line(const UI::TextStyle & style, const std::string & text)
{
	for (LineCache::iterator it = linecache.begin(); it != linecache.end(); ++it) {
		if (it->style != style || it->text != text)
			continue;

		// Found a hit, move to front
		if (it != linecache.begin())
			linecache.splice(linecache.begin(), linecache, it);
		return *it;
	}

	// Cache miss; render a new picture
	LineCache::iterator it = linecache.insert(linecache.begin(), LineCacheEntry());
	it->style = style;
	it->text = text;
	render_line(*it);

	while (linecache.size() > MaxLineCacheSize)
		linecache.pop_back();

	return *it;
}

/**
 * Render the picture of a \ref LineCacheEntry whose key data has
 * already been filled in.
 */
void Font_Handler::Data::render_line(LineCacheEntry & lce)
{
	//static int count = 0;
	//log("render_line(%s): %i\n", lce.text.c_str(), ++count);

	TTF_Font * font = lce.style.font->get_ttf_font();
	SDL_Color sdl_fg = {lce.style.fg.r(), lce.style.fg.g(), lce.style.fg.b(), 0};

	// Work around an Issue in SDL_TTF that dies when the surface
	// has zero width
	int width = 0;
	if (TTF_SizeUTF8(font, lce.text.c_str(), &width, 0) < 0 or !width) {
		lce.width = 0;
		lce.height = TTF_FontHeight(font);
		return;
	}

	lce.style.setup();

	SDL_Surface * text_surface = TTF_RenderUTF8_Blended(font, lce.text.c_str(), sdl_fg);
	if (!text_surface) {
		log
			("Font_Handler::render_line, an error : %s\n",
			 TTF_GetError());
		log("Text was: '%s'\n", lce.text.c_str());
		return;
	}

	lce.picture = g_gr->convert_sdl_surface_to_picture(text_surface, true);
	lce.width = lce.picture->get_w();
	lce.height = lce.picture->get_h();
}

/**
 * Draw unwrapped, single-line text (i.e. no line breaks).
 */
void Font_Handler::draw_text
	(RenderTarget & dst,
	 const TextStyle & style,
	 Point dstpoint,
	 const std::string & text,
	 Align align,
	 uint32_t caret)
{
	const LineCacheEntry & lce = d->get_line(style, text);

	do_align(align, dstpoint.x, dstpoint.y, lce.width + 2 * LINE_MARGIN, lce.height);

	if (lce.picture && lce.picture->valid())
		dst.blit(Point(dstpoint.x + LINE_MARGIN, dstpoint.y), lce.picture);

	if (caret <= text.size())
		draw_caret(dst, style, dstpoint, text, caret);
}

/**
 * Draw unwrapped, single-line text (i.e. no line breaks) with a gray shadow.
 */
void Font_Handler::draw_text_shadow
	(RenderTarget & dst,
	 const TextStyle & style,
	 Point dstpoint,
	 const std::string & text,
	 Align align,
	 uint32_t caret)
{

	TextStyle gray_style = style;
	gray_style.fg = RGBColor (0, 0, 0);

	draw_text (dst, gray_style, dstpoint - Point(1, 1), text, align, caret);
	draw_text (dst, style, dstpoint, text, align, caret);
}

/**
 * Draw unwrapped, un-aligned single-line text at the given point, and return the width of the text.
 */
uint32_t Font_Handler::draw_text_raw
	(RenderTarget & dst,
	 const UI::TextStyle & style,
	 Point dstpoint,
	 const std::string & text)
{
	const LineCacheEntry & lce = d->get_line(style, text);

	if (lce.picture && lce.picture->valid())
		dst.blit(dstpoint, lce.picture);

	return lce.width;
}

/**
 * Draw multi-line text, optionally with wrapping.
 *
 * @note Consider using @ref WordWrap directly, if the same multi-line
 * text is going to be rendered many times.
 */
void Font_Handler::draw_multiline
	(RenderTarget & dst,
	 const TextStyle & style,
	 Point dstpoint,
	 const std::string & text,
	 Align align,
	 uint32_t wrap,
	 uint32_t caret)
{
	WordWrap ww(style, wrap);

	ww.wrap(text);
	ww.draw(dst, dstpoint, align, caret);
}


/**
 * Draw the caret for the given text rendered exactly at the given point
 * (including \ref LINE_MARGIN).
 */
void Font_Handler::draw_caret
	(RenderTarget & dst,
	 const TextStyle & style,
	 Point dstpoint,
	 const std::string & text,
	 uint32_t caret)
{
	std::string sub = text.substr(0, caret);

	int caret_x = style.calc_bare_width(sub);

	PictureID caretpic = g_gr->get_picture(PicMod_UI, "pics/caret.png");
	Point caretpt;
	caretpt.x = dstpoint.x + caret_x + LINE_MARGIN - caretpic->get_w();
	caretpt.y = dstpoint.y + (style.font->height() - caretpic->get_h()) / 2;

	dst.blit(caretpt, caretpic);
}


//Sets dstx and dsty to values for a specified align
void Font_Handler::do_align
	(Align const align,
	 int32_t & dstx, int32_t & dsty,
	 int32_t const w, int32_t const h)
{
	//Vertical Align
	if (align & (Align_VCenter | Align_Bottom)) {
		if (align & Align_VCenter)
			dsty -= (h + 1) / 2; //  +1 for slight bias to top
		else
			dsty -= h;
	}

	//Horizontal Align
	if ((align & Align_Horizontal) != Align_Left) {
		if (align & Align_HCenter)
			dstx -= w / 2;
		else if (align & Align_Right)
			dstx -= w;
	}
}

/*
 * Flushes the cached picture ids
 */
void Font_Handler::flush_cache()
{
	d->linecache.clear();
}

/**
 * Compute the total size of the given text, when wrapped to the given
 * maximum width and rendered in the given text style.
 */
void Font_Handler::get_size
	(const TextStyle & textstyle,
	 const std::string & text,
	 uint32_t & w, uint32_t & h,
	 uint32_t wrap)
{
	WordWrap ww(textstyle, wrap);
	ww.wrap(text);
	w = ww.width();
	h = ww.height();
}

/**
 * Calculates size of a given text.
 */
void Font_Handler::get_size
	(std::string const & fontname, int32_t const fontsize,
	 const std::string & text,
	 uint32_t & w, uint32_t & h,
	 uint32_t const wrap)
{
	// use bold style by default for historical reasons
	get_size(TextStyle::makebold(Font::get(fontname, fontsize), RGBColor(255, 255, 255)), text, w, h, wrap);
}

} // namespace UI
