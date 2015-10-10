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

#include "base/utf8.h"
#include "graphic/font_handler1.h" // We need the fontset for the size offset
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"
#include "io/filesystem/layered_filesystem.h"

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

using FontMap = std::map<FontDescr, UI::Font *>;

FontMap g_fontmap;

} // anonymous namespace


namespace UI {

/**
 * Open a font file and load the corresponding font.
 */
Font::Font(const std::string & name, int input_size)
{
	// Load the TrueType Font
	std::string filename = "i18n/fonts/";
	filename += name;
	m_size = input_size;

	//  We must keep this File Read open, otherwise the following calls are
	//  crashing. do not know why...
	m_fontfile.open(*g_fs, filename);

	SDL_RWops * const ops = SDL_RWFromMem(m_fontfile.data(0), m_fontfile.get_size());
	if (!ops)
		throw wexception("could not load font!: RWops Pointer invalid");

	m_font = TTF_OpenFontIndexRW(ops, 1, input_size, 0);
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
 * \return the maximum height of glyphs of this font.
 */
uint32_t Font::size() const
{
	return m_size;
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
	size += UI::g_fh1->fontset().size_offset();
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

} // namespace UI
