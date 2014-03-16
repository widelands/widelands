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

#include "graphic/font_handler.h"

#include <list>

#include <SDL_ttf.h>
#include <boost/algorithm/string.hpp>

#include "graphic/graphic.h"
#include "graphic/in_memory_image.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "graphic/wordwrap.h"
#include "log.h"
#include "wexception.h"

namespace UI {

namespace  {
/**
 * Draw the caret for the given text rendered exactly at the given point
 * (including \ref LINE_MARGIN).
 */
void draw_caret
	(RenderTarget & dst,
	 const TextStyle & style,
	 const Point& dstpoint,
	 const std::string & text,
	 uint32_t caret_offset)
{
	int caret_x = style.calc_bare_width(text.substr(0, caret_offset));

	const Image* caret_image = g_gr->images().get("pics/caret.png");
	Point caretpt;
	caretpt.x = dstpoint.x + caret_x + LINE_MARGIN - caret_image->width();
	caretpt.y = dstpoint.y + (style.font->height() - caret_image->height()) / 2;

	dst.blit(caretpt, caret_image);
}

}  // namespace

/// The global unique \ref Font_Handler object
Font_Handler * g_fh = nullptr;

/**
 * The line cache stores unprocessed rendered lines of text.
 */
struct LineCacheEntry {
	/*@{*/
	TextStyle style;
	std::string text;
	/*@}*/

	/*@{*/
	const Image* image;
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

	~Data() {
		while (!linecache.empty()) {
			delete linecache.back().image;
			linecache.pop_back();
		}
	}

private:
	void render_line(LineCacheEntry & lce);
};

/**
 * Plain Constructor
 */
Font_Handler::Font_Handler() :
	d(new Data)
{
}


Font_Handler::~Font_Handler() {
	flush();
	Font::shutdown();
}

void Font_Handler::flush() {
	d.reset(new Data);
}

/*
 * Returns the height of the font, in pixels.
*/
uint32_t Font_Handler::get_fontheight
	(const std::string & name, int32_t const size)
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

	// Cache miss; render a new image.
	LineCache::iterator it = linecache.insert(linecache.begin(), LineCacheEntry());
	it->style = style;
	it->text = text;
	it->image = nullptr;
	render_line(*it);

	while (linecache.size() > MaxLineCacheSize) {
		delete linecache.back().image;
		linecache.pop_back();
	}

	return *it;
}

/**
 * Render the image of a \ref LineCacheEntry whose key data has
 * already been filled in.
 */
void Font_Handler::Data::render_line(LineCacheEntry & lce)
{
	TTF_Font * font = lce.style.font->get_ttf_font();
	SDL_Color sdl_fg = {lce.style.fg.r, lce.style.fg.g, lce.style.fg.b, 0};

	// Work around an Issue in SDL_TTF that dies when the surface
	// has zero width
	int width = 0;
	if (TTF_SizeUTF8(font, lce.text.c_str(), &width, nullptr) < 0 or !width) {
		lce.width = 0;
		lce.height = TTF_FontHeight(font);
		return;
	}

	lce.style.setup();

	SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font, lce.text.c_str(), sdl_fg);
	if (!text_surface) {
		log
			("Font_Handler::render_line, an error : %s\n",
			 TTF_GetError());
		log("Text was: '%s'\n", lce.text.c_str());
		return;
	}

	lce.image = new_in_memory_image("dummy_hash", Surface::create(text_surface));
	lce.width = lce.image->width();
	lce.height = lce.image->height();
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
	// Erase every backslash in front of brackets
	std::string copytext = boost::replace_all_copy(text, "\\<", "<");
	boost::replace_all(copytext, "\\>", ">");
	const LineCacheEntry & lce = d->get_line(style, copytext);

	UI::correct_for_align(align, lce.width + 2 * LINE_MARGIN, lce.height, &dstpoint);

	if (lce.image)
		dst.blit(Point(dstpoint.x + LINE_MARGIN, dstpoint.y), lce.image);

	if (caret <= copytext.size())
		draw_caret(dst, style, dstpoint, copytext, caret);
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

	if (lce.image)
		dst.blit(dstpoint, lce.image);

	return lce.width;
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
	(const std::string & fontname, int32_t const fontsize,
	 const std::string & text,
	 uint32_t & w, uint32_t & h,
	 uint32_t const wrap)
{
	// use bold style by default for historical reasons
	get_size(TextStyle::makebold(Font::get(fontname, fontsize), RGBColor(255, 255, 255)), text, w, h, wrap);
}

} // namespace UI
