/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#include <iostream>
#include <SDL_ttf.h>
#include "font_handler.h"
#include "font_loader.h"
#include "wexception.h"
#include "rgbcolor.h"
#include "graphic.h"
#include "error.h"
#include "filesystem.h"

Font_Handler* g_fh = 0; // the font handler

/*
===============================================================================

Font Loader IMPLEMENTATION

===============================================================================
*/


/*
* Opens a font file and returns a TTF_FONT* pointer.
*/

TTF_Font* Font_Loader::open_font(const std::string& name, int size) {
	// Load the TrueType Font
   std::string filename="fonts/";
   filename+=name;
	TTF_Font* font = TTF_OpenFont(filename.c_str(),size);
   if(!font) throw wexception("Couldn't load font!: %s\n", TTF_GetError());
	return font;
}

/*
* Looks for a font with given name and size in the font cache and returns it.
* Ohterwise font will be loaded with open_font and chached.
*/
TTF_Font* Font_Loader::get_font(std::string name, int size) {
	char buf[5];
	snprintf(buf,sizeof(buf),"%i",size);
	
	std::string key_name = name+"-"+buf;
	const std::map<std::string,TTF_Font*>::iterator it = m_font_table.find(key_name);
	if(it != m_font_table.end()) {
		TTF_SetFontStyle(it->second,TTF_STYLE_BOLD);
		return it->second;
	}

	TTF_Font* font = open_font(name,size);

	if(font == NULL)
		return NULL;

	TTF_SetFontStyle(font,TTF_STYLE_BOLD);
	
	m_font_table.insert(std::pair<std::string,TTF_Font*>(key_name,font));
	
   return font;
}

/*
* Clears the font cache.
*/
void Font_Loader::clear_fonts() {
	for(std::map<std::string,TTF_Font*>::iterator i = m_font_table.begin(); i != m_font_table.end(); i++) {
		TTF_CloseFont(i->second);
	}
	m_font_table.clear();
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

