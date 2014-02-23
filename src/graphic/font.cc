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
 */

#include "graphic/font.h"

#include <map>

#include "constants.h"
#include "io/filesystem/layered_filesystem.h"
#include "utf8.h"

namespace {

struct FontDescr {
	std::string name;
	int size;

	bool operator<(const FontDescr & o) const {
		return
			size < o.size ||
			(size == o.size && name < o.name);
	}
};

typedef std::map<FontDescr, UI::Font *> FontMap;

FontMap g_fontmap;

} // anonymous namespace


namespace UI {

/**
 * Open a font file and load the corresponding font.
 */
Font::Font(const std::string & name, int size)
{
	// Load the TrueType Font
	std::string filename = "fonts/";
	filename += name;

	//  We must keep this File Read open, otherwise the following calls are
	//  crashing. do not know why...
	m_fontfile.Open(*g_fs, filename.c_str());

	SDL_RWops * const ops = SDL_RWFromMem(m_fontfile.Data(0), m_fontfile.GetSize());
	if (!ops)
		throw wexception("could not load font!: RWops Pointer invalid");

	m_font = TTF_OpenFontIndexRW(ops, 1, size, 0);
	if (!m_font)
		throw wexception("could not load font!: %s", TTF_GetError());

	// Compute the line skip based on some glyphs by sampling some letters,
	// special characters, and accented/umlauted versions of capital A
	// It seems more reasonable to use TTF_FontLineSkip(), but the fonts
	// we use claim to have a very excessive line skip.
	static uint16_t glyphs[] = {'A', '_', '@', ',', 'q', 'y', '"', 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5};
	m_computed_typical_miny = 0;
	m_computed_typical_maxy = 0;

	for (unsigned int idx = 0; idx < sizeof(glyphs) / sizeof(glyphs[0]); ++idx) {
		int miny, maxy;
		if (TTF_GlyphMetrics(m_font, glyphs[idx], nullptr, nullptr, &miny, &maxy, nullptr) < 0)
			continue; // error, e.g. glyph not found

		if (miny < m_computed_typical_miny)
			m_computed_typical_miny = miny;
		if (maxy > m_computed_typical_maxy)
			m_computed_typical_maxy = maxy;
	}
}

/**
 * Free all resources associated with this font.
 */
Font::~Font()
{
	TTF_CloseFont(m_font);
	m_font = nullptr;
}

/**
 * \return the maximum height of glyphs of this font.
 */
uint32_t Font::height() const
{
	return TTF_FontHeight(m_font);
}

/**
 * \return the maximum ascent from the font baseline
 */
uint32_t Font::ascent() const
{
	return TTF_FontAscent(m_font);
}

/**
 * \return the number of pixels between lines in this font (from baseline to baseline).
 */
uint32_t Font::lineskip() const
{
	return m_computed_typical_maxy - m_computed_typical_miny;
}

/**
 * Return the font for the given name and size.
 *
 * \note Fonts are cached and need not be freed explicitly. \see shutdown
 */
Font * Font::get(const std::string & name, int size)
{
	FontDescr descr;
	descr.name = name;
	descr.size = size;

	FontMap::iterator it = g_fontmap.find(descr);
	if (it == g_fontmap.end()) {
		Font * font = new Font(name, size);
		it = g_fontmap.insert(std::make_pair(descr, font)).first;
	}

	return it->second;
}

/**
 * Free all registered fonts.
 */
void Font::shutdown()
{
	while (!g_fontmap.empty()) {
		delete g_fontmap.begin()->second;
		g_fontmap.erase(g_fontmap.begin());
	}
}

/**
 * Prepare the TTF style settings for rendering in this style.
 */
void TextStyle::setup() const
{
	int32_t font_style = TTF_STYLE_NORMAL;
	if (bold)
		font_style |= TTF_STYLE_BOLD;
	if (italics)
		font_style |= TTF_STYLE_ITALIC;
	if (underline)
		font_style |= TTF_STYLE_UNDERLINE;
	TTF_SetFontStyle(font->get_ttf_font(), font_style);
}

/**
 * Compute the bare width (without caret padding) of the given string.
 */
uint32_t TextStyle::calc_bare_width(const std::string & text) const
{
	int w, h;
	setup();
	TTF_SizeUTF8(font->get_ttf_font(), text.c_str(), &w, &h);
	return w;
}

/**
 * \note Please only use this function once you understand the definitions
 * of ascent/descent etc.
 *
 * Computes the actual line height we should use for rendering the given text.
 * This is heuristic, because it pre-initializes the miny and maxy values to
 * the ones that are typical for Latin scripts, so that lineskips should always
 * be the same for such scripts.
 */
void TextStyle::calc_bare_height_heuristic(const std::string & text, int32_t & miny, int32_t & maxy) const
{
	miny = font->m_computed_typical_miny;
	maxy = font->m_computed_typical_maxy;

	setup();
	std::string::size_type pos = 0;
	while (pos < text.size()) {
		uint16_t ch = Utf8::utf8_to_unicode(text, pos);
		int32_t glyphminy, glyphmaxy;
		TTF_GlyphMetrics(font->get_ttf_font(), ch, nullptr, nullptr, &glyphminy, &glyphmaxy, nullptr);
		miny = std::min(miny, glyphminy);
		maxy = std::max(maxy, glyphmaxy);
	}
}


/*
=============================

Default fonts and styles

=============================
*/

Font * Font::ui_big()
{
	Font * font = nullptr;
	if (!font)
		font = Font::get(UI_FONT_BIG);
	return font;
}

Font * Font::ui_small()
{
	Font * font = nullptr;
	if (!font)
		font = Font::get(UI_FONT_SMALL);
	return font;
}

Font * Font::ui_ultrasmall()
{
	Font * font = nullptr;
	if (!font)
		font = Font::get(UI_FONT_ULTRASMALL);
	return font;
}

const TextStyle & TextStyle::ui_big()
{
	static TextStyle style;
	static bool init = false;

	if (!init) {
		style.font = Font::ui_big();
		style.fg = UI_FONT_CLR_FG;
		style.bold = true;
		init = true;
	}

	return style;
}

const TextStyle & TextStyle::ui_small()
{
	static TextStyle style;
	static bool init = false;

	if (!init) {
		style.font = Font::ui_small();
		style.fg = UI_FONT_CLR_FG;
		style.bold = true;
		init = true;
	}

	return style;
}

const TextStyle & TextStyle::ui_ultrasmall()
{
	static TextStyle style;
	static bool init = false;

	if (!init) {
		style.font = Font::ui_ultrasmall();
		style.fg = UI_FONT_CLR_FG;
		style.bold = true;
		init = true;
	}

	return style;
}

} // namespace UI
