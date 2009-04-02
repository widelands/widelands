/*
 * Copyright (C) 2002, 2008-2009 by the Widelands Development Team
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

#include "font_loader.h"

#include "fileread.h"
#include "font_handler.h"
#include "graphic.h"
#include "layered_filesystem.h"
#include "rgbcolor.h"
#include "wexception.h"

#include <SDL_ttf.h>

#include <iostream>
#include <string>
#include <cstring>

Font_Handler * g_fh = 0; // the font handler

/*
===============================================================================

Font Loader IMPLEMENTATION

===============================================================================
*/


/*
* Opens a font file and returns a TTF_FONT* pointer.
*/

TTF_Font * Font_Loader::open_font(std::string const & name, int32_t const size)
{
	// Load the TrueType Font
	std::string filename = "fonts/";
	filename            += name;

	//  We must keep this File Read open, otherwise the following calls are
	//  crashing. do not know why...
	FileRead & fr = *new FileRead();
	fr.Open(*g_fs, filename.c_str());

	m_freads.push_back(&fr);

	SDL_RWops * const ops = SDL_RWFromMem(fr.Data(0), fr.GetSize());
	if (!ops)
		throw wexception("could not load font!: RWops Pointer invalid");

	if (TTF_Font * const font = TTF_OpenFontIndexRW(ops, 1, size, 0))
		return font;
	else
		throw wexception("could not load font!: %s", TTF_GetError());
}

/*
* Looks for a font with given name and size in the font cache and returns it.
* Ohterwise font will be loaded with open_font and chached.
*/
TTF_Font * Font_Loader::get_font(std::string const & name, int32_t const size)
{
	char buffer[5];
	snprintf(buffer, sizeof(buffer), "%i", size);

	const std::string key_name = name + '-' + buffer;
	std::map<std::string, TTF_Font *>::iterator const it =
		m_font_table.find(key_name);
	if (it != m_font_table.end()) {
		TTF_SetFontStyle(it->second, TTF_STYLE_BOLD);
		return it->second;
	}

	TTF_Font * const font = open_font(name, size);

	if (font) {
		TTF_SetFontStyle(font, TTF_STYLE_BOLD);
		m_font_table.insert(std::pair<std::string, TTF_Font *>(key_name, font));
	}
	return font;
}

/*
* Clears the font cache.
*/
void Font_Loader::clear_fonts() {
	for
		(std::map<std::string, TTF_Font *>::iterator i = m_font_table.begin();
		 i != m_font_table.end();
		 ++i)
		TTF_CloseFont(i->second);
	m_font_table.clear();

	for (uint32_t i = 0; i < m_freads.size(); ++i)
		delete m_freads[i];
	m_freads.clear();
}


/*
===============
Font::~Font

Free resources.
===============
*/
Font_Loader::~Font_Loader() {
	clear_fonts();
}
