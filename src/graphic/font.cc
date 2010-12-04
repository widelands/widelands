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

#include "font.h"

#include <map>

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
}

/**
 * Free all resources associated with this font.
 */
Font::~Font()
{
	TTF_CloseFont(m_font);
	m_font = 0;
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

	//TODO: get rid of this nasty hack
	TTF_SetFontStyle(it->second->get_ttf_font(), TTF_STYLE_BOLD);

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
